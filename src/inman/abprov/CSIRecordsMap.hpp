/* ************************************************************************** *
 *
 * ************************************************************************** */
#ifndef __SMSC_INMAN_IAPRVD_CSIRECORD_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_IAPRVD_CSIRECORD_HPP

#include <map>

#include "inman/GsmSCFInfo.hpp"
#include "inman/comp/CSIDefs.hpp"
#include "inman/abprov/IAPDefs.hpp"

namespace smsc  {
namespace inman {
namespace iaprvd {

using smsc::inman::comp::CSIUid_e;
using smsc::inman::comp::UnifiedCSI;

struct CSIRecord {
  IAPType_e   iapId;  //type of IAProvider that was used to obtain info about this CSI
  CSIUid_e    csiId;  //unified Id of CSI
  GsmSCFinfo  scfInfo;  //gsmSCF serving this CSI

  static const size_t _strSZ = 
    sizeof("%s{%s}") + UnifiedCSI::_strSZ_TDP + GsmSCFinfo::_strSZ;

  typedef core::buffers::FixedLengthString<_strSZ> StringForm_t;

  CSIRecord() : iapId(IAPProperty::iapUnknown), csiId(UnifiedCSI::csi_UNDEFINED)
  { }
  CSIRecord(IAPType_e iap_id, CSIUid_e csi_id, const GsmSCFinfo & gsm_scf)
    : iapId(iap_id), csiId(csi_id), scfInfo(gsm_scf)
  { }
  ~CSIRecord()
  { }

  void clear(void)
  {
    iapId = IAPProperty::iapUnknown; csiId = UnifiedCSI::csi_UNDEFINED;
    scfInfo.Reset();
  }

  void  toString(StringForm_t & use_str) const
  {
    snprintf(use_str.str, use_str.MAX_SZ-1, "%s{%s}",
            UnifiedCSI::nmTDP(csiId), scfInfo.toString().c_str());
  }

  StringForm_t  toString(void) const
  {
    StringForm_t rval;
    toString(rval);
    return rval;
  }
};

typedef CSIRecord::StringForm_t CSIRecordString_t;

//Defines serving gsmSCF params for DPs of specified category
class CSIRecordsMap : public std::map<CSIUid_e, CSIRecord> {
public:
  CSIRecordsMap() : std::map<CSIUid_e, CSIRecord>()
  { }
  ~CSIRecordsMap()
  { }

  void insertRecord(const CSIRecord & use_rec)
  {
    insert(std::map<CSIUid_e, CSIRecord>::value_type(use_rec.csiId, use_rec));
  }

  size_type Merge(const CSIRecordsMap & use_map)
  {
    for (CSIRecordsMap::const_iterator it = use_map.begin(); it != use_map.end(); ++it)
      insert(*it);
    return size();
  }

  const GsmSCFinfo * getSCFinfo(CSIUid_e csi_id) const
  {
    CSIRecordsMap::const_iterator it = find(csi_id);
    return ((it == end()) || it->second.scfInfo.empty()) ? NULL : &(it->second.scfInfo);
  }
  //
  const CSIRecord * getCSIRecord(CSIUid_e csi_id) const
  {
    CSIRecordsMap::const_iterator it = find(csi_id);
    return ((it == end()) || it->second.scfInfo.empty()) ? NULL : &(it->second);
  }

  //
  CSIRecordString_t toString(CSIUid_e csi_id) const;
  //returns size of string representation of map
  size_t  str_size(void) const;
  //
  std::string toString(void) const;
};

} //iaprvd
} //inman
} //smsc

#endif /* __SMSC_INMAN_IAPRVD_CSIRECORD_HPP */

