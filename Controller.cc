#include <csignal>
//#include <adk/log.h>
#include "Controller.h"
#include <iostream>
#include <chrono>
#include <memory>
#include <sys/resource.h>
//#include <boost/thread/thread.hpp> 
//#include <boost/thread/mutex.hpp>
#include <vector>
#include "publish_mdns.hpp"
//#include <boost/asio.hpp>
#include <memory>
using namespace std;
//using boost::asio::ip::tcp;

//using namespace boost::asio;
Controller::Controller() {
}
Controller::~Controller() = default;
void Controller::Start()
{

    // io_service iosev;
    // tcp::acceptor acceptor(iosev, ip::tcp::endpoint(tcp::v4(), 1000));
    // cout << "ipv4 ="<<endl; 
    // for(;;)
    // {
    //     tcp::socket socket(iosev);
    //     acceptor.accept(socket);
    //     std::cout << socket.remote_endpoint().address() << std::endl;
    //     boost::system::error_code ec;
    //     socket.write_some(buffer("hello world!"), ec);
    //     if(ec)
    //     {
    //         std::cout << 
    //             boost::system::system_error(ec).what() << std::endl;
    //         break;
    //     }
    // }
     cout << "in the start" << endl;
}

void Controller::Stop()
{
    printf("%d: %s\n", __LINE__, __func__);
}




int main(void)
{
//        boost::asio::io_context io_context;
        cout << "v1.1.0" << endl;
        auto publishZeroConfg = std::make_unique<PublishZeroConf>("hmj"/*, io_context*/);
        std::vector<mDNSService> dns_services;
        dns_services.emplace_back("_controller._tcp", 1000);
        publishZeroConfg->publish(dns_services);
        auto start_server = std::make_shared<Controller>();
        cout << "do not block in main?" << endl;
        start_server->Start();

}

