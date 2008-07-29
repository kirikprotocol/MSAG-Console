#ifndef ConfigListener2_dot_h
#define ConfigListener2_dot_h

#include "ConfigType.h"

namespace scag {
namespace config2 {

using namespace scag::config;

class ConfigListener {
private:
    ConfigType type;
protected:
    ConfigListener(ConfigType type_);
public:
    virtual ~ConfigListener();
    virtual void configChanged() = 0;
};

}
}

#endif
