#include "set_interface_command.h"

namespace network_monitor::command{

SetInterfaceCommand::SetInterfaceCommand(std::string interface_name, int action){
    memset(&request_, 0, sizeof(request_));
    request_.hdr.nlmsg_pid = getpid();
    request_.hdr.nlmsg_len = sizeof(request_);
    request_.hdr.nlmsg_flags = NLM_F_REQUEST;
    request_.hdr.nlmsg_type = RTM_NEWLINK;

    request_.msg.ifi_index=if_nametoindex(interface_name.c_str());
    request_.msg.ifi_family = AF_UNSPEC;
    request_.msg.ifi_flags = action == InterfaceAction::DOWN ? 0 : IFF_UP;
    request_.msg.ifi_change = 0x1;
}

void SetInterfaceCommand::execute(){
    netlink_socket_.sendRequest(&request_);
}

}