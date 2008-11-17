#ifndef _SCAG_PERS_UTIL_PERSCLIENT2_H
#define _SCAG_PERS_UTIL_PERSCLIENT2_H

#include "scag/re/base/LongCallContextBase.h"
#include "scag/config/base/ConfigManager2.h"

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
                      bool async );

    static void Init( const config::PersClientConfig& cfg );

    /// run the proxy and return the result
    virtual bool call( lcm::LongCallContextBase* context ) = 0;
    virtual int getClientStatus() = 0;
    virtual void Stop() = 0;

protected:
    PersClient() {}
    virtual ~PersClient() {}
};

}
}
}

#endif /* ! _SCAG_PERS_UTIL_PERSCLIENT2_H */
