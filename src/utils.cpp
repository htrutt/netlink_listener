#include "utils.h"
#include <arpa/inet.h>
#include <net/if.h>

namespace network_monitor::utils
{
    std::string parseInterfaceIPAddress(const nlmsghdr *nh){
    char ifAddress[256];    // network addr
    struct ifaddrmsg *ifa = (struct ifaddrmsg*)NLMSG_DATA(nh); // get data from the network interface
    int len = nh->nlmsg_len;

    rtattr *rtattr = IFA_RTA(ifa);
    auto rt_map = parseRtAttr(rtattr, len);

    auto iter = rt_map.find(IFA_LOCAL); // Find the interface name attribute

    if(iter != rt_map.end()){
        inet_ntop(AF_INET, RTA_DATA(iter->second), ifAddress, sizeof(ifAddress)); // get IP addr
    }
    return ifAddress;
}

InterfaceState parseInterfaceState(const nlmsghdr *nh){
    ifinfomsg *ifi = (ifinfomsg*) NLMSG_DATA(nh);    // get information about changed network interface
    bool if_running = false;
    bool if_up = false;

    ifi->ifi_flags & IFF_RUNNING ? if_running=true : if_running = false;
    ifi->ifi_flags & IFF_UP ? if_up=true : if_running = false;

    // TODO make this a bitmask
    if(if_running){
        if(if_up){
            return InterfaceState::UP_RUNNING;
        } else {
            return InterfaceState::DOWN_RUNNING;
        }
    } else {
        if(if_up){
            return InterfaceState::UP_NOT_RUNNING;
        } else {
            return InterfaceState::DOWN_NOT_RUNNING;
        }
    }
}

int parseInterfaceIndex(const nlmsghdr *nh){
    ifinfomsg *ifi = (ifinfomsg*) NLMSG_DATA(nh);    // get information about changed network interface
    return ifi->ifi_index;
}

std::string parseInterfaceName(const nlmsghdr *nh){
    ifinfomsg *ifi = (ifinfomsg*) NLMSG_DATA(nh);    // get information about changed network interface
    int len = nh->nlmsg_len;
    rtattr *rtattr = IFLA_RTA(ifi);

    auto rt_map = parseRtAttr(rtattr, len); // Parse all the rtattr in that nlmsg

    auto iter = rt_map.find(IFLA_IFNAME); // Find the interface name attribute

    if(iter != rt_map.end()){
        return (char*)RTA_DATA(iter->second); // Convert it to string
    }

    return "unknown";
}


std::map<int, struct rtattr*> parseRtAttr(struct rtattr *rtattr, int len){
    auto rt_attr_map = std::map<int, struct rtattr*>();

    while (RTA_OK(rtattr, len)) {  // while not end of the message
        rt_attr_map[rtattr->rta_type] = rtattr;
        rtattr = RTA_NEXT(rtattr,len);    // get next attr
    }
    return rt_attr_map;
}
} // namespace network_monitor::utils


