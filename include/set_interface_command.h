#pragma once

#include "ICommand.h"

namespace network_monitor::command{

enum InterfaceAction{
    DOWN,
    UP
};

class SetInterfaceCommand : public BaseCommand{
    public:
        SetInterfaceCommand(std::string interface_name, int action);
        void execute() override;
    private:
        NetlinkIfinfomsgMessage request_;
};

} // namespace network_monitor::command