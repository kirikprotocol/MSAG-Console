#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/ICSRegistry.hpp"

#include "util/csv/CSVArrayOf.hpp"

namespace smsc  {
namespace inman {

//Parses ICService loadUp parameter and loads required service
//loadUp parameter has format: "CfgSectionName" = "LoadUpValue"
//LoadUpValue has one of two forms:
//  "[uid:] icsUid_name" or "dll: so_lib_name"
ICSUId processICSLoadUp(const char * nm_sec, const char * value, Logger * use_log)
    /* throw(ConfigException)*/
{
  util::csv::CSVArrayOfStr prm(2, ':');
  if (!prm.fromStr(value))
    throw ConfigException("loadUp value '%s' is invalid", value);

  ICSUId uid = ICSIdent::icsIdUnknown;
  if ((prm.size() == 1) || !prm[0].compare("uid")) {
    const char * strUid = (prm.size() == 1) ? prm[0].c_str() : prm[1].c_str();
    uid = ICSIdent::name2UId(strUid);
    if (uid == ICSIdent::icsIdUnknown) {
      smsc_log_fatal(use_log, "service UId '%s' is invalid", strUid);
      smsc_log_info(use_log, "Known UIds: %s", ICSLoadupsReg::get().knownUIds().toString().c_str());
      throw ConfigException("service UId '%s' is invalid", strUid);
    }
    uid = ICSLoadupsReg::get().loadService(uid, use_log, nm_sec)->icsUId();
  } else if (!prm[0].compare("dll")) {
    uid = ICSLoadupsReg::get().loadService(prm[1].c_str(), use_log, nm_sec)->icsUId();
  } else
    throw ConfigException("loadUp value '%s' is invalid", value);

  return uid;
}

/* ************************************************************************** *
 * class ICSMultiSectionCfgReaderAC implementation:
 * ************************************************************************** */
//Returns true if service depends on other ones
//Clears arguments upon return
ICSrvCfgReaderAC::CfgState 
  ICSMultiSectionCfgReaderAC::parseConfig(void * opaque_arg/* = NULL*/) throw(ConfigException)
{
  if (!_cfgXCV.get())
    _cfgXCV.reset(new XConfigView(this->rootSec, this->nmCfgSection()));

  std::auto_ptr<CStrSet> subs(_cfgXCV->getShortSectionNames());

  const CStrSet * nm_lst;
  if (this->icsArg.empty() || !this->icsArg.begin()->compare("*")) {
    nm_lst = subs.get();
  } else {
    nm_lst = &(this->icsArg);
    //check for requested sections presence
    for (CStrSet::const_iterator cit = this->icsArg.begin();
                                    cit != this->icsArg.end(); ++cit) {
      if (subs->find(*cit) == subs->end())
        throw ConfigException("subsection is missed: %s", cit->c_str());
    }
  }
  //parse requested sections
  for (CStrSet::const_iterator cit = nm_lst->begin(); cit != nm_lst->end(); ++cit) {
    std::auto_ptr<XConfigView> subsCfg(_cfgXCV->getSubConfig(cit->c_str()));
    parseSection(subsCfg.get(), *cit, opaque_arg);
  }

  //check overall state
  ICSrvCfgReaderAC::CfgState nextState = ICSrvCfgReaderAC::cfgComplete;
  for (CStrSet::const_iterator cit = subs->begin(); cit != subs->end(); ++cit) {
    const CfgParsingResult * state = sectionState(*cit);
    if (!state || (state->cfgState != ICSrvCfgReaderAC::cfgComplete)) {
      nextState = ICSrvCfgReaderAC::cfgPartial;
      break;
    }
  }
  return nextState;
}



} // namespace inman
} // namespace smsc

