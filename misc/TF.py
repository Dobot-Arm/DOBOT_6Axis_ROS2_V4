#!/usr/bin/env python3
import subprocess
import re

def getCurrentPose():
    """
    获取当前末端执行器位姿（base_link → Link6）
    返回: (translation, rotation) 或 (None, None)
    """
    try:
        result = subprocess.run(
            ["timeout", "3", "ros2", "run", "tf2_ros", "tf2_echo", "base_link", "Link6"],
            capture_output=True,
            text=True
        )

        out = result.stdout + result.stderr

        t = re.search(r"Translation:\s*\[([-\d.]+),\s*([-\d.]+),\s*([-\d.]+)\]", out)
        r = re.search(r"Rotation:\s*in Quaternion \(xyzw\)\s*\[([-\d.]+),\s*([-\d.]+),\s*([-\d.]+),\s*([-\d.]+)\]", out)

        if t and r:
            return (
                {"x": float(t[1]), "y": float(t[2]), "z": float(t[3])},
                {"x": float(r[1]), "y": float(r[2]), "z": float(r[3]), "w": float(r[4])}
            )
    except Exception:
        pass

    return None, None


if __name__ == "__main__":
    pos, ori = getCurrentPose()
    if pos:
        print("✅ 当前末端位姿：")
        print(f"位置: {pos}")
        print(f"姿态: {ori}")
