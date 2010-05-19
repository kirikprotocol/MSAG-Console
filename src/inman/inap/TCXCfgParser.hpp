/* ************************************************************************** *
 * TCAP Dispatcher & User XML configurations parsers.
 * ************************************************************************** */
#ifndef __INMAN_TCAP_CFGS_PARSERS_HPP__
#ident "@(#)$Id$"
#define __INMAN_TCAP_CFGS_PARSERS_HPP__

#include "inman/inap/TCDspDefs.hpp"
#include "util/config/XCFView.hpp"

#ifdef EIN_HD
#include "inman/inap/SS7HDCfgParser.hpp"
#endif /* EIN_HD */

namespace smsc  {
namespace inman {
namespace inap {

using smsc::util::config::Config;
using smsc::util::config::XConfigView;
using smsc::util::config::ConfigException;


#ifdef EIN_HD
typedef SS7HDCfgParser TCDspCfgParser;
#else /* EIN_HD */
class TCDspCfgParser {
private:
    Logger *        logger;
    const char *    nmSec;

public:
    TCDspCfgParser(Logger * use_log, const char * abs_nm_sec = "SS7")
        : logger(use_log), nmSec(abs_nm_sec)
    { }
    ~TCDspCfgParser()
    { }

    const char * nmCfgSection(void) const { return nmSec; }

    void readConfig(Config & root_sec, TCDsp_CFG & st_cfg) throw(ConfigException)
    {
        XConfigView cfgSec(root_sec, nmSec);

        uint32_t tmo = 0;    //ss7UserId
        try { tmo = (uint32_t)cfgSec.getInt("ss7UserId");
        } catch (const ConfigException & exc) { tmo = TCDsp_CFG::_MAX_USER_ID + 1; }
        if (!tmo || (tmo >= TCDsp_CFG::_MAX_USER_ID))
            throw ConfigException("'ss7UserId' is missing or invalid,"
                                  " allowed range [1..%u]", (unsigned)TCDsp_CFG::_MAX_USER_ID);
        st_cfg.mpUserId = (uint8_t)tmo;
        smsc_log_info(logger, "  ss7UserId: %u", st_cfg.mpUserId);

        tmo = 0;    //maxMsgNum, optional
        try { tmo = (uint32_t)cfgSec.getInt("maxMsgNum");
        } catch (const ConfigException & exc) { }
        if (tmo && (tmo >= TCDsp_CFG::_MAX_MSG_ENTRIES))
            throw ConfigException("'maxMsgNum' is invalid, allowed range [1..%u]",
                                  (unsigned)TCDsp_CFG::_MAX_MSG_ENTRIES);
        st_cfg.maxMsgNum = tmo ? (uint16_t)tmo : TCDsp_CFG::_DFLT_MSG_ENTRIES;
        smsc_log_info(logger, "  maxMsgNum: %u%s", st_cfg.maxMsgNum, !tmo ? " (default)":"");
        /**/
        return;
    }
};
#endif /* EIN_HD */

class TCAPUsrCfgParser {
private:
    Logger *        logger;
    const char *    nmSec;

public:
    TCAPUsrCfgParser(Logger * use_log, const char * abs_nm_sec = "TCAPUser")
        : logger(use_log), nmSec(abs_nm_sec)
    { }
    ~TCAPUsrCfgParser()
    { }

    const char * nmCfgSection(void) const { return nmSec; }

    void readConfig(Config & root_sec, TCAPUsr_CFG & st_cfg) throw(ConfigException)
    {
        XConfigView cfgSec(root_sec, nmSec);

        const char * cstr = NULL;
        try { cstr = cfgSec.getString("ownAddress");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("TCAPUser ISDN address is missing");
        if (!st_cfg.ownAddr.fromText(cstr) || !st_cfg.ownAddr.fixISDN())
            throw ConfigException("TCAPUser ISDN address is invalid: %s", cstr);

        uint32_t tmo = 0;
        try { tmo = (uint32_t)cfgSec.getInt("ownSsn");
        } catch (const ConfigException & exc) { }
        if (tmo && (tmo > 255))
            tmo = 0;
        else
            st_cfg.ownSsn = (uint8_t)tmo;
        if (!tmo)
            throw ConfigException("TCAPUser SSN number must belong to range [1..255]");

        tmo = 0;    //fakeSsn, optional
        try { tmo = (uint32_t)cfgSec.getInt("fakeSsn");
        } catch (const ConfigException & exc) { }
        if (tmo && (tmo > 255))
            throw ConfigException("TCAPUser fakeSSN number must belong to range [1..255]");
        st_cfg.fakeSsn = (uint8_t)tmo;

        smsc_log_info(logger, "  GT: %s", st_cfg.toString().c_str());

        tmo = 0;    //maxTimeout, optional
        try { tmo = (uint32_t)cfgSec.getInt("rosTimeout");
        } catch (const ConfigException & exc) { }
        if (tmo && (tmo > TCAPUsr_CFG::_MAX_ROS_TIMEOUT))
            throw ConfigException("'rosTimeout' must belong to range [1..%u] seconds",
                                  (unsigned)TCAPUsr_CFG::_MAX_ROS_TIMEOUT);
        st_cfg.rosTimeout = tmo ? (uint16_t)tmo : TCAPUsr_CFG::_DFLT_ROS_TIMEOUT;
        smsc_log_info(logger, "  rosTimeout: %u secs%s",
                      st_cfg.rosTimeout, !tmo ? " (default)":"");

        tmo = 0;    //maxDialogs, optional
        try { tmo = (uint32_t)cfgSec.getInt("maxDialogs");
        } catch (const ConfigException & exc) { }
        if ((tmo >= TCAPUsr_CFG::_MAX_TCAP_DIALOGS) || (tmo < 1))
            throw ConfigException("'maxDialogs' must belong to range [1..%u]",
                                  (unsigned)TCAPUsr_CFG::_MAX_TCAP_DIALOGS);
        st_cfg.maxDlgId = tmo ? (uint16_t)tmo : TCAPUsr_CFG::_DFLT_TCAP_DIALOGS;
        smsc_log_info(logger, "  maxDialogs: %u%s",
                      st_cfg.maxDlgId, !tmo ? " (default)":"");
        /**/
        return;
    }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_CFGS_PARSERS_HPP__ */

