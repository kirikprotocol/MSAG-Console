#ifndef SMSC_UTIL_CONFIG_CONFIG_VIEW
#define SMSC_UTIL_CONFIG_CONFIG_VIEW

#include <string.h>
#include <inttypes.h>

#include <util/debug.h>
#include <logger/Logger.h>

#include "Manager.h"

namespace smsc { namespace util { namespace config
{
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    class ConfigView
    {
    protected:
        
        static log4cpp::Category    &log;

        Manager&    config;
        char*       category;
    
        char* prepareSubSection(const char* sub);

    public:

        ConfigView(Manager& manager, const char* cat=0);
        virtual ~ConfigView();
        
        inline const char* getCategory() {
            return category;
        };
        
        ConfigView* getSubConfig(const char* sub, bool full=false);
        
        CStrSet* getSectionNames()
            throw (ConfigException);
        CStrSet* getIntParamNames()
            throw (ConfigException);
        CStrSet* getBoolParamNames()
            throw (ConfigException);
        CStrSet* getStrParamNames()
            throw (ConfigException);
        
        char* getString(const char* param, const char* error=0, bool check=true)
            throw (ConfigException);
        int32_t getInt(const char* param, const char* error=0)
            throw (ConfigException);
        bool getBool(const char* param, const char* error=0)
            throw (ConfigException);
    };

}}}

#endif
