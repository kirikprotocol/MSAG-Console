#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/iapSRI/IAPrvdSRICfgReader.hpp"
#include "inman/inap/TCXCfgParser.hpp"
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
  XConfigView  iapCFG(rootSec, nmCfgSection());

  const char * cstr = NULL;
  try { cstr = iapCFG.getString("tcapUser");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("parameter 'tcapUser' isn't set!");

  std::string nmSec(cstr);
  if ((str_cut_blanks(nmSec)).empty())
    throw ConfigException("parameter 'tcapUser' isn't set!");

  cstr = nmSec.c_str();
  if (!rootSec.findSection(cstr))
    throw ConfigException("section %s' is missing!", cstr);
  smsc_log_info(logger, "Reading settings from '%s' ..", cstr);
  /**/
  TCAPUsrCfgParser  parser(logger, cstr);
  parser.readConfig(rootSec, icsCfg.get()->sriCfg); //throws
  /**/
  return ICSrvCfgReaderAC::cfgComplete;
}

} //sri
} //iaprvd
} //inman
} //smsc

