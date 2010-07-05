/* ************************************************************************** *
 * EIN SS7 HD stack interaction parameters XML configurations parser.
 * ************************************************************************** */
#ifndef __SS7HD_CFG_PARSER_HPP__
#ident "@(#)$Id$"
#define __SS7HD_CFG_PARSER_HPP__

#include "inman/inap/SS7HDCfgDefs.hpp"
#include "util/config/XCFView.hpp"
#include "util/csv/CSVListOf.hpp"

namespace smsc  {
namespace inman {
namespace inap {

using smsc::util::config::Config;
using smsc::util::config::XConfigView;
using smsc::util::config::ConfigException;
using smsc::util::csv::CSVListOfStr;

class SS7HDCfgParser {
private:
    Logger *        logger;
    const char *    nmSec;

protected:

    void readUnits(XConfigView & xcfg_sec, SS7HD_CFG & st_cfg, const char * nm_subs)
        throw(ConfigException)
    {
        if (!xcfg_sec.findSubSection(nm_subs))
            throw ConfigException("SS7Units subsection is missing: %s", nm_subs);

        smsc_log_info(logger, "Reading \'%s\' settings", nm_subs);
        std::auto_ptr<XConfigView> cfgSec(xcfg_sec.getSubConfig(nm_subs));

        const char * cstr = NULL; //unitId
        try { cstr = cfgSec->getString("unitId");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("SS7Unit id is missing");

        uint16_t unitId = SS7HD_CFG::unitIdByName(cstr);
        if (!unitId)
            throw ConfigException("SS7Unit id is invalid or unsupported: %s", cstr);
        smsc_log_info(logger, "  unitId: %s", cstr);


        cstr = NULL; //instancesList
        try { cstr = cfgSec->getString("instancesList");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("SS7Unit instances list is missing");

        CSVListOfStr nmIds;
        if (!nmIds.fromStr(cstr))
            throw ConfigException("SS7Unit instances list is invalid: %s", cstr);

        SS7Unit_CFG & unitCfg = st_cfg.ss7Units[unitId];
        unitCfg.unitId = unitId; //if it's just created

        for (CSVListOfStr::const_iterator cit = nmIds.begin(); cit != nmIds.end(); ++cit) {
            int instId = atoi(cit->c_str());
            if (!instId || (instId > SS7HD_CFG::_MAX_UNIT_INSTANCE_ID))
                throw ConfigException("SS7Unit instanceId is invalid: %s", cit->c_str());
            if (unitCfg.instIds.find((uint8_t)instId) != unitCfg.instIds.end())
                throw ConfigException("SS7Unit instanceId is duplicated: %s", cit->c_str());
            unitCfg.instIds.insert(SS7UnitInstsMap::value_type((uint8_t)instId,
                                                    SS7UnitInstance((uint8_t)instId)));
        }
        smsc_log_info(logger, "  instancesList: %s", nmIds.toString().c_str());
        return;
    }

    void readLayout(XConfigView & root_sec, SS7HD_CFG & st_cfg)
        throw(ConfigException)
    {
        if (!root_sec.findSubSection("Layouts"))
            throw ConfigException("\'Layouts\' subsection is missing");
        std::auto_ptr<XConfigView> loSec(root_sec.getSubConfig("Layouts"));

        const char * cstr = st_cfg.nmLayout.c_str();
        if (!loSec->findSubSection(cstr))
            throw ConfigException("SS7 units layout subsection is missing: \'%s\'", cstr);

        smsc_log_info(logger, "Reading \'%s\'  layout", cstr);
        std::auto_ptr<XConfigView> cfgSec(loSec->getSubConfig(cstr));

        uint32_t tmo = 0;    //instanceId
        try { tmo = (uint32_t)cfgSec->getInt("instanceId");
        } catch (const ConfigException & exc) { tmo = SS7HD_CFG::_MAX_APPINSTANCE_ID + 1; }
        if (!tmo || (tmo > SS7HD_CFG::_MAX_APPINSTANCE_ID))
            throw ConfigException("application instanceId"
                                  " is missing or invalid, allowed range [1..%u]",
                                  SS7HD_CFG::_MAX_APPINSTANCE_ID);
        st_cfg.appInstId = (uint8_t)tmo;
        smsc_log_info(logger, "  instanceId: %u", st_cfg.appInstId);

        cstr = NULL; //ss7Units
        try { cstr = cfgSec->getString("ss7Units");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("SS7 Unit(s) subsection list is missing");

        CSVListOfStr nmUnits;
        if (!nmUnits.fromStr(cstr))
            throw ConfigException("SS7 Unit(s) subsection list is invalid: %s", cstr);
        smsc_log_info(logger, "  SS7Units: %s", nmUnits.toString().c_str());


        if (!root_sec.findSubSection("SS7Units"))
            throw ConfigException("\'SS7Units\' subsection is missing");
        std::auto_ptr<XConfigView> unitSec(root_sec.getSubConfig("SS7Units"));

        for (CSVListOfStr::const_iterator cit = nmUnits.begin();
                                cit != nmUnits.end(); ++cit) {
            readUnits(*unitSec.get(), st_cfg, cit->c_str());
        }
        return;
    }


public:
    SS7HDCfgParser(Logger * use_log, const char * abs_nm_sec = "SS7_HD")
        : logger(use_log), nmSec(abs_nm_sec)
    { }
    ~SS7HDCfgParser()
    { }

    const char * nmCfgSection(void) const { return nmSec; }

    void readConfig(Config & root_sec, SS7HD_CFG & st_cfg) throw(ConfigException)
    {
        XConfigView cfgSec(root_sec, nmSec);

        uint32_t      tmo = 0;
        const char *  cstr = NULL; //cpMgrHost

        try { cstr = cfgSec.getString("cpMgrHosts");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("Remote CommonParts Manager"
                                  " host addresses are missing");

        { //validate addresses format and recompose it without blanks
          CSVListOfStr  hosts;
          if (!hosts.fromStr(cstr))
            throw ConfigException("Remote CommonParts Manager"
                                  " host addresses are invalid: %s", cstr);
          unsigned i = 0;
          for (CSVListOfStr::iterator it = hosts.begin(); it != hosts.end(); ++it, ++i) {
            CSVListOfStr hp(':');
            if (hp.fromStr(it->c_str()) != 2)
              throw ConfigException("Remote CommonParts Manager"
                                    " host address is invalid: %s", it->c_str());
            if (i)
              st_cfg.rcpMgrAdr += ',';
            hp.toString(st_cfg.rcpMgrAdr, 0);
          }
        }
        smsc_log_info(logger, "  cpMgrHosts: \'%s\'", st_cfg.rcpMgrAdr.c_str());

        tmo = 0;    //cpMgrInstId
        try { tmo = (uint32_t)cfgSec.getInt("cpMgrInstId");
        } catch (const ConfigException & exc) { tmo = SS7HD_CFG::_MAX_CPMGR_INSTANCE_ID + 1; }
        if (tmo > SS7HD_CFG::_MAX_CPMGR_INSTANCE_ID)
            throw ConfigException("Remote CommonParts Manager instanceId"
                                  " is missing or invalid, allowed range [0..%u]",
                                  SS7HD_CFG::_MAX_CPMGR_INSTANCE_ID);
        st_cfg.rcpMgrInstId = (uint8_t)tmo;
        smsc_log_info(logger, "  cpMgrInstId: %u", st_cfg.rcpMgrInstId);


        tmo = 0;    //mpUserId
        try { tmo = (uint32_t)cfgSec.getInt("mpUserId");
        } catch (const ConfigException & exc) { tmo = SS7HD_CFG::_MAX_USER_ID + 1; }
        if (!tmo || (tmo >= SS7HD_CFG::_MAX_USER_ID))
            throw ConfigException("'mpUserId' is missing or invalid,"
                                  " allowed range [1..%u]", (unsigned)SS7HD_CFG::_MAX_USER_ID);
        st_cfg.mpUserId = (uint8_t)tmo;
        smsc_log_info(logger, "  mpUserId: %u", st_cfg.mpUserId);

        tmo = 0;    //maxMsgNum, optional
        try { tmo = (uint32_t)cfgSec.getInt("maxMsgNum");
        } catch (const ConfigException & exc) { }
        if (tmo && (tmo >= TCDsp_CFG::_MAX_MSG_ENTRIES))
            throw ConfigException("'maxMsgNum' is invalid, allowed range [1..%u]",
                                  (unsigned)SS7HD_CFG::_MAX_MSG_ENTRIES);
        st_cfg.maxMsgNum = tmo ? (uint16_t)tmo : SS7HD_CFG::_DFLT_MSG_ENTRIES;
        smsc_log_info(logger, "  maxMsgNum: %u%s", st_cfg.maxMsgNum, !tmo ? " (default)":"");


        cstr = NULL; //dfltLayout
        try { cstr = cfgSec.getString("dfltLayout");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("SS7 units layout name is not specified");
        st_cfg.nmLayout = cstr;
        smsc_log_info(logger, "  dfltLayout: \'%s\'", cstr);
        /**/
        readLayout(cfgSec, st_cfg);
        return;
    }
};


} //inap
} //inman
} //smsc
#endif /* __SS7HD_CFG_PARSER_HPP__ */


