#pragma once

#include "netlink_socket.h"
#include <memory.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>

namespace network_monitor::command{

class ICommand{
    public:
        virtual void execute() = 0;
};

class BaseCommand : public ICommand{
    protected:
        NetlinkSocket netlink_socket_;
};

} // namespace network_monitor::command