#include "BillingManagerConfig.h"

namespace scag {
namespace config {

BillingManagerConfig::BillingManagerConfig(ConfigView& cv) throw(ConfigException)
{
}

void BillingManagerConfig::init(ConfigView& cv) throw(ConfigException)
{
    try {
        std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
        std::auto_ptr<char> host_(cv.getString("BillingServerHost"));
        int port_(cv.getInt("BillingServerPort"));
        

        BillingTimeOut = cv.getInt("BillingTimeOut");
        BillingReconnectTimeOut = cv.getInt("BillingReconnectTimeOut");
        //MaxThreads = cv.getInt("MaxThreads");
        MaxThreads = 10;


        cfg_dir = cfg_dir_.get();
        BillingHost = host_.get();
        BillingPort = port_;


    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("BillingManagerConfig.init, Unknown exception.");
    }

}

bool BillingManagerConfig::check(ConfigView& cv) throw(ConfigException)
{   
    try {
        //std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
        //if(   !strcmp( cfg_dir.c_str(), cfg_dir_.get() )   )
        //    return false;

        std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
        if(   !strcmp( cfg_dir.c_str(), cfg_dir_.get() )   )
            return false;
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("BillingManagerConfig.check, Unknown exception.");
    }

    return true;
}

}
}