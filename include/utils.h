#pragma once

#include <linux/rtnetlink.h>
#include <map>
#include <string>

namespace network_monitor::utils{

    enum InterfaceState{
        DOWN_NOT_RUNNING,
        DOWN_RUNNING,
        UP_NOT_RUNNING,
        UP_RUNNING
    };

    struct Interface{
        std::string name;
        std::string address;
        InterfaceState state;
        int index;
    };

    InterfaceState parseInterfaceState(const nlmsghdr *nh);
    std::string parseInterfaceName(const nlmsghdr *nh);
    std::string parseInterfaceIPAddress(const nlmsghdr *nh);
    int parseInterfaceIndex(const nlmsghdr *nh);

    std::map<int, rtattr*> parseRtAttr(struct rtattr *rtattr, int len);

}