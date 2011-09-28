#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapmgr/SKAlgorithms.hpp"

#include "inman/comp/CSIDefs.hpp"
#include "inman/common/RPCList.hpp"
#include "inman/abprov/CSIRecordsMap.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {
/* ************************************************************************** *
 * class SKAlgorithm_SKMap implementation:
 * ************************************************************************** */
// ----------------------------------------
// -- SKAlgorithmAC interface methods
// ----------------------------------------
//arg is type of uint32_t
uint32_t SKAlgorithm_SKMap::getSKey(const void * use_arg/* = NULL*/) const
{
  if (!use_arg)
    return 0;

  uint32_t arg = *(uint32_t*)use_arg;
  SRVKeyMAP::const_iterator it = _skMap.find(arg);
  return it != _skMap.end() ? it->second : 0;
}
//
void  SKAlgorithm_SKMap::print(std::string & out_str) const
{
  out_str += "map{";
  out_str += UnifiedCSI::nmTDP(_argCSI); out_str += ",";
  out_str += UnifiedCSI::nmTDP(csiType()); out_str += "}: {";

  unsigned i = 0;
  for (SRVKeyMAP::const_iterator
       it = _skMap.begin(); it != _skMap.end(); ++it, ++i) {
    char buf[sizeof("%s{%u,%u}, ") + 2*sizeof(uint32_t)*3];
    snprintf(buf, sizeof(buf)-1, "%s{%u,%u}", i ? ", ":"",  it->first, it->second);
    out_str += buf;
  }
  out_str += "}";
}

/* ************************************************************************** *
 * class SKAlgorithmsDb implementation:
 * ************************************************************************** */
uint32_t SKAlgorithmsDb::getSKey(CSIUid_e tgt_csi,
                                 const CSIRecordsMap * org_csis/* = NULL*/) const
{
  SKAlgorithmMAP::const_iterator acit = _algMap.find(tgt_csi);
  if (acit != _algMap.end()) {
    if (acit->second->algId() == SKAlgorithmAC::algSKVal)
      return acit->second->getSKey(NULL);

    if ((acit->second->algId() == SKAlgorithmAC::algSKMap)
        && org_csis && !org_csis->empty()) {
      const SKAlgorithm_SKMap * alg = static_cast<const SKAlgorithm_SKMap *>(acit->second);
      const CSIRecord * pRcd = org_csis->find(alg->argType());
      if (pRcd)
        return alg->getSKey((void*)&(pRcd->scfInfo.serviceKey));
    }
  }
  //there are no translation algorithms for tgt_csi, so just
  //return serviceKey from orgCSIs (if it exists)
  if (org_csis && !org_csis->empty()) {
    const CSIRecord * pRcd = org_csis->find(tgt_csi);
    if (pRcd)
      return pRcd->scfInfo.serviceKey;
  }
  return 0;
}

} //iapmgr
} //inman
} //smsc



