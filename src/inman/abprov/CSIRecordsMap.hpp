/* ************************************************************************** *
 *
 * ************************************************************************** */
#ifndef __SMSC_INMAN_IAPRVD_CSIRECORD_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_IAPRVD_CSIRECORD_HPP

#include "core/buffers/LWArrayTraitsInt.hpp"
#include "core/buffers/LWArrayT.hpp"

#include "inman/GsmSCFInfo.hpp"
#include "inman/comp/CSIDefs.hpp"
#include "inman/abprov/IAPDefs.hpp"

namespace smsc  {
namespace inman {
namespace iaprvd {

using smsc::inman::comp::CSIUid_e;
using smsc::inman::comp::UnifiedCSI;

struct CSIRecord {
  CSIUid_e    csiId;  //unified Id of CSI
  IAPType_e   iapId;  //type of IAProvider that was used to obtain info about this CSI
  GsmSCFinfo  scfInfo;  //gsmSCF serving this CSI

  static const size_t _strSZ = 
    sizeof("%s{%s}") + UnifiedCSI::_strSZ_TDP + GsmSCFinfo::_strSZ;

  typedef core::buffers::FixedLengthString<_strSZ> StringForm_t;

  explicit CSIRecord(CSIUid_e csi_uid = UnifiedCSI::csi_UNDEFINED)
    : csiId(csi_uid), iapId(IAPProperty::iapUnknown)
  { }
  CSIRecord(CSIUid_e csi_id, IAPType_e iap_id, const GsmSCFinfo & gsm_scf)
    : csiId(csi_id), iapId(iap_id), scfInfo(gsm_scf)
  { }
  ~CSIRecord()
  { }

  void clear(void)
  {
    csiId = UnifiedCSI::csi_UNDEFINED;
    iapId = IAPProperty::iapUnknown;
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

  bool operator< (const CSIRecord & cmp_obj) const
  {
    return csiId < cmp_obj.csiId;
  }
};

typedef CSIRecord::StringForm_t CSIRecordString_t;

//NOTE: CSIRecordsMap operates with values of CSIUid_e < 0xFF
class CSIRecordsMap {
protected:
  typedef smsc::core::buffers::LWArray_T<
    uint8_t, uint8_t, UnifiedCSI::k_lastCSI_Id + 1, smsc::core::buffers::LWArrayTraitsPOD_T
  > RecordsIdxMap;
  typedef smsc::core::buffers::LWArray_T<
    CSIRecord, uint8_t, 2, smsc::core::buffers::LWArrayTraitsPOD_T
  > RecordsArray;

  RecordsIdxMap mIdxMap;
  RecordsArray  mRcdArr;

public:
  typedef RecordsArray::size_type size_type;
  //
  struct InsertResult {
    CSIRecord * mpRcd;
    bool        mIsOvr;
    
    explicit InsertResult(CSIRecord * p_rcd = 0, bool is_ovr = false)
      : mpRcd(p_rcd), mIsOvr(is_ovr)
    { }
  };

  CSIRecordsMap()
  { }
  ~CSIRecordsMap()
  { }

  void clear(void) { mIdxMap.clear(); mRcdArr.clear(); }

  //Returns pointer to record if succeeded and true if record is overwritten.
  //If requested CSIUid_e if beyond the allowed range, NULL is returned.
  InsertResult insert(const CSIRecord & use_rec) /*throw()*/;

  //Returns number of record that were overwritten.
  size_type insert(const CSIRecordsMap & use_map) /*throw()*/;

  //Returns initialized/assigned element of map associated wit given key.
  //if target element isn't assigned yet, it's initialized by default value.
  //NOTE: throws if requested CSIUid_e if beyond the allowed range.
  CSIRecord & at(CSIUid_e csi_id) /*throw(std::exception)*/;

  CSIRecord & operator[] (CSIUid_e csi_id) /*throw(std::exception)*/
  {
    return at(csi_id);
  }

  //
  bool empty(void) const { return mRcdArr.empty(); }
  //
  size_type size(void) const { return mRcdArr.size(); }
  
  //
  const CSIRecord * find(CSIUid_e csi_id) const
  {
    return ((csi_id < mIdxMap.size()) && (mIdxMap[csi_id])) ? &(mRcdArr[mIdxMap[csi_id] - 1]) : NULL;
  }
  //
  const GsmSCFinfo * getSCFinfo(CSIUid_e csi_id) const
  {
    return ((csi_id < mIdxMap.size()) && (mIdxMap[csi_id])) ? &(mRcdArr[mIdxMap[csi_id] - 1].scfInfo) : NULL;
  }
  //Returns first assigned CSI record.
  const CSIRecord * begin(void) const;
  //Returns assigned CSI record with id next to given one.
  const CSIRecord * next(CSIUid_e csi_id) const;

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

