#ifndef SCAG_RE_ACTIONS_IMPL_PERSCALL_H
#define SCAG_RE_ACTIONS_IMPL_PERSCALL_H

#include "scag/re/base/PersCallParams.h"
#include "scag/pvss/api/core/client/Client.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/ProfileResponse.h"

namespace scag2 {
namespace re {
namespace actions {

class PersCall : public lcm::PersCallParams, public pvss::core::client::Client::ResponseHandler
{
public:
    PersCall( pvss::ProfileRequest* req );

    inline pvss::ProfileKey*   getProfileKey() { return req_.get() ? &req_->getProfileKey() : 0; }
    inline pvss::ProfileRequest*      getRequest() { return req_.get(); }
    inline pvss::ProfileResponse*     getResponse() {
        CHECKMAGTC;
        return resp_.get(); 
    }
    inline const pvss::PvssException& getException() {
        CHECKMAGTC;
        return exc_; 
    }

    /// these two methods are invoked from pvss::ClientCore
    /// NOTE: they pass pers call back to lcm initiator.
    virtual void handleResponse( std::auto_ptr< pvss::Request > request, std::auto_ptr< pvss::Response > response );
    virtual void handleError( const pvss::PvssException& exc, std::auto_ptr< pvss::Request > request );

protected:
    /// this method is invoked when long call is failed.
    /// contrary to handleError it does not pass pers call to lcm initiator
    virtual void setError( const pvss::PvssException& exc, std::auto_ptr< pvss::Request > request );

    virtual bool doCallPvss();

private:
    DECLMAGTC(PersCall);
    pvss::PvssException                  exc_;
    std::auto_ptr<pvss::ProfileRequest>  req_;
    std::auto_ptr<pvss::ProfileResponse> resp_;
};

} // namespace actions
} // namespace re
} // namespace scag2

#endif /* !SCAG_RE_ACTIONS_IMPL_PERSCALL_H */
