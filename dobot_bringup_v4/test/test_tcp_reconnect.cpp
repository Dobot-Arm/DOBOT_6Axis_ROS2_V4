/**
 * Regression tests for recovery of the controller TCP links.
 *
 * Restarting the Dobot controller used to require restarting the ROS node: the
 * links never came back on their own. Three separate defects caused that, one
 * per test below. Each test drives a throwaway loopback listener — no robot and
 * no ROS graph required.
 */

#include <gtest/gtest.h>
#include <dobot_bringup/command.h>
#include <dobot_bringup/tcp_socket.h>
#include <dirent.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// A loopback listener standing in for the controller. Binds an ephemeral port
// unless a specific one is requested (so a test can re-listen on a port it has
// deliberately vacated).
class Listener
{
public:
    explicit Listener(uint16_t port = 0)
    {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        int one = 1;
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(port);
        bound_ = fd_ >= 0 && ::bind(fd_, (sockaddr *)&addr, sizeof(addr)) == 0 &&
                 ::listen(fd_, 4) == 0;
        if (!bound_)
            return;

        socklen_t len = sizeof(addr);
        ::getsockname(fd_, (sockaddr *)&addr, &len);
        port_ = ntohs(addr.sin_port);
    }

    ~Listener()
    {
        if (fd_ >= 0)
            ::close(fd_);
    }

    bool bound() const { return bound_; }
    uint16_t port() const { return port_; }

    // Accept a pending connection, waiting up to timeout_ms. -1 if none arrived.
    int accept(int timeout_ms)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd_, &fds);
        timeval tv{ timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
        if (::select(fd_ + 1, &fds, nullptr, nullptr, &tv) <= 0)
            return -1;
        return ::accept(fd_, nullptr, nullptr);
    }

private:
    int fd_ = -1;
    bool bound_ = false;
    uint16_t port_ = 0;
};

// Number of descriptors this process holds open, or -1 where /proc is absent.
static int openFdCount()
{
    DIR *dir = ::opendir("/proc/self/fd");
    if (dir == nullptr)
        return -1;
    int count = 0;
    while (::readdir(dir) != nullptr)
        count++;
    ::closedir(dir);
    return count;
}

// A port with nothing listening on it: bind one, then let it go.
static uint16_t vacantPort()
{
    Listener probe;
    return probe.bound() ? probe.port() : 0;
}

// ---------------------------------------------------------------------------
// TcpClient
// ---------------------------------------------------------------------------

// A failed ::connect() leaves the socket spent — retrying on the same fd fails
// forever with EINVAL. connect() must therefore start from a fresh socket every
// time, or the very first attempt made while the controller was still booting
// would wedge the reconnect loop for the life of the process.
TEST(TcpClientReconnect, ConnectSucceedsAfterAFailedAttempt)
{
    uint16_t port = vacantPort();
    ASSERT_NE(port, 0);

    TcpClient client("127.0.0.1", port);
    EXPECT_THROW(client.connect(), TcpClientException);
    EXPECT_FALSE(client.isConnect());

    Listener server(port);
    ASSERT_TRUE(server.bound());

    ASSERT_NO_THROW(client.connect());
    EXPECT_TRUE(client.isConnect());
    int peer = server.accept(1000);
    EXPECT_GE(peer, 0);
    if (peer >= 0)
        ::close(peer);
}

// disConnect() used to clear fd_ before closing it, so it closed -1 and leaked
// the real descriptor — once reconnect attempts became frequent that would
// exhaust the process's descriptors.
TEST(TcpClientReconnect, DisconnectClosesTheDescriptor)
{
    int before = openFdCount();
    if (before < 0)
        GTEST_SKIP() << "/proc/self/fd unavailable on this platform";

    Listener server;
    ASSERT_TRUE(server.bound());
    TcpClient client("127.0.0.1", server.port());

    for (int i = 0; i < 20; i++)
    {
        ASSERT_NO_THROW(client.connect());
        int peer = server.accept(1000);
        ASSERT_GE(peer, 0);
        ::close(peer);
        client.disConnect();
    }

    EXPECT_LE(openFdCount(), before + 1);  // +1 for the listener itself
}

// A recv timeout says nothing about the socket, so it must not tear it down on
// its own — but the link has to stay droppable and re-establishable, which is
// how recvTask recovers from a silent controller.
TEST(TcpClientReconnect, TimeoutKeepsLinkThenReconnects)
{
    Listener server;
    ASSERT_TRUE(server.bound());
    TcpClient client("127.0.0.1", server.port());

    ASSERT_NO_THROW(client.connect());
    int peer = server.accept(1000);
    ASSERT_GE(peer, 0);

    uint8_t buf[64];
    EXPECT_FALSE(client.tcpRecvExact(buf, sizeof(buf), 50));  // server sends nothing
    EXPECT_TRUE(client.isConnect());

    client.disConnect();
    EXPECT_FALSE(client.isConnect());
    ::close(peer);

    ASSERT_NO_THROW(client.connect());
    EXPECT_TRUE(client.isConnect());
    int peer2 = server.accept(1000);
    EXPECT_GE(peer2, 0);
    if (peer2 >= 0)
        ::close(peer2);
}

// ---------------------------------------------------------------------------
// CRCommanderRos2::recvTask
// ---------------------------------------------------------------------------

// The reported failure: a controller restart leaves the realtime socket
// half-open (the controller never gets to send a FIN), so recv only ever times
// out while the connection still looks up. recvTask must drop the link and
// reconnect — observed here as a second connection arriving on the realtime
// port after the fake controller goes quiet.
TEST(CommanderReconnect, SilentRealtimeLinkIsDroppedAndReconnected)
{
    Listener realtime(30004);
    Listener dashboard(29999);
    if (!realtime.bound() || !dashboard.bound())
        GTEST_SKIP() << "ports 30004/29999 busy (a robot or another test run?)";

    CRCommanderRos2 commander("127.0.0.1");
    commander.init();

    int first = realtime.accept(2000);
    ASSERT_GE(first, 0) << "commander never opened the realtime link";
    int dash = dashboard.accept(2000);
    EXPECT_GE(dash, 0);

    // Send nothing on the accepted realtime socket: three 1 s recv timeouts must
    // convince recvTask the link is dead, and it must then reconnect.
    int second = realtime.accept(8000);
    EXPECT_GE(second, 0) << "commander never reconnected the realtime link";

    if (second >= 0)
        ::close(second);
    ::close(first);
    if (dash >= 0)
        ::close(dash);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
