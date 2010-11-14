/* ************************************************************************* *
 * TCAP Dispatcher types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_TCAP_DISPATCHER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_DISPATCHER_DEFS_HPP

#include "logger/Logger.h"
#include "util/TonNpiAddress.hpp"

#include "inman/inap/ACRegDefs.hpp"
#include "inman/inap/SS7HDCfgDefs.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

using smsc::logger::Logger;
using smsc::util::TonNpiAddress;

#ifdef EIN_HD
typedef SS7HD_CFG TCDsp_CFG; //SS7 TCAP dispatcher
#else  /* EIN_HD */
struct TCDsp_CFG {  //SS7 TCAP unit configuration
    static const uint8_t  _MAX_USER_ID = 20;
    static const uint16_t _DFLT_MSG_ENTRIES = 512;
    static const uint16_t _MAX_MSG_ENTRIES = 65535;

    uint8_t   appInstId;    //local application instanceId, default = 0
    uint8_t   mpUserId;     //CP userId of local message port owner, [1..20]
    uint16_t  maxMsgNum;    //maximum size of input SS7 message buffer
    SS7UnitsCFG ss7Units;

    TCDsp_CFG() : appInstId(0), mpUserId(0), maxMsgNum(0)
    { }
};
#endif /* EIN_HD */

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

    int toString(char* buf, unsigned buflen = TCAPUsr_CFG::_strSZ) const
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

    std::string toString(void) const
    {
        char buf[TCAPUsr_CFG::_strSZ];
        toString(buf, (unsigned)sizeof(buf));
        return buf;
    }
};


class SSNSession;

class TCAPDispatcherITF {
protected:
    virtual ~TCAPDispatcherITF() //forbid interface destruction
    { }

public:
    enum DSPState_e {
      dspStopped = 0, dspStopping, dspRunning
    };
    enum SS7State_e { ss7None = 0
        , ss7INITED     = 1 //CP static data is initialized
        , ss7REGISTERED = 2 //remote CP manager and message port owner are registered,
                            //SS7 communication facility is initialized
        , ss7OPENED     = 3 //user input message queue is opened
        , ss7CONNECTED  = 4 //user is connected to at least one TCAP BE unit
    };

    //Returns dispatcher state
    virtual DSPState_e  dspState(void) const = 0;
    //Returns state of TCAP BE unit(s) connection
    virtual SS7State_e  ss7State(void) const = 0;
    //Binds SSN and initializes SSNSession (TCAP dialogs registry/factory)
    virtual SSNSession *
        openSSN(uint8_t ssn_id, uint16_t max_dlg_id = 2000,
                                        Logger * uselog = NULL) = 0;
    //
    virtual SSNSession* findSession(uint8_t ssn) const = 0;
    //
    virtual ApplicationContextRegistryITF * acRegistry(void) const = 0;
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_DISPATCHER_DEFS_HPP */

