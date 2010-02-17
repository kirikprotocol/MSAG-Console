#include "ContextQueue.h"
#include "ServerContext.h"

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


void ContextQueue::requestReceived( std::auto_ptr<ServerContext>& context ) /* throw (PvssException) */ 
{
    if ( !acceptRequests_ ) {
        throw PvssException(PvssException::SERVER_BUSY,"queue is shutdown");
    }
    if ( queueLimit_ > 0 && getSize() > queueLimit_ ) {
        throw PvssException(PvssException::SERVER_BUSY,"too many requests, try later");
    }
    if ( context->getRequest()->hasTiming() ) {
        char buf[50];
        sprintf(buf,"reqRecv(qsz=%u)",getSize());
        context->getRequest()->timingMark(buf);
    }
    context->setRespQueue(*this);
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
    Response* resp = context->getResponse().get();
    if ( resp && resp->hasTiming() ) {
        Request* req = context->getRequest().get();
        if ( req ) {
            req->mergeTiming(*resp);
        }
        char buf[50];
        sprintf(buf,"respRecv(qsz=%u)",queues_[0].Count());
        req->timingMark(buf);
    }
    MutexGuard mg(queueMon_);
    queues_[0].Push(context.release());
    queueMon_.notify();
}


ServerContext* ContextQueue::getContext( int tmo )
{
    ServerContext* res = 0;
    MutexGuard mg(queueMon_);
    while ( true ) {
        if ( queues_[0].Pop(res) ) break;
        if ( queues_[1].Pop(res) ) break;
        if ( ! started_ ) break;
        if ( ! acceptRequests_ ) {
            queueMon_.wait(100);
            queues_[0].Pop(res);
            break;
        }
        if (tmo>0) {
            queueMon_.wait(tmo);
            if (!queues_[0].Pop(res)) { queues_[1].Pop(res); }
            break;
        }
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
