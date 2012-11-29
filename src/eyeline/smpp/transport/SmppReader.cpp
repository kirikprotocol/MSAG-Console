#include "SocketCloser.h"
#include "SmppReader.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {

int SmppReader::Execute()
{
    smsc_log_debug(log_,"started");
    typedef smsc::core::network::Multiplexer::SockArray SockArray;
    SockArray ready, error;
    msectime_type nextActCheck = currentTimeMillis();
    PendingQueue pq;
    while ( true ) {
        {
            MutexGuard mg(mon_);
            swapPendingSockets(pq);
            if ( isStopping ) {
                break;
            } else if ( pq.Count() == 0 && sockets_.empty() ) {
                mon_.wait(200);
                continue;
            }
        }

        if ( pq.Count() > 0 ) {
            handlePendingSockets(pq);
        }
        if ( sockets_.empty() ) { continue; }

        int res = mul_.canRead(ready,error,200);
        if (!res) {
            // timeout
            const msectime_type now = currentTimeMillis();
            if ( now >= nextActCheck ) {
                nextActCheck = checkInactivity(now);
            }
            continue;
        } else if ( res < 0 ) {
            // waked
            char buf[20];
            wakePipe_.read(buf,sizeof(buf));
        }

        // processing errors
        for ( int i = 0; i < error.Count(); ++i ) {
            smsc::core::network::Socket* s = error[i];
            Socket* sock = Socket::fromSocket(s);
            if ( sock ) {
                socketCloser_.terminateSocket(*sock);
                innerRemoveSocket(*sock);
            }
        }

        for ( int i = 0; i < ready.Count(); ++i ) {
            smsc::core::network::Socket* s = ready[i];
            Socket* sock = Socket::fromSocket(s);
            if (!sock) { continue; }
            try {
                sock->processInput();
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"Sk%u input exc: %s",sock->getSocketId(),e.what());
                socketCloser_.terminateSocket(*sock);
                innerRemoveSocket(*sock);
            }
        }

        const msectime_type now = currentTimeMillis();
        if ( now >= nextActCheck ) {
            nextActCheck = checkInactivity(now);
        }
    }
    smsc_log_debug(log_,"finished");
    return 0;
}


void SmppReader::postSetSocket( bool add, Socket& sock )
{
    if (add) {
        sock.addTo(mul_);
    } else {
        sock.removeFrom(mul_);
    }
}


std::vector< SocketPtr >::iterator SmppReader::innerRemoveSocket( Socket& sock )
{
    sock.removeFrom(mul_);
    std::vector< SocketPtr >::iterator i = std::find(sockets_.begin(),sockets_.end(),&sock);
    if ( i != sockets_.end() ) { return sockets_.erase(i); }
    return i;
}


msectime_type SmppReader::checkInactivity( msectime_type now )
{
    msectime_type res = now + inactivityLimit_;
    for ( std::vector< SocketPtr >::iterator i = sockets_.begin();
          i != sockets_.end(); ) {
        Socket* sock = i->get();
        const msectime_type next = sock->getLastReadTime() + inactivityLimit_;
        if ( next < now ) {
            smsc_log_debug(log_,"Sk%u was not sending for %u msec, now=%llu lastr=%llu next=%llu",
                           sock->getSocketId(),unsigned(now - sock->getLastReadTime()),
                           ulonglong(now), ulonglong(sock->getLastReadTime()), ulonglong(next));
            socketCloser_.terminateSocket(*sock);
            i = innerRemoveSocket(*sock);
        } else {
            if ( next < res ) { res = next; }
            ++i;
        }
    }
    return res;
}

}
}
