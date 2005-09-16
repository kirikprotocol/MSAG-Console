#ifndef ConfigListener_dot_h
#define ConfigListener_dot_h

#include "scag/config/ConfigManager.h"

namespace scag {
namespace config {

class ConfigListener {
private:
	ConfigType type;
protected:
	ConfigListener(ConfigType type_) : type(type_) {};
public:
	virtual ~ConfigListener(){};
	virtual void configChanged() = 0;
};

}
}

#endif
