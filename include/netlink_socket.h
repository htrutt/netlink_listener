#pragma once

#include <string>
#include <vector>

#include "utils.h"

namespace network_monitor{

struct NetlinkMessage {
    nlmsghdr hdr;
};

struct NetlinkIfinfomsgMessage : public NetlinkMessage {
    ifinfomsg msg;
};


class NetlinkSocket {
 public:
    NetlinkSocket();
    ~NetlinkSocket();

    NetlinkSocket(const NetlinkSocket&) = delete;
    NetlinkSocket(NetlinkSocket&&) = delete;

    void startListening();
    void stopListening();

    void getAllInterfaces();
    void sendRequest(NetlinkMessage *request);
    std::vector<network_monitor::utils::Interface> getResponse();


private:
    void bind_to_socket(unsigned int flags = 0);

    int netlink_fd_;
    int sequence_number_=0;
};

}