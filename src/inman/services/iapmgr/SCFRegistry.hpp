/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) types, interfaces and
 * helpers definitions.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_SCF_REGISTRY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMANAGER_SCF_REGISTRY_HPP

#include "inman/common/ObjRegistryT.hpp"
#include "inman/services/iapmgr/InScfCfg.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

using smsc::util::TonNpiAddress;

typedef std::map<TonNpiAddress, const INScfCFG * /*params*/> SCFParmsMap;

class SCFRegistry {
protected:
  typedef smsc::util::POBJRegistry_T<INScfIdent_t, INScfCFG>  INCfgRegistry;

  INCfgRegistry _cfgReg;
  SCFParmsMap   _scfMap; //keeps references to _cfgReg

public:
  SCFRegistry()
  { }
  ~SCFRegistry()
  { }

  void insertParms(INScfCFG * scf_cfg)
  {
    _cfgReg.insert(scf_cfg->_ident, scf_cfg);
    _scfMap.insert(SCFParmsMap::value_type(scf_cfg->_scfAdr, (const INScfCFG *)scf_cfg));
  }

  const INScfCFG * insertAlias(INScfCFG * scf_cfg, const INScfIdent_t & nm_alias)
  {
    scf_cfg->_isAlias = _cfgReg.find(nm_alias);
    if (scf_cfg->_isAlias) {
      _cfgReg.insert(scf_cfg->_ident, scf_cfg);
      _scfMap.insert(SCFParmsMap::value_type(scf_cfg->_scfAdr, scf_cfg->_isAlias));
    }
    return scf_cfg->_isAlias;
  }
  
  //NOTE: Nether returns 'aliasFor' form configuration
/*  const INScfCFG * getScfParms(const INScfIdent_t & scf_id) const
  {
    const INScfCFG * pCfg = _cfgReg.find(scf_id);
    return (pCfg && pCfg->_isAlias) ? pCfg->_isAlias : pCfg;
  } */
  //NOTE: Nether returns 'aliasFor' form configuration
  const INScfCFG * getScfParms(const TonNpiAddress & scf_adr) const
  {
    SCFParmsMap::const_iterator it = _scfMap.find(scf_adr);
    return (it == _scfMap.end()) ? NULL : it->second;
  }

  const INScfCFG * exportScfParms(const INScfIdent_t & scf_id, SCFParmsMap & parms_map) const
  {
    const INScfCFG * pCfg = _cfgReg.find(scf_id);
    if (pCfg) {
      const INScfCFG * parmsCfg = pCfg->_isAlias ? pCfg->_isAlias : pCfg;
      parms_map.insert(SCFParmsMap::value_type(pCfg->_scfAdr, parmsCfg));
      return parmsCfg;
    }
    return NULL;
  }

  unsigned exportScfParms(SCFParmsMap & parms_map) const
  {
    parms_map = _scfMap;
    return (unsigned)parms_map.size();
  }
};


} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_SCF_REGISTRY_HPP */

