#include <csignal>
//#include <adk/log.h>
#include "Controller.h"

#include <chrono>
#include <memory>
#include <sys/resource.h>
#include <boost/thread/thread.hpp> 
#include <boost/thread/mutex.hpp>
#include <vector>
#include "publish_mdns.hpp"

namespace adk {
namespace snap_controller {

Controller::Controller() : adk_message_service_(std::string("snap_controller")) {
}
Controller::~Controller() = default;

void Controller::Start()
{
    printf("%d: %s\n", __LINE__, __func__);
}

void Controller::Stop()
{
    printf("%d: %s\n", __LINE__, __func__);
}

ControllerMsgService::ControllerMsgService() : controller_(new Controller()) {
    control_messages_.emplace(AdkMessage::kSystemStateSwitch,
                        &ControllerMsgService::HandleSystemStateSwitch);
}

ControllerMsgService::~ControllerMsgService() = default;

bool ControllerMsgService::Start()
{
    controller_->adk_message_service_.Initialise();

    if (!controller_->adk_message_service_.Subscribe(
        AdkMessage::AdkMsgCase::kSystemStateSwitch,
        [this](AdkMessage& message) { ControllerMsgService::HandleSystemStateSwitch(message); })) {
        //ADK_LOG_ERROR("%s -> Could not Subscribe to message", __func__);
        printf("%s -> Could not Subscribe to message\n", __func__);
        return false;
    }

    return true;
}

bool ControllerMsgService::Stop()
{
    //ADK_LOG_DEBUG("%d: %s ", __LINE__, __func__);
    printf("%d: %s\n", __LINE__, __func__);

    return true;
}

void ControllerMsgService::HandleSystemStateSwitch(const AdkMessage& command)
{
    //ADK_LOG_DEBUG("%d:%s ", __LINE__, __func__);
    printf("%d: %s\n", __LINE__, __func__);

    if (command.system_state_switch().has_intent()) {
    std::string intent = command.system_state_switch().intent();

    if (!intent.compare("snapcontroller::Master")) {
        //TODO

    }
    else if(!intent.compare("snapcontroller::Slave")) {
        //TODO
    }
    else if(!intent.compare("snapcontroller::None")){
        //TODO
    }
    else if (!intent.compare("simulate::TopRecover")) {
    //   if (ReadfromFiles(IS_STREAMING_FILE_PATH, 0)) {
    //     AdkMessage messages;
    //     auto msg = messages.mutable_led_start_pattern();
    //     msg->set_pattern(top_recover_pattern_num_);
    //     priv_->message_service_.Send(messages);
    //   }
    }
  } else {
    printf("System State Switch intent not found");
  }
}

}
}

int main(void)
{
        boost::asio::io_context io_context;
        auto publishZeroConfg = std::make_unique<PublishZeroConf>("123", io_context);
        std::vector<mDNSService> dns_services;
        dns_services.emplace_back("_controller._tcp", 1704);
        dns_services.emplace_back("_controller._tcp", 1705);

        publishZeroConfg->publish(dns_services);
    sigset_t signal_set;
    if (sigemptyset(&signal_set) < 0) {
        printf("ERROR: Unable to create process signal mask");
        printf("\n");
        std::exit(EXIT_FAILURE);
    }
    if (sigaddset(&signal_set, SIGINT) < 0) {
        printf("ERROR: Unable to add to process signal mask");
        printf("\n");
        std::exit(EXIT_FAILURE);
    }
    if (sigaddset(&signal_set, SIGTERM) < 0) {
        printf("ERROR: Unable to add to process signal mask");
        printf("\n");
        std::exit(EXIT_FAILURE);
    }
    if (sigaddset(&signal_set, SIGPIPE) < 0) {
        printf("ERROR: Unable to add to process signal mask");
        printf("\n");
        std::exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_BLOCK, &signal_set, nullptr) < 0) {
        printf("ERROR: Unable to set process signal mask");
        printf("\n");
        std::exit(EXIT_FAILURE);
    }

    adk::snap_controller::ControllerMsgService SnapController;

    bool success = true;
    if ((success = SnapController.Start())) {
        bool waiting_for_signals = true;
        while (waiting_for_signals) {
            int signal_number;
            //system_wake_lock_toggle(false, "snap_controller", 0);
            int error = sigwait(&signal_set, &signal_number);
            //system_wake_lock_toggle(true, "snap_controller", 0);

            if (error) {
                printf("WARN: Error %d while waiting for process signals", error);
                printf("\n");
                success = false;
                break;
            }

            // Check which signal it was
            switch (signal_number) {
            case SIGPIPE:
                break;
            case SIGINT:
            case SIGTERM:
                // Exit loop, terminate gracefully
                waiting_for_signals = false;
                break;

            default:
                printf("ERROR: Received unexpected process signal");
                printf("\n");
                waiting_for_signals = false;
                success = false;
                break;
            }
        }
    }
    SnapController.Stop();
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

