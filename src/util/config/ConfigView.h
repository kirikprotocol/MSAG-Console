#ifndef __GNUC__
#pragma ident "$Id$"
#endif

#ifndef SMSC_UTIL_CONFIG_CONFIG_VIEW
#define SMSC_UTIL_CONFIG_CONFIG_VIEW

#include <string.h>
#include <inttypes.h>

#include <util/debug.h>
#include <logger/Logger.h>

#include "Manager.h"

namespace smsc {
namespace util {
namespace config {

    using smsc::logger::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::Config;
    using smsc::util::config::ConfigException;

    class ConfigView
    {
    protected:
        
        smsc::logger::Logger *log;

        const Config& config;
        char*       category;
    
        char* prepareSubSection(const char* sub) const;

    public:

        ConfigView(Manager & manager, const char* cat = 0);
        ConfigView(const Config & use_config, const char* cat = 0);
        virtual ~ConfigView();
        
        inline const char* getInstance() {
            return category;
        };

        bool    findSubSection(const char * const subsectionName) const;
        ConfigView* getSubConfig(const char* sub, bool full=false) const;
        
        CStrSet* getSectionNames() const
            throw (ConfigException);
        CStrSet* getShortSectionNames() const
            throw (ConfigException);

        CStrSet* getIntParamNames() const
            throw (ConfigException);
        CStrSet* getBoolParamNames() const
            throw (ConfigException);
        CStrSet* getStrParamNames() const
            throw (ConfigException);
        
        /// NOTE: returned value should be deleted []
        char* getString(const char* param, const char* error=0, bool check=true) const
            throw (ConfigException);
        int32_t getInt(const char* param, const char* error=0) const
            throw (ConfigException);
        bool getBool(const char* param, const char* error=0) const
            throw (ConfigException);
    };

}}}

#endif
