#ifndef __CONTROLLER_H_
#define __CONTROLLER_H_

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <unistd.h>
#include <stdlib.h>

#include <adk/message-service/adk-message-service.h>

/*
class FileOperation
{
private:
    std::ofstream vappp;
public:
    enum Type {
        VALUE = 0,
        JSON,
        TYPE_MAX
    };

    FileOperation();
    ~FileOperation();

    int read();
    void write();
};
*/

using adk::msg::AdkMessageService;
using adk::msg::AdkMessage;
namespace adk {
namespace snap_controller {

class Controller
{
public:
    Controller();
    ~Controller();

    void Start();
    void Stop();

    //void worker();
    AdkMessageService adk_message_service_;

private:
    enum streaming_type {
        UNDEF = 0,
        AIRPLAY,
        SPOTIFY,
        QPLAY,
        TYPE_MAX
    };
/*
    enum control_state {
        UNDEF = 0,
        MASTER,
        SLAVE,
        STATE_MAX
    };
*/
};

class ControllerMsgService
{
public:
    ControllerMsgService();
    ~ControllerMsgService();

    bool Start();
    bool Stop();

private:
    typedef void (adk::snap_controller::ControllerMsgService::*handler)(const AdkMessage &command);
    std::shared_ptr<Controller> controller_;
    std::map<AdkMessage::AdkMsgCase, handler> control_messages_;

    void HandleSystemStateSwitch(const AdkMessage& command);
};

}
}

#endif