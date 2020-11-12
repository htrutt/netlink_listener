#pragma once

#include <string>
#include <linux/rtnetlink.h>
#include <map>

namespace network_monitor{

enum InterfaceState{
    DOWN_NOT_RUNNING,
    DOWN_RUNNING,
    UP_NOT_RUNNING,
    UP_RUNNING
};

class NetlinkSocket {
 public:
    explicit NetlinkSocket();
    ~NetlinkSocket();

    void startListening();
    void stopListening();

private:
    InterfaceState parseInterfaceState(const nlmsghdr *nh);
    std::string parseInterfaceName(const nlmsghdr *nh);
    std::string parseInterfaceIPAddress(const nlmsghdr *nh);
    std::map<int, rtattr*> parseRtAttr(struct rtattr *rtattr, int len);
    void listen();
    int netlink_fd_;

};

}