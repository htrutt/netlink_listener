#include "netlink_socket.h"

#include <sys/socket.h>
#include <iostream>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <memory>
#include <ctime>

using namespace network_monitor::utils;

namespace network_monitor {

NetlinkSocket::NetlinkSocket(){
    netlink_fd_ = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);   // create netlink socket

    if (netlink_fd_ < 0) {
        throw std::runtime_error("Failed to create netlink socket "+std::string(strerror(errno)));
    }

    // seed once rand for sequence_id in nlmsghdr
    std::srand(std::time(0));
}

NetlinkSocket::~NetlinkSocket(){
    if(netlink_fd_ > 0){
        close(netlink_fd_);
    }
}

void NetlinkSocket::setupSocket(uint32_t events){
    struct sockaddr_nl  sa_local;  // local addr struct
    memset(&sa_local, 0, sizeof(sa_local));

    sa_local.nl_family = AF_NETLINK;       // set protocol family
    sa_local.nl_groups =  events;   // set groups we interested in
    sa_local.nl_pid = getpid();    // set out id using current process id

    if (bind(netlink_fd_, (struct sockaddr*)&sa_local, sizeof(sa_local)) < 0) {     // bind socket
        close(netlink_fd_);
        throw std::runtime_error("Failed to bind netlink socket "+std::string(strerror(errno)));
    }
}

void NetlinkSocket::listenToEvents(uint32_t events, std::function<void(nlmsghdr*)> callback){
    setupSocket(events);
    readResponse(callback);
}

void NetlinkSocket::sendRequest(NetlinkMessage *request){
    sendRequest(request, {});
}

void NetlinkSocket::sendRequest(NetlinkMessage *request, std::function<void(nlmsghdr*)> callback){
    setupSocket();

    struct sockaddr_nl sa{};
    sa.nl_family = AF_NETLINK;
    sa.nl_pid = 0;          // kernel
    sa.nl_groups = 0;       // unicast

    sequence_number_ = std::rand();
    request->hdr.nlmsg_seq = sequence_number_;

    struct iovec iov = { request, request->hdr.nlmsg_len };

    struct msghdr msg;
    msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };

    int ret = sendmsg(netlink_fd_, &msg, 0);
    if(ret==-1){
        std::cout << "Sending failed : " << strerror(errno) << std::endl;
    }

    readResponse(callback);
}

// TODO : wrap this in a separate thread that can be cancelled
// TODO : use epoll for this instead of while loop
void NetlinkSocket::readResponse(std::function<void(nlmsghdr*)> callback){
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
    struct nlmsghdr *nh;
    struct nlmsgerr* error;
    while (1) {
        ssize_t status = TEMP_FAILURE_RETRY(recvmsg(netlink_fd_, &msg, MSG_DONTWAIT));

        for (nh = (struct nlmsghdr*)buf; NLMSG_OK (nh, status); nh = NLMSG_NEXT (nh, status)) {   // read all messagess headers
            switch(nh->nlmsg_type){
                case NLMSG_DONE:
                    return;
                case NLMSG_ERROR:
                    error = static_cast<nlmsgerr*>(NLMSG_DATA(nh));
                    if(error->error != 0){ //actual error
                        throw std::runtime_error("NLMSG_ERROR " + std::string(strerror(-error->error)));
                    } // this is an ack
                    if(nh->nlmsg_seq != sequence_number_){
                        throw std::runtime_error("Unexpected sequence number in the ack");
                    }
                default:
                    if(callback){
                        callback(nh);
                    }
            }
        }
    }
}

}