#include "LongCallManagerConfig.h"

namespace scag { namespace config {

LongCallManagerConfig::LongCallManagerConfig(const ConfigView& cv) throw(ConfigException)
{
    init(cv);
}

void LongCallManagerConfig::init(const ConfigView& cv)   throw(ConfigException)
{
    try {
        maxThreads = cv.getInt("maxTasks", NULL);        
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("LongCallManagerConfig.init, Unknown exception.");
    }
}

bool LongCallManagerConfig::check(const ConfigView& cv)   throw(ConfigException)
{   
    try {
        return maxThreads != cv.getInt("maxTasks", NULL);
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("LongCallManagerConfig.check, Unknown exception.");
    }
}

}}
