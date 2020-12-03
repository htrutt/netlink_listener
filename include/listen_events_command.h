#pragma once

#include "ICommand.h"

namespace network_monitor::command{

class ListenEventsCommand : public BaseCommand{
    public:
        void execute() override;
    private:
        static void event_parser(nlmsghdr *nh);
};

} // namespace network_monitor::command