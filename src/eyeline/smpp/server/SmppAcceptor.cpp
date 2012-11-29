#include "SmppAcceptor.h"
#include "eyeline/smpp/SmppException.h"
#include "eyeline/smpp/transport/SocketBinder.h"
#include "eyeline/smpp/transport/Socket.h"
#include "core/network/Multiplexer.hpp"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {

SmppAcceptor::SmppAcceptor( SocketBinder& binder,
                            const char* host,
                            int port,
                            size_t maxrsize,
                            size_t maxwqueue ) :
log_(smsc::logger::Logger::getInstance("smpp.acc")),
socketBinder_(binder),
stopping_(true),
maxrsize_(maxrsize), maxwqueue_(maxwqueue)
{
    smsc_log_debug(log_,"init server at %s:%u",host,unsigned(port));
    if (socket_.InitServer(host,port,1,0,true)) {
        // failed
        char ebuf[100];
        throw SmppException("initserver exc: err=%d %s", errno, STRERROR(errno,ebuf,sizeof(ebuf)));
    }
}


void SmppAcceptor::start()
{
    smsc_log_info(log_,"starting");
    if (!stopping_) return;
    MutexGuard mg(mon_);
    if (!stopping_) return;
    stopping_ = false;
    if (socket_.StartServer()) {
        // fail
        stopping_ = true;
    }
    Start();
}


void SmppAcceptor::stop()
{
    smsc_log_info(log_,"stopping");
    if (stopping_) return;
    {
        MutexGuard mg(mon_);
        if (stopping_) return;
        stopping_ = true;
        wakePipe_.write("w",1);
        mon_.notify();
    }
    WaitFor();
}


int SmppAcceptor::Execute()
{
    smsc_log_info(log_,"started");
    smsc::core::network::Multiplexer mul(wakePipe_.getR());
    smsc::core::network::Multiplexer::SockArray ready, error;
    mul.add(&socket_);
    while ( true ) {
        {
            MutexGuard mg(mon_);
            if (stopping_) break;
        }

        int res = mul.canRead(ready,error,1000);

        if ( res == 0 ) {
            continue;
        } else if ( res < 0 ) {
            char buf[20];
            wakePipe_.read(buf,sizeof(buf));
        }
        for ( int i = 0; i < error.Count(); ++i ) {
            if ( error[i] == &socket_ ) {
                smsc_log_error(log_,"listening socket has failed");
                break;
            }
        }
        for ( int i = 0; i < ready.Count(); ++i ) {
            if ( ready[i] == &socket_ ) {
                sockaddr_in addrin;
                socklen_t sz = sizeof(addrin);
                SOCKET s =  accept(socket_.getSocket(),
                                   (sockaddr*)&addrin,
                                   &sz);
                if ( s == -1 ) {
                    char ebuf[100];
                    smsc_log_error(log_,"accept failed: err=%u %s",errno,
                                   STRERROR(errno,ebuf,sizeof(ebuf)));
                } else {
                    SocketPtr ptr(new Socket(new smsc::core::network::Socket(s,addrin),
                                             maxrsize_, maxwqueue_));
                    try {
                        socketBinder_.socketIsReadyToBind(*ptr);
                    } catch ( std::exception& e ) {
                        smsc_log_warn(log_,"Sk%u readyToBind exc: %s",
                                      ptr->getSocketId(),
                                      e.what());
                    }
                }
                break;
            }
        }
    }
    smsc_log_info(log_,"finished");
    return 0;
}


}
}
