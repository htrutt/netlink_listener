#pragma once

#include <string>
#include <vector>
#include <functional>

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

    /*
    *  Listen to events of type events and events will be given back to caller via callback
    *  Will throw in case of error
    */
    void listenToEvents(uint32_t events, std::function<void(nlmsghdr*)> callback);

    /*
    *  Send request without reading out response
    *  Will throw in case of error
    */
    void sendRequest(NetlinkMessage *request);
    /*
    * Send request and response will be sent to callback
    *  Will throw in case of error
    */
    void sendRequest(NetlinkMessage *request, std::function<void(nlmsghdr*)> callback);

private:
    void setupSocket(uint32_t events = 0);
    void readResponse(std::function<void(nlmsghdr*)> callback);

    int netlink_fd_=-1;
    uint32_t sequence_number_;
};

}