#include "listen_events_command.h"
#include "utils.h"

using namespace network_monitor::utils;

namespace network_monitor::command{

void ListenEventsCommand::execute(){
    netlink_socket_.listenToEvents(RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE, ListenEventsCommand::event_parser);
}

void ListenEventsCommand::event_parser(nlmsghdr *nh){
    std::cout << "New nlmsghdr, seq: " << nh->nlmsg_seq << std::endl;
    std::string if_name, if_addr;
    int if_index, if_mtu;
    InterfaceState if_state;
    switch(nh->nlmsg_type){
        case RTM_NEWROUTE:
        case RTM_DELROUTE:
            std::cout << "Routing table was changed" << std::endl;
            break;
        case RTM_DELADDR:
            std::cout << "Address was deleted" << std::endl;
            break;
        case RTM_NEWADDR:
            if_name = parseInterfaceName(nh);
            if_addr = parseInterfaceIPAddress(nh);
            std::cout << "Address was added for " << if_name << " address " << if_addr << std::endl;
            break;
        case RTM_DELLINK:
            std::cout << "Link was deleted" << std::endl;
            break;
        case RTM_NEWLINK:
            if_name = parseInterfaceName(nh);
            if_state = parseInterfaceState(nh);
            if_index = parseInterfaceIndex(nh);
            if_mtu = parseInterfaceMtu(nh);
            std::cout << "Link was added, name[index] " << if_name << "[" << if_index <<"] inteface state " << if_state 
                    << " MTU " << if_mtu
                    <<  std::endl;
            break;
    }
}

}