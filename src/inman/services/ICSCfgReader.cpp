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
 * class ICSrvCfgReaderAC implementation:
 * ************************************************************************** */
//Adds arguments which customize config parsing
//NOTE: argument '*' is a reserved one.
void ICSrvCfgReaderAC::addArgument(const std::string & use_arg)
{
  if (icsArg.empty() || icsArg.begin()->compare("*")) {
    if (!use_arg.compare("*"))
      icsArg.clear();
    icsArg.insert(use_arg);
  }
}
//Adds arguments which customize config parsing
void ICSrvCfgReaderAC::addArguments(const CStrList & use_args)
{
  if (use_args.empty())
    return;
  if (!icsArg.empty() && !icsArg.begin()->compare("*"))
    return;
  //first search for "*"
  for (CStrList::const_iterator
       it = use_args.begin(); it != use_args.end(); ++it) {
    if (!it->compare("*")) {
      addArgument(*it);
      return;
    }
  }
  for (CStrList::const_iterator
       it = use_args.begin(); it != use_args.end(); ++it ) {
    icsArg.insert(*it);
  }
}

//Adds arguments which customize config parsing
void ICSrvCfgReaderAC::addArguments(const CStrSet & use_args)
{
  if (use_args.empty())
    return;
  if (!icsArg.empty() && !icsArg.begin()->compare("*"))
    return;
  if (use_args.find("*") == use_args.end())
    icsArg.insert(use_args.begin(), use_args.end());
  else
    addArgument(std::string("*"));
}


//Returns true if service depends on other ones
//Clears arguments upon return
bool ICSrvCfgReaderAC::readConfig(void * opaque_arg/* = NULL*/)
  throw(ConfigException)
{
  if (!_topSec.isAssigned())
    _xmfCfg.getSectionView(_topSec, nmCfgSection());

  if (cfgState != ICSrvCfgReaderAC::cfgComplete) {
    if (!cfgState) {
      if (nmCfgSection() && !_xmfCfg.hasSection(nmCfgSection()))
        throw ConfigException("section is missed: %s", nmCfgSection());
    }
    if (nmCfgSection()) {
      smsc_log_info(logger, "Reading settings from '%s' ..", nmCfgSection());
    } else {
      smsc_log_info(logger, "Reading settings ..");
    }
    cfgState = parseConfig(opaque_arg);
  } else {
    if (nmCfgSection()) {
      smsc_log_info(logger, "Processed settings of '%s' ..", nmCfgSection());
    } else {
      smsc_log_info(logger, "Processed settings ..");
    }
  }
  icsArg.clear();
  return !icsDeps.empty();
}


/* ************************************************************************** *
 * class ICSMultiSectionCfgReaderAC implementation:
 * ************************************************************************** */
//Returns true if service depends on other ones
//Clears arguments upon return
ICSrvCfgReaderAC::CfgState 
  ICSMultiSectionCfgReaderAC::parseConfig(void * opaque_arg/* = NULL*/) throw(ConfigException)
{
  std::auto_ptr<CStrSet> subs(_topSec.getShortSectionNames());

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
    XConfigView subsCfg;
    _topSec.getSubConfig(subsCfg, cit->c_str());
    parseSection(subsCfg, *cit, opaque_arg);
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

