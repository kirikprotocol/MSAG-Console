/* ************************************************************************* *
 * Structures containing parameters required for interaction with HD version
 * of EIN SS7 stack. 
 * ************************************************************************* */
#ifndef __SS7HD_DISPATCHER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SS7HD_DISPATCHER_DEFS_HPP

#include <string>
#include <map>

//#ifdef EIN_HD
//#include "portss7.h"
//#endif /* EIN_HD */

namespace smsc   {
namespace inman  {
namespace inap   {

struct SS7UnitInstance {
    enum ConnectStatus {
        uconnIdle = 0, uconnError, uconnAwaited, uconnOk
    };

    uint8_t         instId;     //SS7 communication unit instanceId, [1..255],
    ConnectStatus   connStatus;

    SS7UnitInstance(uint8_t inst_id = 0)
        : instId(inst_id), connStatus(uconnIdle)
    { }

    SS7UnitInstance(const SS7UnitInstance & unit_inst)
        : instId(unit_inst.instId), connStatus(unit_inst.connStatus)
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

    SS7Unit_CFG(uint16_t unit_id = 0) : unitId(unit_id)
    { }
};

typedef std::map<uint16_t /*unitId*/, SS7Unit_CFG> SS7UnitsCFG;

#ifdef EIN_HD
struct SS7HD_CFG {
    static const uint8_t  _MAX_CPMGR_INSTANCE_ID = 255;
    static const uint8_t  _MAX_UNIT_INSTANCE_ID = 255;
    static const uint8_t  _MIN_USER_ID = 1;
    static const uint8_t  _MAX_USER_ID = 20;
    static const uint16_t _DFLT_APPINSTANCE_ID = 1;
    static const uint8_t  _MAX_APPINSTANCE_ID = 255;
    static const uint16_t _DFLT_MSG_ENTRIES = 512;
    static const uint16_t _MAX_MSG_ENTRIES = 65535;

    std::string nmLayout;       //name of this SS7 units layout
    uint8_t     appInstId;      //local application instanceId, [1..255]
    std::string rcpMgrAdr;      //CSV list of remote CommonParts Managers host:port
    uint8_t     rcpMgrInstId;   //remote CommonParts Manager instanceId, [0..255],
                                //by default: 0
    uint8_t     mpUserId;       //CP userId of local message port owner, [1..20]
    uint16_t    maxMsgNum;      //input SS7 messages buffer size, [1..65535]
    SS7UnitsCFG ss7Units;

    SS7HD_CFG()
        : appInstId(0), rcpMgrInstId(0), mpUserId(0), maxMsgNum(0)
    { }

    //Returns zero if unit_nm is unknown or is not supported
    static uint16_t unitIdByName(const char * unit_nm)
    {
        if (unit_nm && unit_nm[0]) {
            if (!strcmp(unit_nm, "TCAP_ID"))
                return 4; //TCAP_ID;
            if (!strcmp(unit_nm, "SCCP_ID"))
                return 5; //SCCP_ID;
        }
        return 0;
    }
};
#endif /* EIN_HD */

} //inap
} //inman
} //smsc
#endif /* __SS7HD_DISPATCHER_DEFS_HPP */

