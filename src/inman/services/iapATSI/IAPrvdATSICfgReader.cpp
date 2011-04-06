#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapATSI/IAPrvdATSICfgReader.hpp"
#include "inman/inap/xcfg/TCUsrCfgParser.hpp"
#include "util/strutil.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

using smsc::inman::inap::TCAPUsrCfgParser;
using smsc::util::str_cut_blanks;

/* ************************************************************************** *
 * class ICSIAPrvdATSICfgReader implementation:
 * ************************************************************************** */

//Parses XML configuration entry section, updates dependencies.
//Returns status of config parsing, 
ICSrvCfgReaderAC::CfgState
  ICSIAPrvdATSICfgReader::parseConfig(void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
  int iTmp = 0;
  try { iTmp = _topSec.getInt("maxThreads");
  } catch (const ConfigException & exc) { }
  if (iTmp > (uint16_t)(-1))
    throw ConfigException("parameter 'maxThreads' is out of range!");

  icsCfg->_maxThreads = (uint16_t)iTmp;
  if (iTmp) {
    smsc_log_info(logger, "  maxThreads: %u", (unsigned)iTmp);
  } else {
    smsc_log_info(logger, "  maxThreads: unlimited");
  }

  const char * cstr = NULL;
  try { cstr = _topSec.getString("tcapUser");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("parameter 'tcapUser' isn't set!");

  std::string nmSec(cstr);
  if ((str_cut_blanks(nmSec)).empty())
    throw ConfigException("parameter 'tcapUser' isn't set!");

  cstr = nmSec.c_str();
  if (!_xmfCfg.hasSection(cstr))
    throw ConfigException("section %s' is missing!", cstr);

  smsc_log_info(logger, "Reading settings from '%s' ..", cstr);
  XCFConfig * pTCfg = _xmfCfg.getSectionConfig(cstr);
  /**/
  TCAPUsrCfgParser  parser(logger, cstr);
  parser.readConfig(pTCfg->second, icsCfg.get()->_atsiCfg); //throws
  /**/
  return ICSrvCfgReaderAC::cfgComplete;
}

} //atih
} //iaprvd
} //inman
} //smsc

