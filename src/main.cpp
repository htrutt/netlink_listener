#include "listen_events_command.h"
#include "set_interface_command.h"
#include "get_interface_command.h"
#include <iostream>
#include <memory>
#include <string>


using network_monitor::NetlinkSocket;
using namespace network_monitor::command;

void printUsage(){
    std::cout << "Simple network manager (similar to ip) using netlink socket to do its job \n"
              << "Usage : \n"
              << "netmonitor -h : shows this help \n"
              << "netmonitor set [interface-name] [up|down] \n"
              << "netmonitor get [interface-name]"
              << std::endl;
}

// TOOD improve arg parsing !! 
int main(int argc, char *argv[])
{
    std::cout << "Starting netlink monitor" << std::endl;

try{
        if(argc < 2 ){
            std::cout<< "Too few args" << std::endl;
            return EXIT_FAILURE;
        }

        std::unique_ptr<ICommand> cmd;

        if(argc == 2){
            if(std::string(argv[1]) == "-h"){
                printUsage();
                return EXIT_FAILURE;
            }
            else if (std::string(argv[1]) == "listen"){
                cmd = std::make_unique<ListenEventsCommand>();
            } else if(std::string(argv[1]) == "get"){
                cmd = std::make_unique<GetInterfaceCommand>();
            }

        } else if(argc == 3){
            if(std::string(argv[1]) == "up"){
                cmd = std::make_unique<SetInterfaceCommand>(std::string(argv[2]), InterfaceAction::UP);
            }else if (std::string(argv[1]) == "down"){
                cmd = std::make_unique<SetInterfaceCommand>(std::string(argv[2]), InterfaceAction::DOWN);
            }
        }
        cmd->execute();
    } catch (std::exception &e){
        std::cout << "Exception occurred " << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
