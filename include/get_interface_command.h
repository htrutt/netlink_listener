#pragma once

#include "ICommand.h"

namespace network_monitor::command{

class GetInterfaceCommand : public BaseCommand{
    public:
        GetInterfaceCommand();
        void execute() override;
    private:
        NetlinkIfinfomsgMessage request_;
};

} // namespace network_monitor::command