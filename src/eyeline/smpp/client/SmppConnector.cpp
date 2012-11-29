#include "SmppConnector.h"
#include "eyeline/smpp/transport/SocketBinder.h"
#include "eyeline/smpp/transport/SocketCloser.h"
#include "eyeline/smpp/SmppException.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {


SmppConnector::SmppConnector( SocketCloser& closer, SocketBinder& binder ) :
log_(smsc::logger::Logger::getInstance("smpp.conn")),
socketCloser_(closer),
socketBinder_(binder),
stopping_(true),
mul_(wakePipe_.getR())
{
}

SmppConnector::~SmppConnector()
{
    stop();
}


void SmppConnector::start()
{
    if (!stopping_) return;
    MutexGuard mg(mon_);
    if (!stopping_) return;
    smsc_log_debug(log_,"starting");
    stopping_ = false;
    Start();
}


void SmppConnector::stop()
{
    if (stopping_) return;
    {
        MutexGuard mg(mon_);
        if (stopping_) return;
        smsc_log_debug(log_,"stopping");
        stopping_ = true;
        wakePipe_.write("w",1);
        mon_.notify();
    }
    WaitFor();
}


void SmppConnector::addSocket( ClientSocket& socket )
{
    if ( socket.isConnected() ) {
        throw SmppException("addSocket exc: Sk%u is already connected",
                            socket.getSocketId());
    }
    do {
        try {
            socket.connect();
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"Sk%u connect exc: %s",
                          socket.getSocketId(),e.what());
            break;
        }
        MutexGuard mg(mon_);
        if (!stopping_) {
            pendingSockets_.push_back( SocketPtr() );
            pendingSockets_.back().reset(&socket);
            wakePipe_.write("w",1);
            mon_.notify();
            return;
        }
    } while (false);
    // we were stopped
    socketCloser_.terminateSocket(socket);
}


int SmppConnector::Execute()
{
    smsc::core::network::Multiplexer::SockArray ready, error;
    smsc_log_debug(log_,"started");
    while ( true ) {
        {
            MutexGuard mg(mon_);
            if (stopping_) break;
            if ( !pendingSockets_.empty() ) {
                for ( SocketList::iterator i = pendingSockets_.begin(),
                      ie = pendingSockets_.end(); i != ie; ++i ) {
                    SocketPtr* ptr = sockets_.GetPtr(i->get()->getSocketId());
                    if (!ptr) {
                        sockets_.Insert(i->get()->getSocketId(), *i);
                        i->get()->addTo( mul_ );
                        smsc_log_debug(log_,"Sk%u added to mul",i->get()->getSocketId());
                    }
                }
                pendingSockets_.clear();
                wakePipe_.write("w",1);
                mon_.notify();
                continue;
            }

            if (sockets_.Count() == 0) {
                // waiting on new sockets
                mon_.wait(300);
                continue;
            }
        }
        
        // check multiplexer for writing?
        int res = mul_.canWrite(ready,error,200);
        if ( res == 0 ) {
            continue;
        } else if ( res < 0 ) {
            char buf[20];
            wakePipe_.read(buf,sizeof(buf));
        }

        for ( int i = 0; i < error.Count(); ++i ) {
            smsc::core::network::Socket* s = error[i];
            Socket* sock = Socket::fromSocket(s);
            if (!sock) continue;
            smsc_log_debug(log_,"Sk%u has error",sock->getSocketId());
            // remove from mul_ and sockets_
            sock->removeFrom(mul_);
            SocketPtr ptr(sock);
            sockets_.Delete(sock->getSocketId());
            socketCloser_.terminateSocket(*sock);
        }

        for ( int i = 0; i < ready.Count(); ++i ) {
            smsc::core::network::Socket* s = ready[i];
            Socket* sock = Socket::fromSocket(s);
            if (!sock) continue;
            smsc_log_debug(log_,"Sk%u is ready to write",sock->getSocketId());
            // remove from mul_ and sockets_
            sock->removeFrom(mul_);
            SocketPtr ptr(sock);
            sockets_.Delete(sock->getSocketId());
            try {
                socketBinder_.socketIsReadyToBind(*sock);
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"Sk%u readyToBind exc: %s",
                              sock->getSocketId(), e.what());
            }
        }
    }

    // finishing
    smsc_log_debug(log_,"finishing...");
    SocketList sl;
    {
        MutexGuard mg(mon_);
        sl.swap(pendingSockets_);
    }
    for ( SocketList::iterator i = sl.begin(), ie = sl.end(); i != ie; ++i ) {
        socketCloser_.terminateSocket(**i);
    }
    SocketPtr ptr;
    int si;
    for ( SocketHash::Iterator i = sockets_.First(); i.Next(si,ptr); ) {
        socketCloser_.terminateSocket(*ptr);
    }
    sockets_.Empty();
    smsc_log_debug(log_,"finished");
    return 0;
}

}
}
