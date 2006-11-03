#ifndef LongCallManagerConfig_dot_h
#define LongCallManagerConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class LongCallManagerConfig
{
public:

    LongCallManagerConfig() : maxThreads(10)
    {
    }

    LongCallManagerConfig(ConfigView& cv)  throw(ConfigException);
    void init(ConfigView& cv)  throw(ConfigException);   
    bool check(ConfigView& cv)  throw(ConfigException);
    
    uint32_t maxThreads;
};

}
}

#endif
