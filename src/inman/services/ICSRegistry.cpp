#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <assert.h>

#include "inman/services/ICSRegistry.hpp"
#include "inman/services/ICSrvLoader.hpp"

namespace smsc  {
namespace inman {
// --------------------------------------------------------------------
//NOTE: ICSLoadupsReg::ICSLoadupsReg() ise defined in SVCHostDflts.cpp
// --------------------------------------------------------------------
/* ************************************************************************** *
 * class ICSLoadupsReg implementation:
 * ************************************************************************** */

ICSLoadupsReg::ICSLoadupCFG * 
  ICSLoadupsReg::findDLL(const char * nm_dll) const
{
  ICSProducersReg::const_iterator it = _prdcReg.begin();
  for (; it != _prdcReg.end(); ++it) {
    ICSUId uid = it->second->icsUId();
    ICSLoadupCFG * ldUp = _knownReg.find(uid); //cann't fail here
    if (ldUp->isMatchedDLL(nm_dll))
      return ldUp;
  }
  return NULL;
}

//Returns state of loaded service.
ICSLoadupsReg::ICSLoadState
  ICSLoadupsReg::getService(ICSUId ics_uid) const /* throw()*/
{
  ICSLoadState res(_prdcReg.find(ics_uid));
  if (res._prod) {
    ICSLoadupCFG * ldUp = _knownReg.find(ics_uid); //cann't fail
    res._nmSec = ldUp->_secNm.empty() ? NULL : ldUp->_secNm.c_str();
  }
  return res;
}

//Loads service, allocates producer and sets parameters for it.
//NOTE: Cann't return NULL, throws in case of any failure!
ICSProducerAC * ICSLoadupsReg::loadService(ICSUId ics_uid, Logger * use_log,
                                           const char * nm_sec/* = NULL*/)
    /* throw(ConfigException)*/
{
  ICSLoadupCFG * ldUp = _knownReg.find(ics_uid);
  if (!ldUp)
    throw ConfigException("ICService '%s' loadUp is not defined!", ICSIdent::uid2Name(ics_uid));

  ICSProducerAC * prod = _prdcReg.find(ics_uid);
  if (prod)
    throw ConfigException("ICService '%s' excessive load request!", ICSIdent::uid2Name(ics_uid));

  if (ldUp->_ldType == ICSLoadupCFG::icsLdDll) {
    //load Dll and get producer
    smsc_log_info(use_log, "Loading ICService driver '%s' ..", ldUp->_loaderDll.c_str());
    prod = ICSrvLoader::LoadICS(ldUp->_loaderDll.c_str()); //throws
  } else
    prod = ldUp->_loaderFp();

  if (!prod)
    throw ConfigException("ICService '%s' loader failed!", ICSIdent::uid2Name(ics_uid));
  _prdcReg.insert(ics_uid, prod);

  if (nm_sec && nm_sec[0])
    ldUp->_secNm = nm_sec;
  smsc_log_info(use_log, "ICService '%s' %s", ICSIdent::uid2Name(ics_uid), ldUp->Details().c_str());
  return prod;
}



//Loads service, allocates producer and sets parameters for it.
//NOTE: Cann't return NULL, throws in case of any failure!
ICSProducerAC * ICSLoadupsReg::loadService(const char * nm_dll, Logger * use_log,
                                           const char * nm_sec/* = NULL*/)
  /* throw(ConfigException)*/
{
  smsc_log_info(use_log, "Loading ICService driver '%s' ..", nm_dll);
  //check for uniqueness of dll name
  ICSLoadupCFG * ldUp = findDLL(nm_dll);
  if (ldUp) //dll is already loaded
    throw ConfigException("ICService driver '%s' is already loaded for '%s'!", nm_dll,
                            ICSIdent::uid2Name(ldUp->_icsUId));

  std::auto_ptr<ICSProducerAC> prod;
  //load dll, get service UId and producer
  prod.reset(ICSrvLoader::LoadICS(nm_dll));
  ICSUId icsUId = prod->icsUId();
  smsc_log_info(use_log, "ICService driver '%s' is loaded, ICSUId: %s", nm_dll,
                ICSIdent::uid2Name(icsUId));

  //verify service loadUp settings (UId and linkage type)
  ldUp = _knownReg.find(icsUId);
  if (!ldUp)
    throw ConfigException("ICService '%s'(%u) loadUp is not defined!",
                          ICSIdent::uid2Name(icsUId), icsUId);
  if (ldUp->_ldType != ICSLoadupCFG::icsLdDll)
    throw ConfigException("ICService '%s' dynamic linkage is not supported",
                          ICSIdent::uid2Name(icsUId));
  //check for uniqueness of UId
  if (_prdcReg.find(icsUId))
    throw ConfigException("ICService '%s' another driver is already loaded",
                          ICSIdent::uid2Name(icsUId));

  if (nm_sec && nm_sec[0])
    ldUp->_secNm = nm_sec;
  _prdcReg.insert(icsUId, prod.release());

  smsc_log_info(use_log, "ICService '%s' %s", ICSIdent::uid2Name(icsUId), ldUp->Details().c_str());
  return _prdcReg.find(icsUId);
}

//Verifies that all default services were loaded, if finds the missed one, then loads it.
//Returns UIds, which were loaded by this call.
ICSIdsSet ICSLoadupsReg::loadDefaults(Logger * use_log)
{
  ICSIdsSet ids;
  for (ICSIdsSet::const_iterator it = _dfltIds.begin(); it != _dfltIds.end(); ++it) {
    const ICSLoadupCFG * ldUp = _knownReg.find(*it);
    assert(ldUp);
    if (!_prdcReg.find(ldUp->_icsUId)) {
      loadService(ldUp->_icsUId, use_log);
      ids.insert(ldUp->_icsUId);
    }
  }
  return ids;
}


} // namespace inman
} // namespace smsc

