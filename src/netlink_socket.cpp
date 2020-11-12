#include "netlink_socket.h"
#include <sys/socket.h>
#include <iostream>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>

namespace network_monitor {

NetlinkSocket::NetlinkSocket(){
    netlink_fd_ = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);   // create netlink socket

    if (netlink_fd_ < 0) {
        throw std::runtime_error("Failed to create netlink socket "+std::string(strerror(errno)));
    }

    struct sockaddr_nl  sa_local;  // local addr struct
    memset(&sa_local, 0, sizeof(sa_local));

    sa_local.nl_family = AF_NETLINK;       // set protocol family
    sa_local.nl_groups =   RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;   // set groups we interested in
    sa_local.nl_pid = getpid();    // set out id using current process id

    if (bind(netlink_fd_, (struct sockaddr*)&sa_local, sizeof(sa_local)) < 0) {     // bind socket
        close(netlink_fd_);
        throw std::runtime_error("Failed to bind netlink socket "+std::string(strerror(errno)));
    }
}

NetlinkSocket::~NetlinkSocket(){
    if(netlink_fd_ > 0){
        close(netlink_fd_);
    }
}

void NetlinkSocket::startListening(){
    // TODO : wrap this in a separate thread that can be cancelled
    listen();
}

void NetlinkSocket::listen(){
    char buf[8192];             // message buffer
    struct iovec iov;           // message structure
    iov.iov_base = buf;         // set message buffer as io
    iov.iov_len = sizeof(buf);  // set size
    struct sockaddr_nl sa;      // addr struct

    // initialize protocol message header
    struct msghdr msg {
        .msg_name = &sa,                  // local address
        .msg_namelen = sizeof(sa),        // address size
        .msg_iov = &iov,                     // io vector
        .msg_iovlen = 1                     // io size
    };


    // read and parse all messages from the netlink socket
    while (1) {
        ssize_t status = TEMP_FAILURE_RETRY(recvmsg(netlink_fd_, &msg, MSG_DONTWAIT));

        // message parser
        struct nlmsghdr *nh;
        std::string if_name, if_addr;
        InterfaceState if_state;
        for (nh = (struct nlmsghdr*)buf; NLMSG_OK (nh, status); nh = NLMSG_NEXT (nh, status)) {   // read all messagess headers
            std::cout << "New nlmsghdr" << std::endl;

            switch(nh->nlmsg_type){
                case RTM_NEWROUTE:
                case RTM_DELROUTE:
                    std::cout << "Routing table was changed" << std::endl;
                    break;
                case RTM_DELADDR:
                    std::cout << "Address was deleted" << std::endl;
                    break;
                case RTM_NEWADDR:
                    if_name = parseInterfaceName(nh);
                    if_addr = parseInterfaceIPAddress(nh);
                    std::cout << "Address was added for " << if_name << " address " << if_addr << std::endl;
                    break;
                case RTM_DELLINK:
                    std::cout << "Link was deleted" << std::endl;
                    break;
                case RTM_NEWLINK:
                    if_name = parseInterfaceName(nh);
                    if_state = parseInterfaceState(nh);
                    std::cout << "Link was added, name " << if_name << " inteface state " << if_state <<  std::endl;
                    break;
            }
        }
        usleep(250000); // sleep for a while
    }

}

std::string NetlinkSocket::parseInterfaceIPAddress(const nlmsghdr *nh){
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

InterfaceState NetlinkSocket::parseInterfaceState(const nlmsghdr *nh){
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

std::string NetlinkSocket::parseInterfaceName(const nlmsghdr *nh){
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


std::map<int, struct rtattr*> NetlinkSocket::parseRtAttr(struct rtattr *rtattr, int len){
    auto rt_attr_map = std::map<int, struct rtattr*>();

    while (RTA_OK(rtattr, len)) {  // while not end of the message
        rt_attr_map[rtattr->rta_type] = rtattr;
        rtattr = RTA_NEXT(rtattr,len);    // get next attr
    }
    return rt_attr_map;
}


}