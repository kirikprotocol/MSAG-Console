#include "PersClientConfig.h"

namespace scag { namespace config {

PersClientConfig::PersClientConfig(ConfigView& cv) throw(ConfigException)
{
    init(cv);
}

void PersClientConfig::init(ConfigView& cv)   throw(ConfigException)
{
    try {
        std::auto_ptr <char> h(cv.getString("host", NULL, false));
        host = h.get();
        port = cv.getInt("port", NULL);
        timeout = cv.getInt("timeout", NULL);        
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("PersClientConfig.init, Unknown exception.");
    }
}

bool PersClientConfig::check(ConfigView& cv)   throw(ConfigException)
{   
    try {
        std::auto_ptr<char> h( cv.getString("host") );
        return timeout != cv.getInt("timeout", NULL) ||
            port != cv.getInt("port", NULL) || strcmp(host.c_str(), h.get());
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("PersClientConfig.check, Unknown exception.");
    }
}

}
}
