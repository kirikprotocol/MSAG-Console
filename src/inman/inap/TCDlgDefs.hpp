/* ************************************************************************* *
 * TCDialog states definition and helper classes for its analysis
 * ************************************************************************* */
#ifndef __SMSC_INMAN_TCAP_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCAP_DEFS__

#include <string>

namespace smsc {
namespace inman {
namespace inap {


//Ident of TCAP dialogue controlled by HD variant of TCAP BE
struct TCDialogID {
    uint8_t     tcInstId;   //instanceId of TCAP unit controlling this dialog
    uint16_t    dlgId;      //

    TCDialogID()
        : tcInstId(0), dlgId(0)
    { }
    TCDialogID(uint8_t tc_inst_id, uint16_t dlg_id)
        : tcInstId(tc_inst_id), dlgId(dlg_id)
    { }

    bool operator< (const TCDialogID & obj2) const
    {
        if (tcInstId < obj2.tcInstId)
            return true;
        if (tcInstId > obj2.tcInstId)
            return false;
        return (dlgId < obj2.dlgId);
    }
    bool operator== (const TCDialogID & obj2) const
    {
        if (tcInstId != obj2.tcInstId)
            return false;
        return (dlgId == obj2.dlgId);
    }
};

struct TCDlgStateS {
#define TCAP_DLG_COMP_LAST  2
#define TCAP_DLG_COMP_WAIT  1
    /* ... */
    unsigned int dlgLInited : 2;    //1: T_BEGIN_REQ 2: + INVOKE_REQ
    unsigned int dlgRInited : 2;    //1: T_CONT_IND, 2: + INVOKE_IND
    unsigned int dlgLContinued : 2; //1: T_CONT_REQ 2: + INVOKE_REQ
    unsigned int dlgRContinued : 2; //1: T_CONT_IND, 2: + INVOKE_IND
    /* ... */
    unsigned int dlgLEnded : 2;     //1: T_END_REQ,  2: + INVOKE_IND
    unsigned int dlgREnded : 2;     //1: T_END_IND, 2: + INVOKE_IND
    unsigned int dlgLUAborted : 1;  //1: T_U_ABORT_REQ
    unsigned int dlgRUAborted : 1;  //1: T_U_ABORT_IND
    unsigned int dlgPAborted : 1;   //1: T_P_ABORT_IND
    unsigned int reserved    : 1;   //
};

typedef union {
    unsigned int    mask;
    TCDlgStateS     s;
} TCDlgStateT;

class TCDlgStateMASK {
private:
    unsigned int    _inited;
    unsigned int    _continued;
    unsigned int    _ended;
    unsigned int    _aborted;
    unsigned int    _closed;

public:
    TCDlgStateMASK()
    {
        //compute masks for TCDlgStateS in order to avoid care on endianness.
        TCDlgStateT     usv;
        //inited
        usv.mask = 0;
        usv.s.dlgLInited = usv.s.dlgRInited = 0x3;
        _inited = usv.mask;
        //continued
        usv.mask = 0;
        usv.s.dlgLContinued = usv.s.dlgRContinued = 0x3;
        _continued = usv.mask;
        //ended
        usv.mask = 0;
        usv.s.dlgLEnded = usv.s.dlgREnded = 0x3;
        _ended = usv.mask;
        //aborted
        usv.mask = 0;
        usv.s.dlgLUAborted = usv.s.dlgRUAborted = usv.s.dlgPAborted = 1;
        _aborted = usv.mask;
        //closed
        _closed = _aborted | _ended;
    }

    unsigned int    _initedMask()       const { return _inited;}
    unsigned int    _continuedMask()    const { return _continued; }
    unsigned int    _endedMask()        const { return _ended; }
    unsigned int    _abortedMask()      const { return _aborted; }
    unsigned int    _closedMask()       const { return _closed; }

};

extern TCDlgStateMASK   _TCDlgStateMasks; //Must be initialized somewhere!

class TCDlgState {
private:
    unsigned int    _inited;
    unsigned int    _continued;
    unsigned int    _ended;
    unsigned int    _aborted;
    unsigned int    _closed;

public:
    TCDlgStateT    value;

    enum Stage_e { dlgIdle = 0,
        dlgInited, dlgContinued, dlgEnded, dlgAborted, dlgClosed
    };

    TCDlgState()
    {
        value.mask = 0;
    }
    ~TCDlgState()
    { }


    bool isInited(void) const     { return (bool)(value.mask & _TCDlgStateMasks._initedMask()); }
    bool isContinued(void) const  { return (bool)(value.mask & _TCDlgStateMasks._continuedMask()); }
    bool isEnded(void) const      { return (bool)(value.mask & _TCDlgStateMasks._endedMask()); }
    bool isAborted(void) const    { return (bool)(value.mask & _TCDlgStateMasks._abortedMask()); }
    bool isClosed(void) const     { return (bool)(value.mask & _TCDlgStateMasks._closedMask()); }

    bool Is(Stage_e check_state) const
    {
        switch (check_state) {
        case dlgInited:     return isInited();
        case dlgContinued:  return isContinued();
        case dlgEnded:      return isEnded();
        case dlgAborted:    return isAborted();
        case dlgClosed:     return isClosed();
        default:; //        case dlgIdle:
        }
        return !value.mask; //dlgIdle
    }


    bool indInvokes(void) const { return ((value.s.dlgRInited > 1)
                                          || (value.s.dlgRContinued > 1)
                                          || (value.s.dlgREnded > 1)); }
    bool reqInvokes(void) const { return ((value.s.dlgLInited > 1)
                                          || (value.s.dlgLContinued > 1)
                                          || (value.s.dlgLEnded > 1)); }

    std::string Print(void) const
    {
        char buf[32];
        short i = 0;
        buf[i++] = 0x30 + value.s.dlgLInited; 
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgRInited;
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgLContinued;
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgRContinued;
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgLEnded;
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgREnded;
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgLUAborted;
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgRUAborted;
        buf[i++] = ','; buf[i++] = 0x30 + value.s.dlgPAborted;
        buf[i] = 0;
        return buf;
    }
};


} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCAP_DEFS__ */

