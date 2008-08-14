#include "ConfigManager2.h"
#include "ConfigListener2.h"

namespace scag2 {
namespace config {

ConfigListener::ConfigListener( ConfigType t ) : type(t) 
{
    ConfigManager::Instance().registerListener(t, this);
}

ConfigListener::~ConfigListener()
{
    ConfigManager::Instance().removeListener(type);
}

}
}
