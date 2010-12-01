#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/SvcHostCfgReader.hpp"
#include "inman/services/ICSRegistry.hpp"

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class ICSHostCfgReader implementation:
 * ************************************************************************** */
//Reads info of extra configuration files, parses them and initializes XMFConfig
void ICSHostCfgReader::readFiles(const XConfigView & root_sec) /*throw(ConfigException)*/
{
  XConfigView   fileSec;
  root_sec.getSubConfig(fileSec, "Files");

  smsc_log_info(logger, "Reading settings from 'Files' ..");
  std::auto_ptr<CStrSet>  params(fileSec.getStrParamNames());

  // <param name="SectionName_mask" type="string">File_name</param>
  for (CStrSet::const_iterator it = params->begin(); it != params->end(); ++it) {
    const char * cstr = NULL;
    try { cstr =  fileSec.getString(it->c_str());
    } catch (...) { }
    if (!cstr || !cstr[0])
      throw ConfigException("%s.%s - value is invalid/missing",
                            fileSec.relSection(), it->c_str());

    const XCFConfig * pXcf = _xmfCfg.setSectionsConfig(cstr, it->c_str()); //throws
    if (!pXcf)
      throw ConfigException("%s.%s - duplicate SectionName prefix",
                            fileSec.relSection(), it->c_str());

    smsc_log_info(logger, "  sections '%s' -> %s", it->c_str(), pXcf->first.c_str());
  }
}

//
bool ICSHostCfgReader::markProducer(ICSUId ics_uid)
{
  ICSLoadupsReg::ICSLoadState icsState = ICSLoadupsReg::get().getService(ics_uid);
  if (icsState._prod) {
    icsCfg->prodReg.insert(ics_uid, new ICSProducerCFG(icsState._prod, _xmfCfg, icsState._nmSec, logger));
    return true;
  }
  return false;
}

//Returns true if at least one new service was loaded
bool ICSHostCfgReader::addDfltLoadUps(void) /*throw(ConfigException)*/
{
  ICSIdsSet ids = ICSLoadupsReg::get().loadDefaults(logger);
  if (ids.empty())
    return false;

  for (ICSIdsSet::const_iterator it = ids.begin(); it != ids.end(); ++it) {
    markProducer(*it);
  }
  return true;
}

//Reads 'Services' section containing services loadup configuration
//NOTE: default config section names for services may be overridden
//      only in 'Services' section
void ICSHostCfgReader::readLoadUps(const XConfigView & root_sec) /*throw(ConfigException)*/
{
  if (!root_sec.findSubSection("Services")) {
    smsc_log_warn(logger, "'Services' section is empty or missed!");
#ifdef ICSERVICES_ASSUMED_ON
    ICSLoadupsReg::get().loadService(ICSIdent::icsIdSmBilling, logger);
    markProducer(ICSIdent::icsIdSmBilling);
#endif /* OLD_CODE */
    return;
  }

  XConfigView cfgSub;
  root_sec.getSubConfig(cfgSub, "Services");

  std::auto_ptr<CStrSet>      srvs(cfgSub.getStrParamNames());
  if (srvs->empty()) {
    smsc_log_warn(logger, "'Services' section is empty or missed!");
#ifdef ICSERVICES_ASSUMED_ON
    ICSLoadupsReg::get().loadService(ICSIdent::icsIdSmBilling, logger);
    markProducer(ICSIdent::icsIdSmBilling);
#endif /* OLD_CODE */
    return;
  }
  for (CStrSet::const_iterator it = srvs->begin(); it != srvs->end(); ++it) {
    const char * cstr = cfgSub.getString(it->c_str());

    if (!_xmfCfg.hasSection(it->c_str())) 
      throw ConfigException("%s section is missed", it->c_str());
    ICSUId uid = processICSLoadUp(it->c_str(), cstr, logger);
    markProducer(uid);
  }
}

//Reads or complete reading the configuration of loaded services,
//also loads up services requested by dependencies
void ICSHostCfgReader::readLoadedCfg(void)
{
  ICSProducerCFG * prodCfg = NULL;
  ICSProducersReg::size_type attempt = icsCfg->prodReg.size() + 1;
  while ((prodCfg = icsCfg->prodReg.nextToRead()) != 0) { //xcfReader is set
    if (!--attempt)
      throw ConfigException("'%s' service config parsing failure",
                            ICSIdent::uid2Name(prodCfg->icsUId));
    if (prodCfg->xcfReader->readConfig()) { //throws
      //process requested dependencies
      const ICSArgsMap & deps = prodCfg->xcfReader->Deps().Map();
      for (ICSArgsMap::const_iterator it = deps.begin(); it != deps.end(); ++it) {
        ICSUId reqUId = it->first;
        const ICSCfgArgs & args = it->second;
        ICSProducerCFG * reqCfg = icsCfg->prodReg.find(reqUId);
        if (!reqCfg) { //dependent producer not marked (possibly service was
                       //just loaded by xcfReader->readConfig()
          smsc_log_debug(logger, "'%s' requires the '%s'",
                        ICSIdent::uid2Name(prodCfg->icsUId), ICSIdent::uid2Name(reqUId));

          if (!markProducer(reqUId)) { //service not even loaded
            ICSLoadupsReg::get().loadService(reqUId, logger, args._nmSec.c_str());
            markProducer(reqUId);
          }
          reqCfg = icsCfg->prodReg.find(reqUId);
          ++attempt;
        }
        if (reqCfg->cfgState() != ICSrvCfgReaderAC::cfgComplete)
          reqCfg->xcfReader->addArguments(args._args);
      }
    }
  }
}


//Parses XML configuration entry section, updates dependencies.
//Returns status of config parsing, 
ICSrvCfgReaderAC::CfgState ICSHostCfgReader::parseConfig(void * opaque_arg/* = NULL*/)
  throw(ConfigException)
{
  const char * cstr = NULL;
  try { cstr = _topSec.getString("version");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
      smsc_log_warn(logger, "Config version is not set");
  else
      smsc_log_info(logger, "Config version: %s", cstr);


  //Read info of extra configuration files and initialize XMFConfig
  if (_topSec.findSubSection("Files"))
    readFiles(_topSec); //throws

  //Read initial services loadup configuration
  readLoadUps(_topSec); //throws
  //Read configurations for loaded services and load up services requested
  //by dependencies
  readLoadedCfg(); //throws
  //verify that configurations of all default services were read
  if (addDfltLoadUps())
    readLoadedCfg();
  /**/
  if (icsCfg->prodReg.empty())
    throw ConfigException("No services loadUps processed!");
  return ICSrvCfgReaderAC::cfgComplete;
}

} // namespace inman
} // namespace smsc

