#ifndef BillingManagerConfig_dot_h
#define BillingManagerConfig_dot_h

#include "scag/config/ConfigView.h"


namespace scag {
namespace config {

class BillingManagerConfig
{
public:
    std::string so_dir;

    std::string BillingHost;
    int BillingPort;
    int BillingTimeOut;
    int MaxThreads;


    BillingManagerConfig() { };
    BillingManagerConfig(ConfigView& cv) throw(ConfigException);
    void init(ConfigView& cv) throw(ConfigException);   
    bool check(ConfigView& cv) throw(ConfigException);
};

}
}

#endif
