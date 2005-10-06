#ifndef BillingManagerConfig_dot_h
#define BillingManagerConfig_dot_h

#include "scag/config/ConfigView.h"


namespace scag {
namespace config {

class BillingManagerConfig
{
public:
    std::string cfg_dir;
    std::string so_dir;
    BillingManagerConfig() { };
    BillingManagerConfig(ConfigView& cv) throw(ConfigException);
    void init(ConfigView& cv) throw(ConfigException);   
    bool check(ConfigView& cv) throw(ConfigException);
};

}
}

#endif
