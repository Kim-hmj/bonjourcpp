#ifndef __CONTROLLER_H_
#define __CONTROLLER_H_

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include "server_settings.hpp"
using namespace std;

class Controller
{
public:
    Controller();
    ~Controller();

    void Start();
    void Stop();
    std::thread recv_Thread_;
    ServerSettings::Tcp tcp_settings_;
    ServerSettings::Http http_settings_;

};


#endif