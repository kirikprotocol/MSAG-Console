#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/tcpsrv/TCPSrvCfgReader.hpp"

using smsc::inman::ICSrvCfgReaderAC;

namespace smsc {
namespace inman {
namespace tcpsrv {

//Parses XML configuration entry section, updates dependencies.
//Returns status of config parsing, 
ICSrvCfgReaderAC::CfgState
  ICSTcpSrvCfgReader::parseConfig(void * opaque_arg/* = NULL*/) throw(ConfigException)
{
  const char * cstr = NULL;
  try {
    cstr = _topSec.getString("host");
    icsCfg->port = _topSec.getInt("port");
  } catch (const ConfigException & exc) {
    throw ConfigException("TCPSrv host or port missing");
  }
  icsCfg->host = cstr;

  uint32_t tmo = 0;
  try { tmo = (uint32_t)_topSec.getInt("maxClients");
  } catch (const ConfigException & exc) { }
  icsCfg->maxConn = tmo ? tmo : _DFLT_CLIENT_CONNS;

  smsc_log_info(logger, "  TCPSrv: %s:%d, maxConn %u%s", icsCfg->host.c_str(),
                icsCfg->port, icsCfg->maxConn, !tmo ? " (default)":"");
  /**/
  return ICSrvCfgReaderAC::cfgComplete;
}

} //tcpsrv
} //inman
} //smsc

