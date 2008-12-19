#ifndef _SCAG_PERS_UTIL_PERSCLIENT2_H
#define _SCAG_PERS_UTIL_PERSCLIENT2_H

// #include "scag/re/base/LongCallContextBase.h"
// #include "scag/config/base/ConfigManager2.h"
#include "PersCallParams.h"

namespace scag2 {
namespace pers {
namespace util {

class PersClient
{
public:
    static PersClient& Instance();

    static void Init( const char* host,
                      int port,
                      int timeout,
                      int pingTimeout,
                      int reconnectTimeout,
                      unsigned maxCallsCount,
                      unsigned clients,
                      bool async = true );

    // static void Init( const config::PersClientConfig& cfg );

    /// tries to put the call to the queue
    /// if the call cannot be processed, return false
    virtual bool callAsync( PersCall* call, PersCallInitiator& fromwho ) = 0;

    /// tries to execute the call.
    /// if the call cannot be processed, return false
    virtual bool callSync( PersCall* call ) = 0;

    virtual int getClientStatus() = 0;
    virtual void Stop() = 0;

protected:
    inline void setNext( PersCall* c, PersCall* n ) const { c->next_ = n; }
    inline void setInitiator( PersCall* c, PersCallInitiator* i ) const { c->initiator_ = i; }
    
    PersClient() {}
    virtual ~PersClient() {}
};

}
}
}

#endif /* ! _SCAG_PERS_UTIL_PERSCLIENT2_H */
