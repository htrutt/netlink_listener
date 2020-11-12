#include "netlink_socket.h"
#include <iostream>

using network_monitor::NetlinkSocket;

int main()
{
    std::cout << "Starting netlink monitor" << std::endl;

try{
        NetlinkSocket nl_socket = NetlinkSocket();
        nl_socket.startListening();
    } catch (std::exception &e){
        std::cout << "Exception occurred " << e.what();
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
