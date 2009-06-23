#include "Core.h"
#include "SocketBase.h"

using namespace smsc::core::synchronization;

namespace {
smsc::core::synchronization::Mutex logMtx;
}

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

smsc::logger::Logger* SocketBase::log_ = 0;

SocketBase::SocketBase( Core& core ) :
core_(core),
refCount_(0),
queue_(new smsc::core::buffers::CyclicQueue< WriteContext* >)
{
    if ( !log_ ) {
        MutexGuard mg(::logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("ewall.sock");
    }
}

/// attach/detach a socket: refcounting
void SocketBase::attach( const char* who )
{
    smsc_log_debug(log_,"attaching socket %p to %s", this, who);
    MutexGuard mg(refMutex_);
    ++refCount_;
}


void SocketBase::detach( const char* who )
{
    smsc_log_debug(log_,"detaching socket %p from %s", this, who);
    bool destroy;
    {
        MutexGuard mg(refMutex_);
        if (refCount_ > 0) {
            --refCount_;
        } else {
            smsc_log_warn(log_,"logics failure: refcount=0 before detach of %p", this);
        }
        destroy = ( refCount_ == 0 );
    }
    if ( destroy ) {
        delete this;
    }
}


unsigned SocketBase::attachCount() {
    MutexGuard mg(refMutex_);
    return refCount_;
}


SocketBase::~SocketBase()
{
    // checking precondition!
    assert( refCount_ == 0 );
    smsc::core::buffers::CyclicQueue< WriteContext* >* prevQueue = 0;
    {
        MutexGuard mg(queueMon_);
        std::swap(queue_,prevQueue);
    }
    // cleanup
    WriteContext* ctx;
    while ( prevQueue->Pop(ctx) ) {
        core_.reportPacket(*this, ctx->getSeqNum(), ctx->popContext(), Context::FAILED);
        delete ctx;
    }
    delete prevQueue;
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
