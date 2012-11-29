#include "SmppWriter.h"
#include "SocketCloser.h"
// #include "eyeline/smpp/Session.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {


void SmppWriter::wakeUp()
{
    smsc_log_debug(log_,"awaken");
    MutexGuard mg(mon_);
    wakePipe_.write("w",1);
    hasData_ = true;
    mon_.notify();
}


int SmppWriter::Execute()
{
    smsc_log_debug(log_,"started");
    typedef smsc::core::network::Multiplexer::SockArray SockArray;
    SockArray ready, error;
    bool skipwait = false; // if wait on mon should be skipped
    PendingQueue pq;
    while ( true ) {

        bool hasNullSocket = false;
        {
            MutexGuard mg(mon_);
            swapPendingSockets(pq);
            if ( isStopping ) {
            } else if ( hasData_ ) {
                // was awaken
                hasData_ = false;
            } else if ( pq.Count() > 0 ) {
            } else if ( !skipwait ) {
                mon_.wait(100);
                // continue;
            }
        }

        if (pq.Count() > 0) {
            handlePendingSockets(pq);
        }

        if ( isStopping ) {
            bool haveData = false;
            for ( std::vector< SocketPtr >::iterator i = sockets_.begin(), ie = sockets_.end();
                  i != ie; ++i ) {
                if (!i->get()) {
                    hasNullSocket = true;
                    continue;
                }
                haveData |= i->get()->setShutdown();
            }
            if (!haveData) {
                // no socket has data to be sent
                break;
            }
        }
               
        smsc_log_debug(log_,"passed to fill mul socks=%u",
                       unsigned(sockets_.size()));

        mul_.clear();
        for ( std::vector< SocketPtr >::iterator i = sockets_.begin(), ie = sockets_.end();
              i != ie; ++i ) {
            if ( !*i ) {
                hasNullSocket = true;
                continue;
            }
            Socket* sock = i->get();
            if ( !sock->isConnected() ) {
                // detach the socket
                i->reset(0);
                hasNullSocket = true;
                continue;
            }

            if (sock->hasWriteData()) {
                sock->addTo(mul_);
            } else if (sock->isShutdown()) {
                // shutdown state but there is no more data
                smsc_log_debug(log_,"Sk%u is at shutdown",sock->getSocketId());
                if (!sock->setShutdown()) {
                    // still has some data
                    smsc_log_debug(log_,"Sk%u still has some data",sock->getSocketId());
                    sock->addTo(mul_);
                } else {
                    // simply remove the socket
                    i->reset(0);
                    hasNullSocket = true;
                }
            }
        }

        if ( hasNullSocket ) {
            sockets_.erase( std::remove(sockets_.begin(),
                                        sockets_.end(),
                                        SocketPtr()));
        }

        if ( !mul_.getSize() ) {
            // no socket has anything to write
            continue;
        }

        smsc_log_debug(log_,"passed to canwrite socks=%u mul=%u",
                       unsigned(sockets_.size()),unsigned(mul_.getSize()));

        int res = mul_.canWrite(ready,error,200);
        if (!res) {
            // timeout, skip wait on monitor as we already waited on mul
            skipwait = true;
            continue;
        } else if ( res<0 ) {
            char buf[20];
            wakePipe_.read(buf,sizeof(buf));
        }

        // processing errors
        smsc_log_debug(log_,"error=%u ready=%u",
                       unsigned(error.Count()),unsigned(ready.Count()));
        for ( int i = 0; i < error.Count(); ++i ) {
            smsc::core::network::Socket* s = error[i];
            Socket* sock = Socket::fromSocket(s);
            if ( sock ) {
                socketCloser_.terminateSocket(*sock);
                innerRemoveSocket(*sock);
            }
        }

        skipwait = false;
        for ( int i = 0; i < ready.Count(); ++i ) {
            smsc::core::network::Socket* s = ready[i];
            Socket* sock = Socket::fromSocket(s);
            if (!sock) { continue; }
            try {
                // socket may still has some data to send
                if ( sock->sendData() ) {
                    // the socket still has some data, skip wait
                    skipwait |= true;
                } else if ( sock->isShutdown() && sock->setShutdown() ) {
                    // everything was read from socket
                    innerRemoveSocket(*sock);
                }
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"Sk%u send exc: %s",sock->getSocketId(),e.what());
                socketCloser_.terminateSocket(*sock);
                innerRemoveSocket(*sock);
            }
        }
    }
    smsc_log_debug(log_,"finished");
    return 0;
}


void SmppWriter::postSetSocket( bool add, Socket& sock )
{
    if (add) {
        sock.setWriter(this);
    } else {
        sock.setWriter(0);
    }
}


void SmppWriter::innerRemoveSocket( Socket& sock )
{
    smsc_log_debug(log_,"innerremove Sk%u",sock.getSocketId());
    std::vector< SocketPtr >::iterator i = 
        std::find(sockets_.begin(),sockets_.end(),&sock);
    if ( i != sockets_.end() ) {
        sockets_.erase(i);
    }
}

}
}
