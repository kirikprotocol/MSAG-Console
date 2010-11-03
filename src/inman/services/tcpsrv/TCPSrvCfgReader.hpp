/* ************************************************************************** *
 * Configurable Services Host config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_TCPSERVER_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCPSERVER_CONFIG_PARSING__

#include "inman/interaction/server.hpp"

#include "inman/services/ICSCfgReader.hpp"

namespace smsc {
namespace inman {
namespace tcpsrv {

using smsc::inman::ICSIdent;
using smsc::inman::ICSrvCfgReaderAC_T;

using smsc::inman::interaction::ServSocketCFG;

class ICSTcpSrvCfgReader : public ICSrvCfgReaderAC_T<ServSocketCFG> {
protected:
    static const unsigned int _DFLT_CLIENT_CONNS = 3;

    //Parses XML configuration entry section, updates dependencies.
    //Returns status of config parsing, 
    CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException)
    {
        XConfigView cfgSec(rootSec, nmCfgSection());
        const char * cstr = NULL;
        try {
            cstr = cfgSec.getString("host");
            icsCfg->port = cfgSec.getInt("port");
        } catch (const ConfigException & exc) {
            throw ConfigException("TCPSrv host or port missing");
        }
        icsCfg->host = cstr;

        uint32_t tmo = 0;
        try { tmo = (uint32_t)cfgSec.getInt("maxClients");
        } catch (const ConfigException & exc) { }
        icsCfg->maxConn = tmo ? tmo : _DFLT_CLIENT_CONNS;

        smsc_log_info(logger, "  TCPSrv: %s:%d, maxConn %u%s", icsCfg->host.c_str(),
                      icsCfg->port, icsCfg->maxConn, !tmo ? " (default)":"");
        /**/
        return ICSrvCfgReaderAC::cfgComplete;
    }

public:
    ICSTcpSrvCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSrvCfgReaderAC_T<ServSocketCFG>(root_sec, use_log, ics_sec ? ics_sec : "Host")
    { }
    ~ICSTcpSrvCfgReader()
    { }
};


} //tcpsrv
} //inman
} //smsc
#endif /* __INMAN_TCPSERVER_CONFIG_PARSING__ */

