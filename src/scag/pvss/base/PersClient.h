#ifndef _SCAG_PVSS_BASE_PERSCLIENT_H
#define _SCAG_PVSS_BASE_PERSCLIENT_H

#include "PersCall.h"

namespace scag2 {
namespace pvss {

class PersClient
{
public:
    virtual ~PersClient();
    static PersClient& Instance();

    /*
    static void Init( const char* host,
                      int port,
                      int timeout,
                      int pingTimeout,
                      int reconnectTimeout,
                      unsigned maxCallsCount,
                      unsigned clients,
                      bool async = true );
     */

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
    
    PersClient();
};

}
}

#endif /* ! _SCAG_PVSS_BASE_PERSCLIENT_H */
