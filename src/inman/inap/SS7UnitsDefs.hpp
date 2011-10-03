/* ************************************************************************* *
 * EIN SS7 stack units descriptive structures.
 * ************************************************************************* */
#ifndef __INMAN_SS7_UNITS_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SS7_UNITS_DEFS_HPP

#include <inttypes.h>
#include <map>

#include "core/buffers/LWArrayTraitsInt.hpp"
#include "core/buffers/LWArrayT.hpp"

namespace smsc   {
namespace inman  {
namespace inap   {

struct SS7UnitInstance {
  enum ConnectStatus {
    uconnIdle = 0, uconnError, uconnAwaited, uconnOk
  };

  uint8_t         instId;     //SS7 communication unit instanceId, [1..255],
  ConnectStatus   connStatus;

  explicit SS7UnitInstance(uint8_t inst_id = 0)
    : instId(inst_id), connStatus(uconnIdle)
  { }

  bool operator< (const SS7UnitInstance & cmp_obj) const
  {
    return instId < cmp_obj.instId;
  }
};

class SS7UnitInstsMap {
protected:
  typedef smsc::core::buffers::LWArray_T<
    SS7UnitInstance, uint8_t, 8, smsc::core::buffers::LWArrayTraitsPOD_T
  > UInstArray;

  UInstArray  mArr; //sorted array

  UInstArray::size_type findPos(uint8_t inst_id) const
  {
    if (!mArr.empty()) {
      UInstArray::size_type atIdx = (mArr.size() - 1) >> 1; //approximately middle position
      if (mArr.get()[atIdx].instId == inst_id)
        return atIdx;

      if (mArr.get()[atIdx].instId < inst_id) { //go to the end
        while (++atIdx < mArr.size()) {
          if (mArr.get()[atIdx].instId == inst_id)
            return atIdx;
        }
      } else {  //go to the start
        while (atIdx) {
          if (mArr.get()[--atIdx].instId == inst_id)
            return atIdx;
        }
      }
    }
    return mArr.npos();
  }

public:
  typedef uint8_t size_type;

  SS7UnitInstsMap()
  { }
  ~SS7UnitInstsMap()
  { }

  size_type npos(void) const { return mArr.npos(); }

  bool empty(void) const { return mArr.empty(); }
  size_type size(void) const { return mArr.size(); }

  //Returns npos() in case of failure
  size_type insert(const SS7UnitInstance & unit_inst) { return mArr.insert(unit_inst); }

  const SS7UnitInstance & operator[] (size_type at_idx) const /*throw(std::exception)*/
  {
    return mArr.at(at_idx);
  }

  //NOTE: throws if there is no assigned element at given pos!!!
  SS7UnitInstance & operator[] (size_type at_idx) /*throw(std::exception)*/
  {
    mArr.verifyIndex(at_idx, "SS7UnitInstsMap"); //throws
    return mArr.at(at_idx);
  }

  SS7UnitInstance * getInstance(uint8_t inst_id)
  {
    UInstArray::size_type atPos = findPos(inst_id);
    return (atPos == mArr.npos()) ? 0 : mArr.getBuf() + atPos;
  }

  const SS7UnitInstance * findInstance(uint8_t inst_id) const
  {
    UInstArray::size_type atPos = findPos(inst_id);
    return (atPos == mArr.npos()) ? 0 : mArr.get() + atPos;
  }

  bool isStatus(uint8_t inst_id, SS7UnitInstance::ConnectStatus use_status) const
  {
    const SS7UnitInstance * pInst = findInstance(inst_id);
    return (pInst && (pInst->connStatus == use_status));
  }
};


struct SS7Unit_CFG {
  uint16_t        unitId;     //ident of interacted communication unit, see portss7.h
  SS7UnitInstsMap instIds;    //SS7 communication unit instanceIds, [1..255]

  explicit SS7Unit_CFG(uint16_t unit_id = 0) : unitId(unit_id)
  { }
};

typedef std::map<uint16_t /*unitId*/, SS7Unit_CFG> SS7UnitsCFG;


struct SS7ConnParms {  //common SS7 units interoperation parameters
  static const uint8_t  _MAX_APPINSTANCE_ID = 255;
  static const uint8_t  _MAX_UNIT_INSTANCE_ID = 255;

  static const uint8_t  _MIN_USER_ID = 1;
  static const uint8_t  _MAX_USER_ID = 20;

  static const uint16_t _DFLT_MSG_ENTRIES = 512;
  static const uint16_t _MAX_MSG_ENTRIES = 65535;

  static const uint16_t _MIN_TMO_RECONN = 500;
  static const uint16_t _MAX_TMO_RECONN = 65535;
  static const uint16_t _DFLT_TMO_RECONN = 800;

  uint8_t   appInstId;  //local application instanceId
  uint8_t   mpUserId;   //CP userId of local message port owner, [1..20]
  uint16_t  maxMsgNum;  //maximum size of input SS7 message buffer
  uint16_t  tmoReconn;  //unit reconnection timeout (millisecs),
                        //[500..65535], default 800
  SS7UnitsCFG ss7Units;

  SS7ConnParms() : appInstId(0), mpUserId(0), maxMsgNum(0), tmoReconn(0)
  { }
};


} //inap
} //inman
} //smsc
#endif /* __INMAN_SS7_UNITS_DEFS_HPP */

