#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapSRI/IAPrvdSRICfgReader.hpp"
#include "inman/inap/xcfg/TCUsrCfgParser.hpp"
#include "util/strutil.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::inap::TCAPUsrCfgParser;
using smsc::util::str_cut_blanks;

/* ************************************************************************** *
 * class ICSIAPrvdSRICfgReader implementation:
 * ************************************************************************** */

//Parses XML configuration entry section, updates dependencies.
//Returns status of config parsing, 
ICSrvCfgReaderAC::CfgState
  ICSIAPrvdSRICfgReader::parseConfig(void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
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
  parser.readConfig(pTCfg->second, icsCfg.get()->sriCfg); //throws
  /**/
  return ICSrvCfgReaderAC::cfgComplete;
}

} //sri
} //iaprvd
} //inman
} //smsc

