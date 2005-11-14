#include "PerfSocketServer.h"
#include "util/Exception.hpp"

namespace scag {
namespace stat {

using smsc::util::Exception;

PerfSocketServer::PerfSocketServer()
    : perfHost(""), perfGenPort(0), perfSvcPort(0), perfScPort(0),
      performanceServer(0), isStopping(true)
{
}

PerfSocketServer::~PerfSocketServer()
{
}

int PerfSocketServer::Execute()
{
    smsc::core::network::Multiplexer::SockArray ready, err;
    while(!isStopping)
    {
        if(listener.canRead(ready, err, 10)){
            for(int i=0; i<= ready.Count() - 1; i++){
                if(ready[i] == &genSocket){
                    sockaddr_in addrin;
                    int sz=sizeof(addrin);
                    SOCKET s=accept(genSocket.getSocket(),(sockaddr*)&addrin,&sz);
                    if(s != -1){
                        Socket * sock = new Socket(s,addrin);
                        performanceServer->addGenSocket(sock);
                    }
                }if(ready[i] == &svcSocket){
                    sockaddr_in addrin;
                    int sz=sizeof(addrin);
                    SOCKET s=accept(svcSocket.getSocket(),(sockaddr*)&addrin,&sz);
                    if(s != -1){
                        Socket * sock = new Socket(s,addrin);
                        performanceServer->addSvcSocket(sock);
                    }
                }if(ready[i] == &scSocket){
                    sockaddr_in addrin;
                    int sz=sizeof(addrin);
                    SOCKET s=accept(scSocket.getSocket(),(sockaddr*)&addrin,&sz);
                    if(s != -1){
                        Socket * sock = new Socket(s,addrin);
                        performanceServer->addScSocket(sock);
                    }
                }
            }
        }
    }

    return 1;
}

void PerfSocketServer::init(PerformanceServer * perfServer)
{
    performanceServer = perfServer;
}

void PerfSocketServer::InitServer(std::string perfHost_, int perfGenPort_, int perfSvcPort_, int perfScPort_)
{
    perfHost = perfHost_;
    perfGenPort = perfGenPort_;
    perfSvcPort = perfSvcPort;
    perfScPort = perfScPort_;

    if(genSocket.InitServer(perfHost.c_str(), perfGenPort, 10))
        throw Exception("Failed to init socket server by host: %s, port: %d", perfHost.c_str(), perfGenPort);
    if(svcSocket.InitServer(perfHost.c_str(), perfSvcPort, 10))
        throw Exception("Failed to init socket server by host: %s, port: %d", perfHost.c_str(), perfSvcPort);
    if(scSocket.InitServer(perfHost.c_str(), perfScPort, 10))
        throw Exception("Failed to init socket server by host: %s, port: %d", perfHost.c_str(), perfScPort);

    if(!listener.add(&genSocket))
        throw Exception("Failed to init PerfSocketServer");
    if(!listener.add(&svcSocket))
        throw Exception("Failed to init PerfSocketServer");
    if(!listener.add(&scSocket))
        throw Exception("Failed to init PerfSocketServer");
}

void PerfSocketServer::Stop()
{
    genSocket.Abort();
    svcSocket.Abort();
    scSocket.Abort();
    isStopping = true;
}

void PerfSocketServer::Start()
{
    isStopping = false;
    Thread::Start();
}

}
}
