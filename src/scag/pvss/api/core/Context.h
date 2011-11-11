#ifndef _SCAG_PVSS_CORE_CONTEXT_H
#define _SCAG_PVSS_CORE_CONTEXT_H

#include <cassert>
#include <memory>
#include "logger/Logger.h"
#include "scag/util/Time.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/Response.h"
#include "informer/io/EmbedRefPtr.h"

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
        smsc::core::synchronization::MutexGuard mg(reflock_);
        ++ref_;
    }
    void unref()
    {
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
            if (--ref_) return;
        }
        delete this;
    }

private:
    smsc::core::synchronization::Mutex reflock_;
    unsigned                           ref_;

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
