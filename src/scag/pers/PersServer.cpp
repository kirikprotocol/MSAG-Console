#include "PersServer.h"
#include "util/Exception.hpp"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::logger::Logger;

PersServer::PersServer()
    : log(Logger::getInstance("perssrv")), persHost(""), persPort(0), isStopping(false)
{
}

PersServer::~PersServer()
{
}

int PersServer::Execute()
{
    Multiplexer::SockArray ready, write, err;

    if(isStopping) return 1;

    if( sock.StartServer())
        smsc_log_warn(log, "Server socket can't start");

    while(!isStopped())
    {
        if(listener.canRead(ready, err)){
			if(isStopped())
				break;
            smsc_log_info(log, "There are ready sockets");
            for(int i=0; i<= ready.Count() - 1; i++){
                if(ready[i] == &sock){
                    smsc_log_info(log, "Listen Socket is ready");
                    sockaddr_in addrin;
                    int sz = sizeof(addrin);
                    SOCKET s = accept(sock.getSocket(), (sockaddr*)&addrin, &sz);
                    if(s != -1){
                        Socket * sock1 = new Socket(s, addrin);
                    }
                }
            }
        }
    }

    return 1;
}

void PersServer::InitServer(const char *persHost_, int persPort_, int maxClientCount_)
{
    persHost = persHost_;
    persPort = persPort_;
	maxClientCount = maxClientCount_;

    if(sock.InitServer(persHost.c_str(), persPort, 10))
        throw Exception("Failed to init socket server by host: %s, port: %d", persHost.c_str(), persPort);

    smsc_log_info(log, "Socket server is inited by host: %s, port: %d", persHost.c_str(), persPort);

    if(!listener.add(&sock))
        throw Exception("Failed to init PersServer");
}

}}
