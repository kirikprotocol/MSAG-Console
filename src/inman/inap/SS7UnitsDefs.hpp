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
};

class SS7UnitInstsMap : public std::map<uint8_t /* instId */, SS7UnitInstance> {
public:
  SS7UnitInstance * getInstance(uint8_t inst_id)
  {
    SS7UnitInstsMap::iterator cit = find(inst_id);
    return cit == end() ? 0 : &(cit->second);
  }

  const SS7UnitInstance * findInstance(uint8_t inst_id) const
  {
    SS7UnitInstsMap::const_iterator cit = find(inst_id);
    return cit == end() ? 0 : &(cit->second);
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

