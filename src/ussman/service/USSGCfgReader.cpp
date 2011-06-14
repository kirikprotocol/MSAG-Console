#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/config/XCFView.hpp"
using smsc::util::config::XConfigView;

#include "inman/inap/xcfg/TCDspCfgParser.hpp"
using smsc::inman::inap::TCDspCfgParser;

#include "inman/inap/xcfg/TCUsrCfgParser.hpp"
using smsc::inman::inap::TCAPUsrCfgParser;

#include "ussman/service/USSGCfgReader.hpp"

namespace smsc {
namespace ussman {
/* ************************************************************************** *
 * class USSGCfgReader implementation:
 * ************************************************************************** */
void USSGCfgReader::readConfig(Config & root_sec) throw(ConfigException)
{
  stCfg.reset(new USSGateway_XCFG());
  const char * cstr = NULL;

  /* read USS Service user parameters */
  if (!root_sec.findSection(nmSrv))
    throw ConfigException("section '%s' is missing!", nmSrv);
  smsc_log_info(logger, "Reading settings from '%s' ..", nmSrv);
  {
    XConfigView usrCfg(root_sec, nmSrv);
    stCfg->_tcp._host = usrCfg.getString("host"); //throws
    stCfg->_tcp._port = usrCfg.getInt("port");   //throws
    smsc_log_info(logger, "  host '%s', port %u", stCfg->_tcp._host.c_str(),
                  stCfg->_tcp._port);

    uint32_t tmo = 0;
    try { tmo = (uint32_t)usrCfg.getInt("maxConn");
    } catch (const ConfigException & exc) { }
    stCfg->_tcp._maxConn = tmo ? tmo : _DFLT_CLIENT_CONNS;
    smsc_log_info(logger, "  maxConn: %u%s", stCfg->_tcp._maxConn,
                  !tmo ? " (default)":"");

    tmo = 0;
    try { tmo = (uint32_t)usrCfg.getInt("maxRequests");
    } catch (const ConfigException & exc) { tmo = _MAX_REQUESTS_NUM + 1; }

    if (tmo > _MAX_REQUESTS_NUM)
      throw ConfigException("'maxRequests' is invalid or missing,"
                              " allowed range [1..%u)", _MAX_REQUESTS_NUM);
    stCfg->_maxRequests = tmo ? tmo : _DFLT_REQUESTS_NUM;
    smsc_log_info(logger, "  maxRequests: %u per connect%s", stCfg->_maxRequests,
                  !tmo ? " (default)":"");

    tmo = 0;
    try { tmo = (uint32_t)usrCfg.getInt("maxThreads");
    } catch (const ConfigException & exc) { tmo = _DFLT_THREADS_NUM; }
    if (tmo > _MAX_THREADS_NUM)
      throw ConfigException("maxThreads is out of range [0 ..65535]");

    stCfg->_maxThreads = (uint16_t)tmo;
    if (!tmo) {
      smsc_log_info(logger, "  maxThreads: unlimited per connect");
    } else {
      smsc_log_info(logger, "  maxThreads: %u per connect%s", (unsigned)stCfg->_maxThreads,
                    (tmo == _DFLT_THREADS_NUM) ? " (default)":"");
    }

    //read name of TCAPUser configuration section
    try { cstr = usrCfg.getString("tcapUser");
    } catch (const ConfigException & exc) { }
    if (!cstr || !cstr[0])
      throw ConfigException("parameter '%s' is invalid or missing!");

    bool bTmp = false;
    try { bTmp = usrCfg.getBool("denyDupRequest");
    } catch (const ConfigException & exc) { }
    stCfg->_denyDupRequest = bTmp;
    smsc_log_info(logger, "  denyDupRequest: %s", stCfg->_denyDupRequest ? "true" : "false");
  }

  /* read SS7 interaction parameters */
  TCAPUsrCfgParser    usrParser(logger, cstr);
  if (!root_sec.findSection(usrParser.nmCfgSection()))
    throw ConfigException("section %s' is missing!", usrParser.nmCfgSection());
  smsc_log_info(logger, "Reading settings from '%s' ..", usrParser.nmCfgSection());
  usrParser.readConfig(root_sec, stCfg->_tcUsr); //throws

  TCDspCfgParser  dspParser(logger);
  if (!root_sec.findSection(dspParser.nmCfgSection()))
    throw ConfigException("section %s' is missing!", dspParser.nmCfgSection());
  smsc_log_info(logger, "Reading settings from '%s' ..", dspParser.nmCfgSection());
  dspParser.readConfig(root_sec, stCfg->_ss7); //throws
}




} //ussman
} //smsc

