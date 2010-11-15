/* ************************************************************************** *
 * Abonents Cache service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_ABCACHE_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABCACHE_CONFIG_PARSING__

#include "inman/incache/AbCacheDefs.hpp"

#include "inman/services/ICSCfgReader.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::inman::ICSIdent;
using smsc::inman::ICSrvCfgReaderAC_T;

class ICSAbCacheCfgReader : public ICSrvCfgReaderAC_T<AbonentCacheCFG> {
protected:
    //Parses XML configuration entry section, updates dependencies.
    //Returns status of config parsing, 
    CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException)
    {
        XConfigView cfgSec(rootSec, nmCfgSection());

        uint32_t tmo = 0;
        //cache parameters
        try { tmo = (uint32_t)cfgSec.getInt("cacheExpiration");
        } catch (const ConfigException & exc) { }
        if (!tmo || (tmo > AbonentCacheCFG::_MAX_CACHE_INTERVAL))
            icsCfg->interval = AbonentCacheCFG::_DFLT_CACHE_INTERVAL;
        else
            icsCfg->interval = tmo;
        smsc_log_info(logger, "  cacheExpiration: %u minutes%s", icsCfg->interval,
                      !tmo ? " (default)":"");
        //convert minutes to seconds
        icsCfg->interval *= 60;

        try { icsCfg->RAM = (uint32_t)cfgSec.getInt("cacheRAM");
        } catch (const ConfigException & exc) { }
        if (!icsCfg->RAM)
            throw ConfigException("'cacheRAM' is missing or invalid");
        smsc_log_info(logger, "  cacheRAM: %u Mb", icsCfg->RAM);

        const char * cstr = NULL;
        try { cstr = cfgSec.getString("cacheDir");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("'cacheDir' is missing or invalid");
        icsCfg->nmDir += cstr;
        smsc_log_info(logger, "  cacheDir: %s", cstr);

        tmo = 0;
        try { tmo = (uint32_t)cfgSec.getInt("cacheRecords");
        } catch (const ConfigException & exc) { }
        if (tmo)
            icsCfg->fileRcrd = (int)tmo;
        smsc_log_info(logger, "  cacheRecords: %d%s", icsCfg->fileRcrd,
                      !tmo ? " (default)":"");
        /**/
        return ICSrvCfgReaderAC::cfgComplete;
    }

public:
    ICSAbCacheCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSrvCfgReaderAC_T<AbonentCacheCFG>(root_sec, use_log, ics_sec ? ics_sec : "AbonentsCache")
    { }
    ~ICSAbCacheCfgReader()
    { }
};

} //cache
} //inman
} //smsc
#endif /* __INMAN_ABCACHE_CONFIG_PARSING__ */

