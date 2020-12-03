#include "get_interface_command.h"
#include <iostream>

namespace network_monitor::command{

GetInterfaceCommand::GetInterfaceCommand(){
    memset(&request_, 0, sizeof(request_));
    request_.hdr.nlmsg_pid = getpid();
    request_.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg));
    request_.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;  // request and dump all infos
    request_.hdr.nlmsg_type = RTM_GETADDR;
}

void GetInterfaceCommand::execute(){
    netlink_socket_.sendRequest(&request_);

    auto interfaces = netlink_socket_.getResponse();
    for(const auto& interface : interfaces){
        std::cout << "["<<interface.index<<"] : " <<interface.name<< " " << interface.address << " " << interface.state << std::endl;
    }
}

}