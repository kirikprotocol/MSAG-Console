#ifndef PersClientConfig_dot_h
#define PersClientConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class PersClientConfig
{
public:

    PersClientConfig(const std::string h, int p, int to, int pto):
        host(h), port(p), timeout(to), pingTimeout(pto)
    {
    }

    PersClientConfig(): port(1200), timeout(60), pingTimeout(180)
    {
    }

    PersClientConfig(const ConfigView& cv)  throw(ConfigException);
    void init(const ConfigView& cv)  throw(ConfigException);   
    bool check(const ConfigView& cv)  throw(ConfigException);

    std::string host;
    int port, timeout, pingTimeout;    
};

}
}

#endif
