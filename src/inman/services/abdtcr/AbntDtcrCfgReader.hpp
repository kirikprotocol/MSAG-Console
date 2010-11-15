/* ************************************************************************** *
 * Abonent contract detector service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_ABDTCR_CFG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABDTCR_CFG_PARSING__

#include "inman/incache/AbCacheDefs.hpp"
#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/abdtcr/AbntDtcrDefs.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::ICSIdent;
using smsc::inman::ICSrvCfgReaderAC_T;

using smsc::inman::cache::AbonentCacheCFG;


class ICSAbntDetectorCfgReader : public ICSrvCfgReaderAC_T<AbntDetectorXCFG> {
protected:
    static const uint16_t   _MAX_REQUESTS_NUM = 0xFFFF;
    static const uint16_t   _DFLT_REQUESTS_NUM = 1000;
    static const uint16_t   _MAX_ABTYPE_TIMEOUT = 0xFFFF;   //units: seconds
    static const uint16_t   _DFLT_ABTYPE_TIMEOUT = 8;       //units: seconds

    //Parses XML configuration entry section, updates dependencies.
    //Returns status of config parsing, 
    CfgState parseConfig(void *opaque_arg) throw(ConfigException);

public:
    ICSAbntDetectorCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSrvCfgReaderAC_T<AbntDetectorXCFG>(root_sec, use_log, ics_sec ? ics_sec : "AbonentDetector")
    {
        icsDeps.insert(ICSIdent::icsIdTCPServer);
        icsDeps.insert(ICSIdent::icsIdTimeWatcher);
    }
    ~ICSAbntDetectorCfgReader()
    { }

    AbntDetectorXCFG * rlseConfig(void)
    {
        icsDeps.exportDeps(icsCfg->icsDeps);
        return icsCfg.release();
    }
};

} //abdtcr
} //inman
} //smsc
#endif /* __INMAN_ABDTCR_CFG_PARSING__ */

