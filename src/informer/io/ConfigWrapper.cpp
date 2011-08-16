#include "ConfigWrapper.h"
#include "logger/Logger.h"
#include "util/config/Config.h"
#include "InfosmeException.h"

namespace eyeline {
namespace informer {

using namespace smsc::util::config;

bool ConfigWrapper::getBool( const char* name, bool def, bool usedefault ) const
{
    bool val;
    try {
        val = cfg_.getBool(name);
    } catch ( HashInvalidKeyException& e ) {
        if (!usedefault) {
            throw InfosmeException(EXC_CONFIG,"the parameter '%s' is not found",name);
        }
        val = def;
    }
    return val;
}


int ConfigWrapper::getInt( const char* name, int def, int min, int max, bool usedefault ) const
{
    int val;
    try {
        val = cfg_.getInt(name);
    } catch ( HashInvalidKeyException& e ) {
        if (!usedefault) {
            throw InfosmeException(EXC_CONFIG,"the parameter '%s' is not found",name);
        }
        if (log_) {
            smsc_log_debug(log_,"the parameter '%s' is not found, using %d", name, def);
        }
        val = def;
    }
    if (max>=min) {
        if (val<min) {
            if (log_) {
                smsc_log_info(log_,"the parameter '%s' (%d) is less than %d, using %d",
                              name, val, min, min);
            }
            val = min;
        } else if (val>max) {
            if (log_) {
                smsc_log_info(log_,"the parameter '%s' (%d) is greater than %d, using %d",
                              name, val, max, max);
            }
            val = max;
        }
    }
    return val;
}


std::string ConfigWrapper::getString( const char* name, const char* def, bool strip ) const
{
    if (!name) {
        throw InfosmeException(EXC_LOGICERROR,"NULL passed in getString()");
    }
    const char* val;
    try {
        val = cfg_.getString(name);
    } catch (HashInvalidKeyException&) {
        if (!def) {
            throw InfosmeException(EXC_CONFIG,"mandatory parameter '%s' is not found",name);
        }
        if (log_) {
            smsc_log_debug(log_,"the parameter '%s' is not found, using '%s'",
                           name, def);
        }
        val = def;
    }
    if (strip) {
        while (*val == ' ' || *val == '\t' || *val == '\n') ++val;
    }
    size_t len = strlen(val);
    if (strip) {
        for ( const char* p = val+len; len>0; ) {
            const char v = *--p;
            if ( v == ' ' || v == '\t' || v == '\n' ) {
                --len;
                continue;
            }
            break;
        }
    }
    return std::string(val,len);
}

} // informer
} // smsc
