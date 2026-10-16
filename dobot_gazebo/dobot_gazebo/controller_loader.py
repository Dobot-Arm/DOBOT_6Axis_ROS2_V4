#!/usr/bin/env python3
"""Load and activate controllers for Gazebo simulation.

Works around a gz_ros2_control bug where load_controller and
configure_controller service calls return ok=false non-deterministically
even when the operation actually succeeds. Instead of trusting the ok field,
this script checks the actual controller state via list_controllers.

Controllers are loaded and configured in PARALLEL to minimize startup time.

Usage: controller_loader.py <robot_type>
Example: controller_loader.py cr5
"""
import sys
import time
import rclpy
from rclpy.node import Node
from controller_manager_msgs.srv import (
    ListControllers, ListHardwareInterfaces,
    LoadController, ConfigureController, SwitchController,
)

MAX_RETRIES = 10
POLL_INTERVAL = 0.2


def list_all_controllers(node, list_cli, timeout=5.0, retries=3):
    """Return dict {name: state} of all controllers, or {} on failure.

    Retries the service call to handle transient failures where the
    response is empty or the call times out (often caused by pending
    responses from previous load/configure requests still in the queue).
    """
    for i in range(retries):
        req = ListControllers.Request()
        future = list_cli.call_async(req)
        rclpy.spin_until_future_complete(node, future, timeout_sec=timeout)
        if future.done() and future.result() is not None:
            result = {c.name: c.state for c in future.result().controller}
            if result:  # non-empty response
                return result
        # Empty response or timeout; retry after short pause
        if i < retries - 1:
            time.sleep(0.1)
    return {}


def call_service_async(node, cli, request):
    """Call a service asynchronously, return the future."""
    return cli.call_async(request)


def main():
    rclpy.init(args=sys.argv)
    robot_type = sys.argv[1] if len(sys.argv) > 1 else 'cr5'

    controllers = [
        f'{robot_type}_group_controller',
        'joint_state_broadcaster',
    ]

    node = Node('controller_loader')
    # NOTE: Do NOT use sim_time here. With use_sim_time=true, service call
    # futures don't complete reliably (controller_manager uses sim_time but
    # the rclpy wait mechanism needs system time). System time works fine.
    logger = node.get_logger()

    # Wait for controller_manager
    list_cli = node.create_client(ListControllers, '/controller_manager/list_controllers')
    load_cli = node.create_client(LoadController, '/controller_manager/load_controller')
    cfg_cli = node.create_client(ConfigureController, '/controller_manager/configure_controller')
    sw_cli = node.create_client(SwitchController, '/controller_manager/switch_controller')
    hw_cli = node.create_client(ListHardwareInterfaces, '/controller_manager/list_hardware_interfaces')

    logger.info('Waiting for controller_manager...')
    for i in range(60):
        if list_cli.wait_for_service(timeout_sec=2.0):
            logger.info('Controller manager ready')
            break
    else:
        logger.fatal('Timeout waiting for controller_manager')
        sys.exit(1)

    # Wait for Gazebo hardware interfaces to be available before loading
    # controllers. This replaces a fixed 8s sleep: we poll list_hardware_interfaces
    # until the command interfaces (joint1/position, etc.) appear.
    logger.info('Waiting for hardware interfaces...')
    start = time.time()
    for attempt in range(100):
        req = ListHardwareInterfaces.Request()
        future = hw_cli.call_async(req)
        rclpy.spin_until_future_complete(node, future, timeout_sec=2.0)
        if future.done() and future.result() is not None:
            cmd_ifaces = [i.name for i in future.result().command_interfaces]
            if any('position' in n for n in cmd_ifaces):
                logger.info(f'  Hardware ready ({len(cmd_ifaces)} cmd interfaces, '
                            f'{time.time()-start:.1f}s)')
                break
        time.sleep(0.2)
    else:
        logger.fatal('Timeout waiting for hardware interfaces')
        sys.exit(1)

    # --- Phase 1: Parallel LOAD all controllers ---
    # Send all load requests at once, then poll until all appear in list_controllers.
    states = list_all_controllers(node, list_cli)
    to_load = [n for n in controllers if n not in states or states[n] == 'unconfigured']
    if to_load:
        logger.info(f'Loading (parallel): {to_load}')
        # Send all load requests simultaneously
        for name in to_load:
            req = LoadController.Request()
            req.name = name
            call_service_async(node, load_cli, req)
        # Poll until all loaded controllers appear (state will be 'unconfigured')
        for attempt in range(MAX_RETRIES):
            rclpy.spin_once(node, timeout_sec=0.1)
            time.sleep(POLL_INTERVAL)
            states = list_all_controllers(node, list_cli)
            if all(n in states for n in to_load):
                logger.info(f'  All loaded: {[(n, states[n]) for n in to_load]}')
                break
            # Resend load requests that haven't appeared yet
            missing = [n for n in to_load if n not in states]
            for name in missing:
                req = LoadController.Request()
                req.name = name
                call_service_async(node, load_cli, req)
        else:
            logger.fatal(f'  Failed to load after {MAX_RETRIES} attempts: {states}')
            sys.exit(1)

    # --- Phase 2: Parallel CONFIGURE all unconfigured controllers ---
    states = list_all_controllers(node, list_cli)
    to_configure = [n for n in controllers if states.get(n) == 'unconfigured']
    if to_configure:
        logger.info(f'Configuring (parallel): {to_configure}')
        # Send all configure requests simultaneously
        for name in to_configure:
            req = ConfigureController.Request()
            req.name = name
            call_service_async(node, cfg_cli, req)
        # Poll until all reach inactive/active
        for attempt in range(MAX_RETRIES):
            rclpy.spin_once(node, timeout_sec=0.1)
            time.sleep(POLL_INTERVAL)
            states = list_all_controllers(node, list_cli)
            if all(states.get(n) in ('inactive', 'active') for n in to_configure):
                logger.info(f'  All configured: {[(n, states[n]) for n in to_configure]}')
                break
            # Resend configure requests for stuck controllers
            stuck = [n for n in to_configure if states.get(n) == 'unconfigured']
            for name in stuck:
                req = ConfigureController.Request()
                req.name = name
                call_service_async(node, cfg_cli, req)
        else:
            logger.fatal(f'  Failed to configure after {MAX_RETRIES} attempts: {states}')
            sys.exit(1)

    # --- Phase 3: ACTIVATE all inactive controllers at once ---
    states = list_all_controllers(node, list_cli)
    to_activate = [n for n in controllers if states.get(n) == 'inactive']
    if to_activate:
        logger.info(f'Activating: {to_activate}')
        req = SwitchController.Request()
        req.activate_controllers = to_activate
        req.strictness = SwitchController.Request.BEST_EFFORT
        for attempt in range(MAX_RETRIES):
            sw_future = sw_cli.call_async(req)
            rclpy.spin_until_future_complete(node, sw_future, timeout_sec=5.0)
            time.sleep(POLL_INTERVAL)
            states = list_all_controllers(node, list_cli)
            if all(states.get(n) == 'active' for n in to_activate):
                logger.info(f'  All controllers active')
                break
        else:
            logger.fatal(f'  Failed to activate after {MAX_RETRIES} attempts: {states}')
            sys.exit(1)

    # Phase 3 already verified all controllers are active.
    # (Final verification removed: list_controllers can transiently return
    # empty right after a switch_controller call, causing false negatives.)

    logger.info('All controllers loaded and activated successfully!')
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
