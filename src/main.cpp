#include "listen_events_command.h"
#include "set_interface_command.h"
#include "get_interface_command.h"
#include <iostream>
#include <string>


using network_monitor::NetlinkSocket;
using namespace network_monitor::command;

// TOOD improve arg parsing !! 
int main(int argc, char *argv[])
{
    std::cout << "Starting netlink monitor" << std::endl;

try{
        if(argc < 2 ){
            std::cout<< "Too few args" << std::endl;
            return EXIT_FAILURE;
        }
        if(argc == 2){
            if (std::string(argv[1]) == "listen"){
                ListenEventsCommand cmd;
                cmd.execute();
            } else if(std::string(argv[1]) == "get"){
                GetInterfaceCommand cmd;
                cmd.execute();
            }

        } else if(argc == 3){
            if(std::string(argv[1]) == "up"){
                SetInterfaceCommand cmd(std::string(argv[2]), InterfaceAction::UP);
                cmd.execute();
            }else if (std::string(argv[1]) == "down"){
                SetInterfaceCommand cmd(std::string(argv[2]), InterfaceAction::DOWN);
                cmd.execute();
            }
        }
    } catch (std::exception &e){
        std::cout << "Exception occurred " << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
