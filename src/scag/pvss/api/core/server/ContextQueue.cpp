#include "ContextQueue.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

using namespace smsc::core::synchronization;

ContextQueue::ContextQueue( int queueLimit ) :
log_(smsc::logger::Logger::getInstance("pvss.queue")),
acceptRequests_(true),
started_(true),
queueLimit_(queueLimit)
{}


void ContextQueue::requestReceived( std::auto_ptr<ServerContext>& context ) throw (PvssException)
{
    if ( !acceptRequests_ ) {
        throw PvssException(PvssException::SERVER_BUSY,"queue is shutdown");
    }
    if ( queueLimit_ > 0 && getSize() > queueLimit_ ) {
        throw PvssException(PvssException::SERVER_BUSY,"too many requests, try later");
    }
    MutexGuard mg(queueMon_);
    queues_[1].Push(context.release());
    queueMon_.notify();
}


void ContextQueue::reportResponse( std::auto_ptr<ServerContext>& context )
{
    if (!started_) {
        smsc_log_error(log_,"logic failure: response arrived when queue is stopped!");
        abort();
    }
    MutexGuard mg(queueMon_);
    queues_[0].Push(context.release());
    queueMon_.notify();
}


ServerContext* ContextQueue::getContext()
{
    ServerContext* res = 0;
    MutexGuard mg(queueMon_);
    while ( true ) {
        if ( queues_[0].Pop(res) ) break;
        if ( queues_[1].Pop(res) ) break;
        if ( ! started_ ) break;
        queueMon_.wait();
    }
    return res;
}


bool ContextQueue::couldHaveRequests() const
{
    if ( acceptRequests_ ) return true;
    MutexGuard mg(queueMon_);
    if ( acceptRequests_ ) return true;
    if ( queues_[1].Count() > 0 ) return true;
    return false;
}


void ContextQueue::startup()
{
    MutexGuard mg(queueMon_);
    started_ = true;
    acceptRequests_ = true;
}


void ContextQueue::stop()
{
    MutexGuard mg(queueMon_);
    acceptRequests_ = false;
    queueMon_.notify();
}


void ContextQueue::shutdown()
{
    MutexGuard mg(queueMon_);
    acceptRequests_ = false;
    started_ = false;
    queueMon_.notify();
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
