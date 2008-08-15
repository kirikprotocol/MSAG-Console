#ifndef BillingManagerConfig_dot_h
#define BillingManagerConfig_dot_h

#include "scag/config/base/ConfigView.h"


namespace scag {
namespace config {

class BillingManagerConfig
{
public:
    std::string cfg_dir;

    std::string BillingHost;
    int BillingPort;
    int BillingTimeOut, BillingReconnectTimeOut;
    int MaxThreads;


    BillingManagerConfig() { };
    BillingManagerConfig(const ConfigView& cv) throw(ConfigException);
    void init(const ConfigView& cv) throw(ConfigException);   
    bool check(const ConfigView& cv) throw(ConfigException);
};

}
}

namespace scag2 {
namespace config {
using scag::config::BillingManagerConfig;
}
}

#endif
