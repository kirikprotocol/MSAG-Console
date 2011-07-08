#include "HttpManagerConfig.h"

namespace scag { namespace config {

HttpManagerConfig::HttpManagerConfig(const ConfigView& cv) throw(ConfigException)
{
    init(cv);
}

void HttpManagerConfig::init(const ConfigView& cv)   throw(ConfigException)
{
    try {
        std::auto_ptr <char> h(cv.getString("host", NULL, false));
        host = h.get();

        readerSockets = cv.getInt("readerSockets", NULL);
        writerSockets = cv.getInt("writerSockets", NULL);
        readerPoolSize = cv.getInt("readerPoolSize", NULL);
        writerPoolSize = cv.getInt("writerPoolSize", NULL);
        scagPoolSize = cv.getInt("scagPoolSize", NULL);
        scagQueueLimit = cv.getInt("scagQueueLimit", NULL);
        connectionTimeout = cv.getInt("connectionTimeout", NULL);
        port = cv.getInt("port", NULL);
        portHttps = cv.getInt("portHttps", NULL);
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("HttpManagerConfig.init, Unknown exception.");
    }
}

bool HttpManagerConfig::check(const ConfigView& cv)   throw(ConfigException)
{   
    try {
        std::auto_ptr<char> h( cv.getString("host") );
        return readerSockets != cv.getInt("readerSockets", NULL) ||
            writerSockets != cv.getInt("writerSockets", NULL) ||
            readerPoolSize != cv.getInt("readerPoolSize", NULL) ||
            writerPoolSize != cv.getInt("writerPoolSize", NULL) ||
            scagPoolSize != cv.getInt("scagPoolSize", NULL) ||
            scagQueueLimit != cv.getInt("scagQueueLimit", NULL) ||
            connectionTimeout != cv.getInt("connectionTimeout", NULL) ||
            port != cv.getInt("port", NULL) ||
            portHttps != cv.getInt("portHttps", NULL) ||
            strcmp(host.c_str(), h.get());
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("HttpManagerConfig.check, Unknown exception.");
    }
}

}
}
