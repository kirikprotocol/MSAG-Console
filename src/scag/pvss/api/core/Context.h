#ifndef _SCAG_PVSS_CORE_CONTEXT_H
#define _SCAG_PVSS_CORE_CONTEXT_H

#include <cassert>
#include <memory>
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
    Context( Request* req );

public:
    virtual ~Context();
        
public:
    util::msectime_type getCreationTime() const {
        return creationTime;
    }

    /* public synchronized */
    uint32_t getSeqNum() const {
        return const_cast<Context*>(this)->request->getSeqNum();
    }

    void setSeqNum( uint32_t seqNum ) {
        request->setSeqNum(seqNum);
    }

    /* public synchronized */
    std::auto_ptr<Request>& getRequest() {
        return request;
    }

    /* public synchronized */
    std::auto_ptr<Response>& getResponse() {
        return response;
    }

    virtual void setResponse( Response* resp ) throw (PvssException) {
        response.reset(resp);
    }

private:
    Context();
    Context( const Context& );
    Context& operator = ( const Context& );

private:
    util::msectime_type creationTime;
    std::auto_ptr<Request>  request;
    std::auto_ptr<Response> response;
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CONTEXT_H */
