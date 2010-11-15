/* ************************************************************************** *
 * gsmSCF serviceKeys determination/translation algorithms.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAPMGR_SKALGORITHMS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPMGR_SKALGORITHMS_HPP

#include "inman/comp/CSIDefs.hpp"
#include "inman/common/RPCList.hpp"
#include "inman/abprov/CSIRecordsMap.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

using smsc::inman::comp::CSIUid_e;
using smsc::inman::comp::UnifiedCSI;

using smsc::inman::iaprvd::CSIRecordsMap;

/* --------------------------------------------------------------------- *
 * gsmSCF serviceKeys determination algorithms.
 * --------------------------------------------------------------------- */
class SKAlgorithmAC {
public:
  enum AlgType_e { algSKVal = 0, algSKMap };

private:
  const AlgType_e   _algId;
  CSIUid_e          _tgtCSI;

protected:
  SKAlgorithmAC(AlgType_e alg_id, CSIUid_e tgt_csi)
    : _algId(alg_id), _tgtCSI(tgt_csi)
  { }

public:
  virtual ~SKAlgorithmAC()
  { }

  AlgType_e algId(void) const { return _algId; }
  CSIUid_e  csiType(void) const { return _tgtCSI; }

  std::string toString(void) const
  {
    std::string outStr;
    toString(outStr);
    return outStr;
  }
  // ----------------------------------------
  // -- SKAlgorithmAC interface methods
  // ----------------------------------------
  //NOTE: argument type depends on algorithm
  virtual uint32_t  getSKey(const void * arg = NULL) const = 0;
  //
  virtual void      toString(std::string & out_str) const = 0;
};

//
class SKAlgorithm_SKVal : public SKAlgorithmAC {
protected:
  uint32_t skVal;

public:
  SKAlgorithm_SKVal(CSIUid_e tgt_csi, uint32_t use_val)
    : SKAlgorithmAC(SKAlgorithmAC::algSKVal, tgt_csi), skVal(use_val)
  { }
  ~SKAlgorithm_SKVal()
  { }

  // ----------------------------------------
  // -- SKAlgorithmAC interface methods
  // ----------------------------------------
  virtual uint32_t  getSKey(const void * arg = NULL) const { return skVal; }
  //
  virtual void      toString(std::string & out_str) const
  {
    char buf[sizeof("val: %u") + sizeof(uint32_t)*3];
    snprintf(buf, sizeof(buf)-1, "val: %u", skVal);
    out_str += buf;
  }
};

//
class SKAlgorithm_SKMap : public SKAlgorithmAC {
protected:
  typedef std::map<uint32_t/*skey_idx*/, uint32_t /*SKeyVal*/> SRVKeyMAP;

  CSIUid_e   _argCSI;
  SRVKeyMAP  _skMap;

public:
  SKAlgorithm_SKMap(CSIUid_e tgt_csi, CSIUid_e arg_csi)
    : SKAlgorithmAC(SKAlgorithmAC::algSKMap, tgt_csi), _argCSI(arg_csi)
  { }
  ~SKAlgorithm_SKMap()
  { }

  CSIUid_e    argType(void) const { return _argCSI; }
  size_t      size(void)  const { return (size_t)_skMap.size(); }

  void insert(uint32_t skey_idx, uint32_t skey_val)
  {
    _skMap.insert(SRVKeyMAP::value_type(skey_idx, skey_val));
  }

  // ----------------------------------------
  // -- SKAlgorithmAC interface methods
  // ----------------------------------------
  //arg is type of uint32_t
  virtual uint32_t  getSKey(const void * use_arg = NULL) const
  {
    if (!use_arg)
      return 0;

    uint32_t arg = *(uint32_t*)use_arg;
    SRVKeyMAP::const_iterator it = _skMap.find(arg);
    return it != _skMap.end() ? it->second : 0;
  }
  //
  virtual void      toString(std::string & out_str) const
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
};

/* --------------------------------------------------------------------- *
 * ServiceKey translation algorithms registry.
 * --------------------------------------------------------------------- */
class SKAlgorithmsDb {
protected:
  typedef std::map<CSIUid_e, SKAlgorithmAC*> SKAlgorithmMAP;

  SKAlgorithmMAP  _algMap;

public:
  SKAlgorithmsDb()
  { }
  ~SKAlgorithmsDb()
  {
    for (SKAlgorithmMAP::iterator it = _algMap.begin(); it != _algMap.end(); ++it)
      delete it->second;
    _algMap.clear();
  }

  unsigned size(void) const { return (unsigned)_algMap.size(); }

  void addAlgorithm(CSIUid_e csi_id, SKAlgorithmAC * sk_alg)
  {
    _algMap.insert(SKAlgorithmMAP::value_type(csi_id, sk_alg));
  }

  uint32_t getSKey(CSIUid_e tgt_csi, const CSIRecordsMap * org_csis = NULL) const
  {
    SKAlgorithmMAP::const_iterator acit = _algMap.find(tgt_csi);
    if (acit != _algMap.end()) {
      if (acit->second->algId() == SKAlgorithmAC::algSKVal)
        return acit->second->getSKey(NULL);

      if ((acit->second->algId() == SKAlgorithmAC::algSKMap) && org_csis) {
        SKAlgorithm_SKMap * alg = static_cast<SKAlgorithm_SKMap *>(acit->second);

        CSIRecordsMap::const_iterator tcit = org_csis->find(alg->argType());
        if (tcit != org_csis->end())
          return alg->getSKey((void*)&(tcit->second.scfInfo.serviceKey));
      }
    }
    return 0;

  }
};


} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_ICS_IAPMGR_SKALGORITHMS_HPP */

