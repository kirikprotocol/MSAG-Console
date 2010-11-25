/* ************************************************************************** *
 * USSMan XML config file parser
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_CFG_READER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define __SMSC_USSMAN_CFG_READER_HPP__

#include "logger/Logger.h"
#include "util/config/XCFView.hpp"

#include "inman/inap/xcfg/TCDspCfgParser.hpp"
#include "inman/inap/xcfg/TCUsrCfgParser.hpp"

#include "inman/uss/UssServiceCfg.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

using smsc::logger::Logger;

using smsc::util::config::Config;
using smsc::util::config::XConfigView;
using smsc::util::config::ConfigException;

using smsc::inman::inap::TCDspCfgParser;
using smsc::inman::inap::TCAPUsrCfgParser;


class USSSrvCfgReader {
private:
    const char *    nmSrv; //name of USS Service
     //structure containing parsed configuration
    std::auto_ptr<UssService_CFG> stCfg;
    Logger *        logger;

public:
    static const uint32_t _DFLT_CLIENT_CONNS = 5;

    USSSrvCfgReader(const char * nm_uss_srv, Logger * use_log)
        : nmSrv(nm_uss_srv), logger(use_log)
    { }
    ~USSSrvCfgReader()
    { }

    void readConfig(Config & root_sec) throw(ConfigException)
    {
        stCfg.reset(new UssService_CFG());
        const char * cstr = NULL;

        /* read USS Service user parameters */
        if (!root_sec.findSection(nmSrv))
            throw ConfigException("section '%s' is missing!", nmSrv);
        {
            XConfigView usrCfg(root_sec, nmSrv);
            stCfg->sock.host = usrCfg.getString("host"); //throws
            stCfg->sock.port = usrCfg.getInt("port");   //throws
            smsc_log_info(logger, "%s : host=%s,port=%d", nmSrv,
                          stCfg->sock.host.c_str(), stCfg->sock.port);

            uint32_t tmo = 0;
            try { tmo = (uint32_t)usrCfg.getInt("maxConn");
            } catch (const ConfigException & exc) { }
            stCfg->sock.maxConn = tmo ? tmo : _DFLT_CLIENT_CONNS;
            smsc_log_info(logger, "%s: maxConn %u%s", nmSrv,
                          stCfg->sock.maxConn, !tmo ? " (default)":"");

            //read name of TCAPUser configuration section
            try { cstr = usrCfg.getString("tcapUser");
            } catch (const ConfigException & exc) { }
            if (!cstr || !cstr[0])
                throw ConfigException("parameter '%s' is invalid or missing!");
        }

        /* read SS7 interaction parameters */
        TCAPUsrCfgParser    usrParser(logger, cstr);
        if (!root_sec.findSection(usrParser.nmCfgSection()))
            throw ConfigException("section %s' is missing!", usrParser.nmCfgSection());
        smsc_log_info(logger, "Reading settings from '%s' ..", usrParser.nmCfgSection());
        usrParser.readConfig(root_sec, stCfg->tcUsr); //throws

        TCDspCfgParser  dspParser(logger);
        if (!root_sec.findSection(dspParser.nmCfgSection()))
            throw ConfigException("section %s' is missing!", dspParser.nmCfgSection());
        smsc_log_info(logger, "Reading settings from '%s' ..", dspParser.nmCfgSection());
        dspParser.readConfig(root_sec, stCfg->ss7); //throws
    }

    UssService_CFG * rlseConfig(void) { return stCfg.release(); }
    UssService_CFG * getConfig(void) const { return stCfg.get(); }
};

} //uss
} //inman
} //smsc

#endif /* __SMSC_USSMAN_CFG_READER_HPP__ */

