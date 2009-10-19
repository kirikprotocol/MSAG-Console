/* ************************************************************************** *
 * IAPManager (abonent policies/providers) service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPMGR_CFG_PARSING__
#ident "@(#)$Id$"
#define __INMAN_IAPMGR_CFG_PARSING__

#include "inman/common/CSVList.hpp"
using smsc::util::CSVList;

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
    struct IAPolicyParser {
        std::string prvdNm;
        CStrSet     scfNms;

        bool Parse(const char * str)
        {
            CSVList vlist(str);
            if (vlist.empty())
                return false;

            prvdNm = vlist[0]; //provider section name may be empty!
            //prepare SCF sections name set
            for (CSVList::size_type n = 1; n < vlist.size(); ++n) {
                if (!vlist[n].empty())
                    scfNms.insert(vlist[n]);
            }
            return !(prvdNm.empty() && scfNms.empty());
        }

        const char * nmPrvd(void) const
        {
             return prvdNm.empty() ? NULL : prvdNm.c_str();
        }

        void print(std::string & pstr) const
        {
            pstr += "Prvd{";
            pstr += prvdNm.empty() ? "<none>" : prvdNm.c_str();
            pstr += "}, INs{";
            if (!scfNms.empty()) {
                CStrSet::const_iterator it = scfNms.begin();
                pstr += *it;
                for (++it; it != scfNms.end(); ++it) {
                    pstr += ",  "; pstr += *it;
                }
            } else
                pstr += "<none>";
            pstr += "}";
        }

        std::string print(void) const
        {
            std::string pstr;
            print(pstr);
            return pstr;
        }
    };

    SCFsCfgReader   scfReader;          //Parses IN-platforms config
    ICSProvidersCfgReader prvdReader;   //Parses IAProviders config

protected:
    // -- ----------------------------------------------
    // -- ICSMultiSectionCfgReaderAC_T interface methods
    // -- ----------------------------------------------
    CfgState parseSection(XConfigView * cfg_sec, const std::string & nm_sec,
                          void * opaque_arg = NULL)
        throw(ConfigException)
    {
        const char * nm_cfg = nm_sec.c_str();
        CfgParsingResult state(sectionState(nm_sec));
        if (state.cfgState == ICSrvCfgReaderAC::cfgComplete) {
            smsc_log_info(logger, "Already read '%s' configuration ..", nm_cfg);
            return ICSrvCfgReaderAC::cfgComplete;
        }

        smsc_log_info(logger, "Reading %s policy config ..", nm_cfg);
        const char * cstr = NULL;
        cstr = cfg_sec->getString("policy"); //throws

        IAPolicyParser   polXCFG;
        if (!polXCFG.Parse(cstr))
            throw ConfigException("'%s' policy value is invalid: %s", nm_cfg, cstr ? cstr : "");
        smsc_log_info(logger, "  policy is: %s", polXCFG.print().c_str());

        std::auto_ptr<AbonentPolicy> polDat(new AbonentPolicy(nm_cfg, polXCFG.nmPrvd()));
        
        if (!polXCFG.scfNms.empty()) {  //lookup IN platforms configs
            scfReader.addArguments(polXCFG.scfNms);
            scfReader.readConfig(&(polDat->scfMap)); //throws
            icsDeps.insert(ICSIdent::icsIdTCAPDisp);
        }
        if (polXCFG.nmPrvd()) {      //add Abonent Provider in dependencies
            prvdReader.addArgument(polDat->prvdNm);
            if (prvdReader.readConfig()) { //throws
                ICSIdsSet ids;
                prvdReader.Deps().exportDeps(ids);
                icsDeps.insert(ids);
            }
        }
        icsCfg->polReg.insert(polDat->ident, polDat.release());
        //mark section as completely parsed
        state.cfgState = ICSrvCfgReaderAC::cfgComplete;
        return registerSection(nm_sec, state);
    }

public:
    ICSIAPMgrCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSMultiSectionCfgReaderAC_T<IAPManagerCFG>(root_sec, use_log,
                                        ics_sec ? ics_sec : "AbonentPolicies")
        , scfReader(root_sec, use_log), prvdReader(root_sec, use_log)
    { }
    ~ICSIAPMgrCfgReader()
    { }

    IAPManagerCFG * rlseConfig(void)
    {
        icsCfg->scfReg.reset(scfReader.rlseConfig());
        std::auto_ptr<IAPrvdConfig> prvdCfg(prvdReader.rlseConfig());
        icsCfg->prvdReg.reset(prvdCfg->registry.release());
        //combine dependencies
        icsDeps.exportDeps(icsCfg->deps);
        icsCfg->deps.insert(prvdCfg->deps.begin(), prvdCfg->deps.end());
        return icsCfg.release();
    }
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_IAPMGR_CFG_PARSING__ */

