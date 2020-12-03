#include "netlink_socket.h"

#include <sys/socket.h>
#include <iostream>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <memory>

using namespace network_monitor::utils;

namespace network_monitor {

NetlinkSocket::NetlinkSocket(){
    netlink_fd_ = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);   // create netlink socket

    if (netlink_fd_ < 0) {
        throw std::runtime_error("Failed to create netlink socket "+std::string(strerror(errno)));
    }
}

NetlinkSocket::~NetlinkSocket(){
    if(netlink_fd_ > 0){
        close(netlink_fd_);
    }
}

void NetlinkSocket::bind_to_socket(unsigned int groups){
    struct sockaddr_nl  sa_local;  // local addr struct
    memset(&sa_local, 0, sizeof(sa_local));

    sa_local.nl_family = AF_NETLINK;       // set protocol family
    sa_local.nl_groups =  groups;   // set groups we interested in
    sa_local.nl_pid = getpid();    // set out id using current process id

    if (bind(netlink_fd_, (struct sockaddr*)&sa_local, sizeof(sa_local)) < 0) {     // bind socket
        close(netlink_fd_);
        throw std::runtime_error("Failed to bind netlink socket "+std::string(strerror(errno)));
    }
}

// TODO : wrap this in a separate thread that can be cancelled
void NetlinkSocket::startListening(){
    bind_to_socket(RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE);

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
        int if_index;
        InterfaceState if_state;
        for (nh = (struct nlmsghdr*)buf; NLMSG_OK (nh, status); nh = NLMSG_NEXT (nh, status)) {   // read all messagess headers
            std::cout << "New nlmsghdr, seq: " << nh->nlmsg_seq << std::endl;

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
                    if_index = parseInterfaceIndex(nh);
                    std::cout << "Link was added, name[index] " << if_name << "[" << if_index <<"] inteface state " << if_state <<  std::endl;
                    break;
            }
        }
        usleep(250000); // sleep for a while
    }

}

void NetlinkSocket::sendRequest(NetlinkMessage *request){
    bind_to_socket();

    struct sockaddr_nl sa{};
    sa.nl_family = AF_NETLINK;
    sa.nl_pid = 0;          // kernel
    sa.nl_groups = 0;       // unicast

    struct iovec iov = { request, request->hdr.nlmsg_len };

    struct msghdr msg;
    msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };

    int ret = sendmsg(netlink_fd_, &msg, 0);
    if(ret==-1){
        std::cout << "Sending failed : " << strerror(errno) << std::endl;
    }
}

void NetlinkSocket::getAllInterfaces(){
    NetlinkIfinfomsgMessage request;
    memset(&request, 0, sizeof(request));
    request.hdr.nlmsg_pid = getpid();
    request.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg));
    request.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;  // request and dump all infos
    request.hdr.nlmsg_type = RTM_GETADDR;

    sendRequest(&request);

    auto interfaces = getResponse();
    for(const auto& interface : interfaces){
        std::cout << "["<<interface.index<<"] : " <<interface.name<< " " << interface.address << " " << interface.state << std::endl;
    }
}

std::vector<Interface> NetlinkSocket::getResponse(){
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
    std::vector<Interface> interfaces;
    while (1) {
        ssize_t status = TEMP_FAILURE_RETRY(recvmsg(netlink_fd_, &msg, MSG_DONTWAIT));

        // message parser
        Interface interface;
        struct nlmsghdr *nh;
        for (nh = (struct nlmsghdr*)buf; NLMSG_OK (nh, status); nh = NLMSG_NEXT (nh, status)) {   // read all messagess headers

            switch(nh->nlmsg_type){
                case RTM_NEWADDR:
                    interface = {
                        .name = parseInterfaceName(nh),
                        .address = parseInterfaceIPAddress(nh),
                        .state = parseInterfaceState(nh),
                        .index = parseInterfaceIndex(nh)};
                    interfaces.push_back(interface);
                    break;
                case NLMSG_DONE:
                    return interfaces;
            }
        }
    }
}

}