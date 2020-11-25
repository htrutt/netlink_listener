#include "netlink_socket.h"
#include <iostream>
#include <string>

using network_monitor::NetlinkSocket;

int main(int argc, char *argv[])
{
    std::cout << "Starting netlink monitor" << std::endl;

try{
        NetlinkSocket nl_socket;
        if(argc < 2 ){
            std::cout<< "Too few args" << std::endl;
            return EXIT_FAILURE;
        }
        if(argc == 2){
            if (std::string(argv[1]) == "listen"){
                nl_socket.startListening();
            } else if(std::string(argv[1]) == "get"){
                nl_socket.getAllInterfaces();
            }

        } else if(argc == 3){
            if(std::string(argv[1]) == "up"){
                nl_socket.bringInterfaceUp(std::string(argv[2]));
            }else if (std::string(argv[1]) == "down"){
                nl_socket.bringInterfaceDown(std::string(argv[2]));
            }
        }
    } catch (std::exception &e){
        std::cout << "Exception occurred " << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
