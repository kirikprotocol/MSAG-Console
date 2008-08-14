#ifndef SCAG_CONFIG_CONFIG_VIEW
#define SCAG_CONFIG_CONFIG_VIEW

#include <string.h>
#include <inttypes.h>

#include <util/debug.h>
#include <logger/Logger.h>

#include "Config.h"

namespace scag {
namespace config {

    using smsc::logger::Logger;
    //using scag::config::ConfigException;

    class ConfigView
    {
    protected:
        
        smsc::logger::Logger *log;

        const Config&    config;
        char*       category;
    
        char* prepareSubSection(const char* sub) const;

    public:

        ConfigView(const Config& config_, const char* cat=0);
        virtual ~ConfigView();
        
        inline const char* getInstance() {
            return category;
        };
        
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
        
        char* getString(const char* param, const char* error=0, bool check=true) const
            throw (ConfigException);
        int32_t getInt(const char* param, const char* error=0) const
            throw (ConfigException);
        bool getBool(const char* param, const char* error=0) const
            throw (ConfigException);
    };

}}

#endif
