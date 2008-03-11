#pragma ident "$Id$"
/* ************************************************************************* *
 * TCAP Dispatcher types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_TCAP_DISPATCHER_DEFS_HPP
#define __INMAN_TCAP_DISPATCHER_DEFS_HPP

#include "util/TonNpiAddress.hpp"
using smsc::util::TonNpiAddress;

#include "inman/inap/ACRegDefs.hpp"

namespace smsc    {
namespace inman   {
namespace inap   {

struct TCDsp_CFG {  //SS7 TCAP unit configuration
    static const uint8_t  _MAX_USER_ID = 20;
    static const uint16_t _DFLT_MSG_ENTRIES = 512;
    static const uint16_t _MAX_MSG_ENTRIES = 65535;

    uint8_t   userId;         //PortSS7 user id [1..20]
    uint16_t  maxMsgNum;      //maximum size of input SS7 message buffer

    TCDsp_CFG() : userId(0), maxMsgNum(0)
    { }
};

struct TCAPUsr_CFG { //TCAP User configuration
    static const unsigned  _strSZ =
        (2*3 + 3 + TonNpiAddress::_strSZ + 1);  //"%s:%u{%u}"

    static const uint16_t _DFLT_TCAP_DIALOGS = 800;
    static const uint16_t _MAX_TCAP_DIALOGS = 65510;
    static const uint16_t _DFLT_ROS_TIMEOUT = 20;   //seconds
    static const uint16_t _MAX_ROS_TIMEOUT = 600;   //seconds

    uint8_t         ownSsn;    //local SSN
    uint8_t         fakeSsn;   //SSN to substitute in TCAP dialog org addr,
                                //0 - means not used
    uint16_t        maxDlgId;   //maximum number of simultaneous TC dialogs
    uint16_t        rosTimeout; //optional timeout for ROS operations carried by TCAP
    TonNpiAddress   ownAddr;   //local ISDN address

    TCAPUsr_CFG()
    { maxDlgId = rosTimeout = ownSsn = fakeSsn = 0; }

    int toString(char* buf, int buflen = TCAPUsr_CFG::_strSZ) const
    {
        int n = 0;
        if (fakeSsn)
            n = snprintf(buf, buflen - 1, "%s:%u{%u}",
                         ownAddr.toString().c_str(),
                         (unsigned)ownSsn, (unsigned)fakeSsn);
        else
            n = snprintf(buf, buflen - 1, "%s:%u",
                         ownAddr.toString().c_str(), (unsigned)ownSsn);
        return n;
    }

    inline std::string toString(void) const
    {
        char buf[TCAPUsr_CFG::_strSZ];
        toString(buf, sizeof(buf));
        return buf;
    }
};


class SSNSession;

class TCAPDispatcherITF {
public:
    enum SS7State_T { ss7None = 0
        , ss7INITED     //SS7 communication facility is initialized
        , ss7OPENED     //user input message queue is opened
        , ss7CONNECTED  //user is connected to TCAP unit
    };

    //Returns state of TCAP unit connection
    virtual SS7State_T  ss7State(void) const  = 0;
    //Binds SSN and initializes SSNSession (TCAP dialogs factory)
    virtual SSNSession *
        openSSN(uint8_t ssn_id, uint16_t max_dlg_id = 2000,
                uint16_t min_dlg_id = 1, Logger * uselog = NULL);
    //
    virtual SSNSession* findSession(uint8_t ssn) const = 0;
    //
    virtual ApplicationContextRegistryITF * acRegistry(void) const = 0;
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_DISPATCHER_DEFS_HPP */

