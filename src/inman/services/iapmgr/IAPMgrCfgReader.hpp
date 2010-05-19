/* ************************************************************************** *
 * IAPManager (abonent policies/providers) service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPMGR_CFG_PARSING__
#ident "@(#)$Id$"
#define __INMAN_IAPMGR_CFG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
using smsc::inman::ICSIdent;
using smsc::inman::ICSMultiSectionCfgReaderAC_T;

#include "inman/services/iapmgr/IAPMgrDefs.hpp"
#include "inman/services/iapmgr/IAPrvdsCfgReader.hpp"
#include "inman/services/iapmgr/SCFsCfgReader.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

class ICSIAPMgrCfgReader : public ICSMultiSectionCfgReaderAC_T<IAPManagerCFG> {
private:
    SCFsCfgReader         scfReader;    //Parses IN-platforms config
    ICSProvidersCfgReader prvdReader;   //Parses IAProviders config

protected:
    // -- ----------------------------------------------
    // -- ICSMultiSectionCfgReaderAC_T interface methods
    // -- ----------------------------------------------
    CfgState parseSection(XConfigView * cfg_sec, const std::string & nm_sec,
                          void * opaque_arg = NULL)
        throw(ConfigException);

public:
    ICSIAPMgrCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSMultiSectionCfgReaderAC_T<IAPManagerCFG>(root_sec, use_log,
                                        ics_sec ? ics_sec : "AbonentPolicies")
        , scfReader(root_sec, use_log), prvdReader(root_sec, use_log)
    { }
    ~ICSIAPMgrCfgReader()
    { }

    IAPManagerCFG * rlseConfig(void);
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_IAPMGR_CFG_PARSING__ */

