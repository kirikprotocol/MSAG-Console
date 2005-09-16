#ifndef StatManConf_dot_h
#define StatManConf_dot_h

#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class StatManConfig {
public:    
    StatManConfig();
    StatManConfig(ConfigView& cv);
    void init(ConfigView& cv);   
    bool check(ConfigView& cv);
protected:
        std::string dir;
};

}
}

#endif
