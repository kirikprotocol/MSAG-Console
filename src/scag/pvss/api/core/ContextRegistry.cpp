#include "ContextRegistry.h"

using smsc::logger::Logger;
using namespace smsc::core::synchronization;

namespace scag2 {
namespace pvss {
namespace core {

smsc::logger::Logger* ContextRegistry::log_ = 0;
unsigned long long ContextRegistry::total_ = 0;

namespace {
Mutex logMtx;
}

ContextRegistry::ContextRegistry( const PvssSockPtr& socket ) :
ref_(0),
socket_(socket)
#ifdef INTHASH_USAGE_CHECKING
,map_(SMSCFILELINE)
#endif
{
    if (!log_) {
        MutexGuard mg(logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("ctxreg");
    }
    smsc_log_debug(log_,"ctor %p for channel %p sock=%p",this,socket.get(),socket->getSocket());
}


ContextRegistry::~ContextRegistry()
{
    smsc_log_debug(log_,"dtor %p for channel %p sock=%p",this,socket_.get(),socket_->getSocket());
}


bool ContextRegistry::push( Context* ctx )
{
    const uint32_t seqNum = ctx->getSeqNum();
    smsc::core::synchronization::MutexGuard mg(mon_);
    bool wasempty = list_.empty();
    if ( map_.Exist( seqNum ) ) {
        // already there
        smsc_log_debug(log_,"could not push ctx=%p seq=%u into %p channel %p",
                       ctx,seqNum,this,socket_.get());
        return false;
    }
    ProcessingList::iterator i = list_.insert(list_.end(),ContextPtr(ctx));
    map_.Insert(seqNum,i);
    if (wasempty) mon_.notify();
    if ( map_.Count() && 0 == (map_.Count() % 1000) ) {
        smsc_log_info(log_,"total number of contexts in %p channel %p: %u",
                      this,socket_.get(),unsigned(map_.Count()));
    }
    smsc_log_debug(log_,"ctx=%p seq=%u pushed into %p channel %p",
                   ctx,seqNum,this,socket_.get());
    return true;
}


ContextPtr ContextRegistry::pop( uint32_t seqNum ) 
{
    ContextPtr res;
    unsigned mapcount;
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        ProcessingList::iterator i;
        if ( !map_.Pop(seqNum,i) ) {
            return res;
        }
        mapcount = unsigned(map_.Count());
        res = *i;
        list_.erase(i);
        if ( list_.empty() ) mon_.notify();
    }
    if ( mapcount && 0 == (mapcount%1000) ) {
        smsc_log_info(log_,"total number of contexts in %p channel %p: %u",
                      this,socket_.get(),mapcount);
    }
    smsc_log_debug(log_,"ctx=%p seq=%u was popped from %p channel %p",
                  res.get(), seqNum, this, socket_.get());
    return res;
}

}
}
}
