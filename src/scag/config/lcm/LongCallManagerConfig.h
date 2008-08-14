#ifndef LongCallManagerConfig_dot_h
#define LongCallManagerConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/base/ConfigView.h"

namespace scag {
namespace config {

class LongCallManagerConfig
{
public:

    LongCallManagerConfig() : maxThreads(10)
    {
    }

    LongCallManagerConfig(const ConfigView& cv)  throw(ConfigException);
    void init(const ConfigView& cv)  throw(ConfigException);   
    bool check(const ConfigView& cv)  throw(ConfigException);
    
    uint32_t maxThreads;
};

}
}

#endif
