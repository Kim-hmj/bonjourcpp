/***
    This file is part of snapcast
    Copyright (C) 2014-2020  Johannes Pohl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/

#include <cstdlib>
#include <thread>

#include "publish_bonjour.hpp"
#include <iostream>
#include <mutex>
#include <fstream>
using namespace std;

typedef union
{
    unsigned char b[2];
    unsigned short NotAnInteger;
} Opaque16;


PublishBonjour::PublishBonjour(const std::string& serviceName, boost::asio::io_context& ioc) : PublishmDNS(serviceName, ioc), active_(false)
{
    ///	dns-sd -R Snapcast _snapcast._tcp local 1704
    ///	dns-sd -R Snapcast _snapcast-jsonrpc._tcp local 1705
}


PublishBonjour::~PublishBonjour()
{
    active_ = false;
    pollThread_.join();
    for (auto client : clients)
    {
        if (client)
            DNSServiceRefDeallocate(client);
    }
}



std::string PublishBonjour::read_sn(){
    std::string get_sn;

    ifstream myfile("/persist/factory/product-message/lvp15.txt");
    if (!myfile.is_open()){
        cout << "open failed " << endl;
        return "Snapcast";
    }
    else{
            cout << "open SN_FILE success " << "\n" <<endl;
            getline(myfile,get_sn);
            if(get_sn.empty()){
                cout << "No____SN "<< "\n" <<endl;
                return "Snapcast";
            }else{
                get_sn = get_sn.substr(10,21);
                return get_sn;
            }
        }
    myfile.close();
}

void PublishBonjour::worker()
{
    //    int dns_sd_fd  = client ? DNSServiceRefSockFD(client) : -1;
    // 1. Set up the fd_set as usual here.
    // This example client has no file descriptors of its own,
    // but a real application would call FD_SET to add them to the set here
    fd_set readfds;
    FD_ZERO(&readfds);

    std::vector<int> dns_sd_fds;
    int nfds = -1;
    for (size_t n = 0; n < clients.size(); ++n)
    {
        int dns_sd_fd = DNSServiceRefSockFD(clients[n]);
        dns_sd_fds.push_back(dns_sd_fd);
        if (nfds < dns_sd_fd)
            nfds = dns_sd_fd;
        // 2. Add the fd for our client(s) to the fd_set
        FD_SET(dns_sd_fd, &readfds);
    }
    ++nfds;

    // 3. Set up the timeout.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100 * 1000;

    active_ = true;
    while (active_)
    {
        FD_ZERO(&readfds);
        for (size_t n = 0; n < dns_sd_fds.size(); ++n)
            FD_SET(dns_sd_fds[n], &readfds);

        int result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
        if (result > 0)
        {

            for (size_t n = 0; n < dns_sd_fds.size(); ++n)
            {
                if (clients[n] && FD_ISSET(dns_sd_fds[n], &readfds))
                {
                    DNSServiceErrorType err = DNSServiceProcessResult(clients[n]);
                    if (err)
                    {
                        cout << "DNSServiceProcessResult returned " << err << "\n";
                        active_ = false;
                    }
                }
            }
        }
        //		else if (result == 0)
        //			myTimerCallBack();
        else if (result < 0)
        {
            cout << "select() returned " << result << " errno " << errno << " " << strerror(errno) << "\n";
            if (errno != EINTR)
                active_ = false;
        }
    }
}


static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode, const char* name, const char* regtype,
                                const char* domain, void* context)
{
    (void)sdref; // Unused
    (void)flags; // Unused

    PublishBonjour* publishBonjour = (PublishBonjour*)context;
    (void)publishBonjour; // unused

    cout << "Got a reply for service " << name << "." << regtype << domain << "\n";

    if (errorCode == kDNSServiceErr_NoError)
    {
        if (flags & kDNSServiceFlagsAdd)
            cout << "Name now registered and active\n";
        else
            cout << "Name registration removed\n";
    }
    else if (errorCode == kDNSServiceErr_NameConflict)
    {
        /// TODO: Error handling
        cout << "Name in use, please choose another\n";
        exit(-1);
    }
    else
        cout << "Error " << errorCode << "\n";

    if (!(flags & kDNSServiceFlagsMoreComing))
        fflush(stdout);
}


void PublishBonjour::publish(const std::vector<mDNSService>& services)
{
    // serviceName_ = read_sn();
    for (auto service : services)
    {
        cout<< service.name_<<endl;
        cout<< service.port_<<endl;
        DNSServiceFlags flags = 0;
        Opaque16 registerPort = {{static_cast<unsigned char>(service.port_ >> 8), static_cast<unsigned char>(service.port_ & 0xFF)}};
        DNSServiceRef client = NULL;
        // DNSServiceRegister(&client, flags, kDNSServiceInterfaceIndexAny, serviceName_.c_str(), service.name_.c_str(), NULL, NULL, registerPort.NotAnInteger,
        // service.txt_.size(), service.txt_.empty()?NULL:service.txt_.c_str(), reg_reply, this);
        DNSServiceRegister(&client, flags, kDNSServiceInterfaceIndexAny, serviceName_.c_str(), service.name_.c_str(), NULL, NULL, registerPort.NotAnInteger, 0,
                           NULL, reg_reply, this);
        clients.push_back(client);
    }

    pollThread_ = std::thread(&PublishBonjour::worker, this);
}
