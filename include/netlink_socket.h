#pragma once

#include <string>

namespace network_monitor{



class NetlinkSocket {
 public:
    NetlinkSocket();
    ~NetlinkSocket();

    NetlinkSocket(const NetlinkSocket&) = delete;
    NetlinkSocket(NetlinkSocket&&) = delete;

    void startListening();
    void stopListening();

    void bringInterfaceUp(const std::string& if_name);
    void bringInterfaceDown(const std::string& if_name);

private:
    void bind_to_socket(unsigned int flags = 0);
    void interfaceAction(int if_action, std::string if_name);

    int netlink_fd_;
    int sequence_number_=0;
};

}