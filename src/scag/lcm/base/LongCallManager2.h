/* $Id$ */

#ifndef SCAG_LCM_CLIENT2_H
#define SCAG_LCM_CLIENT2_H


namespace scag2 {

namespace pvss {
namespace core {
namespace client {
class Client;
}
}
}

namespace lcm {

class LongCallContextBase;

class LongCallManager 
{

public:
    static LongCallManager& Instance();

    virtual ~LongCallManager();
    virtual bool call( LongCallContextBase* context) = 0;
    virtual void shutdown() = 0;

    /// reference to a pvss client
    virtual pvss::core::client::Client& pvssClient() = 0;

protected:
    LongCallManager();
};

}}

#endif
