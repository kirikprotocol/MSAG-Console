#ifndef _SCAG_PVSS_CORE_CONTEXT_H
#define _SCAG_PVSS_CORE_CONTEXT_H

#include <cassert>
#include <memory>
#include "logger/Logger.h"
#include "scag/util/Time.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/Response.h"

namespace scag2 {
namespace pvss {
namespace core {

class PvssSocket;

/// abstract class
class Context
{
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
        return request_->getSeqNum();
    }

    void setSeqNum( uint32_t seqNum ) {
        request_->setSeqNum(seqNum);
    }

    /* public synchronized */
    std::auto_ptr<Request>& getRequest() {
        return request_;
    }

    /* public synchronized */
    std::auto_ptr<Response>& getResponse() {
        return response_;
    }

    virtual void setResponse( Response* resp ) throw (PvssException);

private:
    Context();
    Context( const Context& );
    Context& operator = ( const Context& );

private:
    util::msectime_type creationTime_;
    std::auto_ptr<Request>  request_;
    std::auto_ptr<Response> response_;
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CONTEXT_H */
