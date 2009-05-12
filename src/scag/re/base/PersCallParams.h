#ifndef _SCAG_RE_BASE_PERSCALLPARAMS_H
#define _SCAG_RE_BASE_PERSCALLPARAMS_H

#include "LongCallContextBase.h"

namespace scag2 {
namespace lcm {

class PersCallParams : public LongCallParams
{
public:
    // PersCallParams( pvss::Request* req ) : excType_(pvss::PvssException::UNKNOWN), req_(req), lcc_(0) {}
    PersCallParams() : lcc_(0) {}

    bool callPvss( LongCallContextBase* ctx ) {
        lcc_ = ctx;
        return doCallPvss();
    }

    /*
    pvss::ProfileKey* getProfileKey();

    std::auto_ptr< pvss::Request >& getRequest() { return req_; }
    std::auto_ptr< pvss::Response >& getResponse() { return resp_; }

    // --- client handler iface
    virtual void handleResponse( std::auto_ptr< pvss::Request > request, std::auto_ptr< pvss::Response > response ) {
        excType_ = pvss::PvssException::OK;
        req_ = request;
        resp_ = response;
        lcc_->initiator->continueExecution( lcc_, false );
    }

    virtual void handleError( const pvss::PvssException& exc, std::auto_ptr< pvss::Request > request ) {
        excType_ = exc.getType();
        exception = exc.getMessage();
        req_ = request;
        lcc_->initiator->continueExecution( lcc_, false );
    }
     */

protected:
    /// should not throw
    virtual bool doCallPvss() = 0;

private:
    PersCallParams( const PersCallParams& );
    PersCallParams& operator = ( const PersCallParams& );

protected:
    // pvss::PvssException::Type       excType_; // OK if ok
    // std::auto_ptr< pvss::Request >  req_;
    // std::auto_ptr< pvss::Response > resp_;
    LongCallContextBase*            lcc_;   // not owned
};


}
}

#endif
