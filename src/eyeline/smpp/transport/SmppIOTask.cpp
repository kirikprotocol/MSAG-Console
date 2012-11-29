#include "SmppIOTask.h"
#include "eyeline/smpp/Session.h"
#include "eyeline/smpp/SmppException.h"

namespace eyeline {
namespace smpp {

SmppIOTask::SmppIOTask( const char* taskName,
                        SocketCloser& socketCloser ) :
smsc::core::threads::ThreadedTask(false),
ref_(0),
taskName_(taskName),
socketCloser_(socketCloser),
log_(smsc::logger::Logger::getInstance(taskName)),
mul_(wakePipe_.getR())
{
    if (-1 == wakePipe_.getR()) {
        throw SmppException("could not open wakePipe");
    }
    smsc_log_debug(log_,"ctor@%p",this);
}


SmppIOTask::~SmppIOTask()
{
    smsc_log_debug(log_,"dtor@%p",this);
}


void SmppIOTask::stop()
{
    smsc_log_debug(log_,"stopping...");
    smsc::core::synchronization::MutexGuard mg(mon_);
    isStopping = true;
    mon_.notify();
    wakePipe_.write("s",1);
}


void SmppIOTask::addSocket( Socket& sock )
{
    smsc_log_debug(log_,"adding Sk%u",sock.getSocketId());
    smsc::core::synchronization::MutexGuard mg(mon_);
    if (isStopping) return;
    PendingSocket& ps = pending_.Push(PendingSocket());
    ps.socket.reset(&sock);
    ps.add = true;
    mon_.notify();
    wakePipe_.write("w", 1);
}


void SmppIOTask::removeSocket( Socket& sock )
{
    smsc_log_debug(log_,"removing Sk%u",sock.getSocketId());
    smsc::core::synchronization::MutexGuard mg(mon_);
    PendingSocket& ps = pending_.Push(PendingSocket());
    ps.socket.reset(&sock);
    ps.add = false;
    mon_.notify();
}


void SmppIOTask::swapPendingSockets( PendingQueue& pq )
{
    pq.Clear();
    if ( pending_.Count() ) {
        pq.swap(pending_);
    }
}


void SmppIOTask::handlePendingSockets( PendingQueue& pq )
{
    while ( pq.Count() ) {
        PendingSocket& ps = pq.Front();
        std::vector< SocketPtr >::iterator i = 
            std::find( sockets_.begin(), sockets_.end(), ps.socket );
        if ( i != sockets_.end() ) {
            // found
            if ( !ps.add ) {
                sockets_.erase(i);
                postSetSocket(false,*ps.socket);
                smsc_log_debug(log_,"removed Sk%u",ps.socket->getSocketId());
            }
        } else {
            // not found
            if (ps.add) {
                sockets_.push_back(ps.socket);
                postSetSocket(true,*ps.socket);
                smsc_log_debug(log_,"added Sk%u",ps.socket->getSocketId());
            }
        }
        pq.Pop();
    }
}


}
}
