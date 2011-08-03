#include "HttpManagerConfig.h"

namespace scag { namespace config {

HttpManagerConfig::HttpManagerConfig(const ConfigView& cv) throw(ConfigException)
{
    init(cv);
}

void HttpManagerConfig::init(const ConfigView& cv)   throw(ConfigException)
{
    try {
        std::auto_ptr <char> h(cv.getString("host", "in HttpManagerConfig.init", true));
        host = h.get();

        readerSockets = cv.getInt("readerSockets", NULL);
        writerSockets = cv.getInt("writerSockets", NULL);
        readerPoolSize = cv.getInt("readerPoolSize", NULL);
        writerPoolSize = cv.getInt("writerPoolSize", NULL);
        scagPoolSize = cv.getInt("scagPoolSize", NULL);
        scagQueueLimit = cv.getInt("scagQueueLimit", NULL);
        connectionTimeout = cv.getInt("connectionTimeout", NULL);
        port = cv.getInt("port", NULL);
        httpsEnabled = cv.getBool("https.enabled", NULL);
        httpsPort = cv.getInt("https.port", NULL);
        std::auto_ptr<char> c(cv.getString("https.certificatesLocation", "in HttpManagerConfig.init", true));
        httpsCertificates = c.get();
        httpsTimeout = cv.getInt("https.timeout", NULL);
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("HttpManagerConfig.init, Unknown exception.");
    }
}

bool HttpManagerConfig::check(const ConfigView& cv)   throw(ConfigException)
{   
    try {
        std::auto_ptr<char> h(cv.getString("host", "in HttpManagerConfig.check", true));
        std::auto_ptr<char> c(cv.getString("https.certificatesLocation", "in HttpManagerConfig.check", true));
        return readerSockets != cv.getInt("readerSockets", NULL) ||
            writerSockets != cv.getInt("writerSockets", NULL) ||
            readerPoolSize != cv.getInt("readerPoolSize", NULL) ||
            writerPoolSize != cv.getInt("writerPoolSize", NULL) ||
            scagPoolSize != cv.getInt("scagPoolSize", NULL) ||
            scagQueueLimit != cv.getInt("scagQueueLimit", NULL) ||
            connectionTimeout != cv.getInt("connectionTimeout", NULL) ||
            port != cv.getInt("port", NULL) ||
            strcmp(host.c_str(), h.get()) ||
            httpsEnabled != cv.getBool("https.enabled", NULL) ||
            httpsPort != cv.getInt("https.port", NULL) ||
            strcmp(httpsCertificates.c_str(), c.get()) ||
            httpsTimeout != cv.getInt("https.timeout", NULL);
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("HttpManagerConfig.check, Unknown exception.");
    }
}

}
}
