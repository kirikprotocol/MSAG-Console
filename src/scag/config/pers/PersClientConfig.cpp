#include "PersClientConfig.h"

namespace scag { namespace config {

PersClientConfig::PersClientConfig(const ConfigView& cv) throw(ConfigException)
{
    init(cv);
}

void PersClientConfig::init(const ConfigView& cv)   throw(ConfigException)
{
    try {
        std::auto_ptr <char> h(cv.getString("host", NULL, false));
        host = h.get();
        port = cv.getInt("port", NULL);
        timeout = cv.getInt("ioTimeout", NULL);
        pingTimeout = cv.getInt("pingTimeout", NULL);
        reconnectTimeout = cv.getInt("reconnectTimeout", NULL);
        maxCallsCount = cv.getInt("maxWaitingRequestsCount", NULL);
        connections = cv.getInt("connections", NULL);
        async = cv.getBool("async", NULL);
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("PersClientConfig.init, Unknown exception.");
    }
}

bool PersClientConfig::check(const ConfigView& cv)   throw(ConfigException)
{   
    try {
        std::auto_ptr<char> h( cv.getString("host") );
        return timeout != cv.getInt("timeout", NULL) || pingTimeout != cv.getInt("pingTimeout", NULL) ||
            port != cv.getInt("port", NULL) || strcmp(host.c_str(), h.get()) ||
            reconnectTimeout != cv.getInt("reconnectTimeout", NULL) ||
            maxCallsCount != cv.getInt("maxWaitingRequestsCount", NULL) ||
            connections != cv.getInt("connections", NULL) ||
            async != cv.getBool("async", NULL);
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("PersClientConfig.check, Unknown exception.");
    }
}

}
}
