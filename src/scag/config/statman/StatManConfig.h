#ifndef StatManConf_dot_h
#define StatManConf_dot_h

#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class StatManConfig {
public:    
    StatManConfig();
    StatManConfig(ConfigView& cv)  throw(ConfigException);
    void init(ConfigView& cv)  throw(ConfigException);   
    bool check(ConfigView& cv)  throw(ConfigException);
protected:
        std::string dir;
};

}
}

#endif
