/* ************************************************************************** *
 * ICServices registry definitions and helpers.
 * ************************************************************************** */
#ifndef __INMAN_ICS_REGISTRY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_REGISTRY_HPP

#include "inman/common/ObjRegistryT.hpp"
#include "inman/services/ICSrvDefs.hpp"
#include "util/config/ConfigException.h"

namespace smsc  {
namespace inman {

using smsc::util::POBJRegistry_T;
using smsc::util::config::ConfigException;

//Registry of known ICServices loadups and producers.
class ICSLoadupsReg {
protected:
  //ICService loadUp configuration:
  // linkage type ( static - reference to loading function,
  //                dynamic - name of dynamic library),
  // optional XML config file section name.
  struct ICSLoadupCFG {
    enum LDType_e { //linkage type
        icsLdNone = 0x0
        , icsLdLib = 0x01   //static linkage
        , icsLdDll = 0x02   //dynamic linkage
    };
    ICSUId          _icsUId; //
    LDType_e        _ldType;
    ICSLoader_fp    _loaderFp;   //service loading function
    std::string     _loaderDll;  //shared library name in case of dynamic linkage 
    std::string     _secNm;  //optional XML config section name

    ICSLoadupCFG()
      : _icsUId(ICSIdent::icsIdUnknown), _ldType(icsLdNone), _loaderFp(0)
    { }
    ICSLoadupCFG(ICSUId ics_uid, ICSLoader_fp use_loader, const char * nm_sec = NULL)
      : _icsUId(ics_uid), _ldType(icsLdLib), _loaderFp(use_loader)
    {
      if (nm_sec)
        _secNm = nm_sec;
    }
    ICSLoadupCFG(ICSUId ics_uid, const char * nm_dll, const char * nm_sec = NULL)
        : _icsUId(ics_uid), _ldType(icsLdDll), _loaderFp(0)
    {
      if (nm_sec)
        _secNm = nm_sec;
      _loaderDll = nm_dll;
    }
    ~ICSLoadupCFG()
    { }

    static const char * nmLDType(LDType_e ld_type)
    {
      if (ld_type == icsLdDll)
        return "dynamic";
      if (ld_type == icsLdLib)
        return "static";
      return "none";
    }
    //
    const char * nmLDType(void) const { return nmLDType(_ldType); }
    //
    bool isMatchedDLL(const char * nm_dll) const
    {
      return (_ldType == icsLdDll) && !_loaderDll.compare(nm_dll);
    }
    //
    std::string Details(void) const
    {
      std::string res("LoadUp[");
      res += ICSIdent::uid2Name(_icsUId); res += "]: ";
      res += nmLDType();
      if ((_ldType == icsLdDll) && !_loaderDll.empty()) {
          res += "("; res += _loaderDll; res += ")";
      }
      res += ", cfg(";
      res += _secNm.empty() ? "none" : _secNm.c_str();
      res += ")";
      return res;
    }
  };

  typedef POBJRegistry_T<ICSUId, ICSLoadupCFG>  ICSLoadersReg;
  typedef POBJRegistry_T<ICSUId, ICSProducerAC> ICSProducersReg;

  ICSIdsSet       _dfltIds;   //services loaded by default, statically initialized
  ICSLoadersReg   _knownReg;  //registry of loadUp settings for all known services,
                              //statically initialized 
  ICSProducersReg _prdcReg;   //registry of producers of currently used services


  void insLoadUp(ICSLoadupCFG * ld_up)
  {
    _knownReg.insert(ld_up->_icsUId, ld_up);
  }

  ICSLoadupsReg();
  ~ICSLoadupsReg()
  { }
  void* operator new(size_t);


  //Searches loaded services registry for one that was loaded from dll with given name
  ICSLoadupCFG * findDLL(const char * nm_dll) const;

public:
  struct ICSLoadState {
    ICSProducerAC * _prod;  //service producer if service is loaded
    const char *    _nmSec; //name of service config section,
                            //may be NULL if section isn't defined

    ICSLoadState(ICSProducerAC * use_prod = NULL, const char * use_sec = NULL)
      : _prod(use_prod), _nmSec(use_sec)
    { }
  };

  static ICSLoadupsReg & get(void)
  {
      static ICSLoadupsReg instance;
      return instance;
  }

  ICSIdsSet knownUIds(void) const
  {
    ICSIdsSet ids;
    for (ICSLoadersReg::const_iterator it = _knownReg.begin(); it != _knownReg.end(); ++it)
      ids.insert(it->first);
    return ids;
  }

  //Returns state of loaded service.
  ICSLoadState getService(ICSUId ics_uid) const /* throw()*/;

  //Loads service, allocates producer and sets parameters for it.
  //NOTE: Cann't return NULL, throws in case of any failure!
  ICSProducerAC * loadService(ICSUId ics_uid, Logger * use_log, const char * nm_sec = NULL)
      /* throw(ConfigException)*/;

  //Loads service, allocates producer and sets parameters for it.
  //NOTE: Cann't return NULL, throws in case of any failure!
  ICSProducerAC * loadService(const char * nm_dll, Logger * use_log,
                                    const char * nm_sec = NULL)
    /* throw(ConfigException)*/;

  //Verifies that all default services were loaded, if finds the missed one, then loads it.
  //Returns UIds, which were loaded by this call.
  ICSIdsSet loadDefaults(Logger * use_log);
};


} //inman
} //smsc
#endif /* __INMAN_ICS_REGISTRY_HPP */

