#ifndef _SCAG_PVSS_CORE_CONTEXT_H
#define _SCAG_PVSS_CORE_CONTEXT_H

#include <cassert>
#include <memory>
#include "logger/Logger.h"
#include "scag/util/Time.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/Response.h"
#include "informer/io/EmbedRefPtr.h"
#include "core/synchronization/AtomicCounter.hpp"

namespace scag2 {
namespace pvss {
namespace core {

class PvssSocket;

/// abstract class
class Context
{
    friend class eyeline::informer::EmbedRefPtr< Context >;
protected:
    static smsc::logger::Logger* log_;

protected:
    Context( Request* req );

public:
    virtual ~Context();
        
public:
    util::msectime_type getCreationTime() const {
        return creationTime_;
    }

    /* public synchronized */
    uint32_t getSeqNum() const {
        return seqNum_;
    }

    void setSeqNum( uint32_t seqNum ) {
        seqNum_ = seqNum;
        if ( request_.get() ) request_->setSeqNum(seqNum);
        if ( response_.get() ) response_->setSeqNum(seqNum);
    }

    /* public synchronized */
    std::auto_ptr<Request>& getRequest() {
        return request_;
    }

    /* public synchronized */
    std::auto_ptr<Response>& getResponse() {
        return response_;
    }

    virtual void setResponse( Response* resp ) /* throw (PvssException) */ ;

private:
    Context();
    Context( const Context& );
    Context& operator = ( const Context& );

protected:
    void ref()
    {
        unsigned long long tot;
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
            if (ref_==0) { total_.inc(); }
            tot = total_.get();
            ++ref_;
        }
        if ( tot && 0 == (tot % 1000) ) {
            smsc_log_info(log_,"ref: total number of Contexts: %lld",tot);
        }
    }


    void unref()
        {
            unsigned long long tot;
            {
                smsc::core::synchronization::MutexGuard mg(reflock_);
                if(--ref_) return;
                tot = total_.dec();
            }
            delete this;
            if ( tot && 0 == (tot % 1000) ) {
                 smsc_log_info(log_,"unref: total number of Contexts: %lld",tot);
            }
        }

private:
    smsc::core::synchronization::Mutex reflock_;
    unsigned                           ref_;
    static smsc::core::synchronization::AtomicCounter<int64_t> total_;

private:
    util::msectime_type     creationTime_;
    uint32_t                seqNum_;
    std::auto_ptr<Request>  request_;
    std::auto_ptr<Response> response_;
};

typedef eyeline::informer::EmbedRefPtr< Context > ContextPtr;

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CONTEXT_H */
