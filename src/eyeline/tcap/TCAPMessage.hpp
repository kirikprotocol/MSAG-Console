/* ************************************************************************** *
 * TCAP messages according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_DEFS_HPP

#include "eyeline/util/MaxSizeof.hpp"
#include "eyeline/tcap/TCStrDialogue.hpp"

namespace eyelinecom {
namespace tcap {

//TCAP Abort message dialogue portion.
//May be the one of following alternatives:
//  1) provider abort indicated by P_AbortCause
//  2) structured dialog (_ac_tcap_strDialogue_as) ABRT-apdu
//  3) some user defined pdu
struct TCMsgAbrtPortion : public ASTypeAC {
public:
    enum AbortOrigin {
        abrtOrgProvider = 0    //provider abort indicated by P_AbortCause
        , abrtOrgDialog        //structured dialog (_ac_tcap_strDialogue_as) ABRT-apdu
        , abrtOrgUser          //some user defined pdu
    };
    enum P_AbortCause {
        p_unrecognizedMessageType = 0
        , p_unrecognizedTransactionID = 1
        , p_badlyFormattedTransactionPortion = 2
        , p_incorrectTransactionPortion = 3
        , p_resourceLimitation = 4
        , p_maxCause = 127 //just a max cap
    };

private:
    uint8_t pduMem[eyelinecom::util::MaxSizeOf2_T<ABRT_apdu, TCExternal>::VALUE];
    union {
        ASTypeAC *   ac;    //PDU's base class
        TCAbrtPDU *  dlg;
        TCExternal * usr;
    } pdu;
    AbortOrigin     abrtOrigin;
    P_AbortCause    abrtPrvd;

protected:
    inline void resetPdu(void)
    {
        if (pdu.ac) {
            pdu.ac->~ASTypeAC();
            pdu.ac = 0;
        }
    }

public:
    TCMsgAbrtPortion(AbortOrigin use_kind = abrtOrgProvider)
        : ASTypeAC(ASTag::tagApplication, 7)
    {
        pdu.ac = 0;
        Init(use_kind);
    }
    ~TCMsgAbrtPortion()
    {
        resetPdu();
    }

    void Init(AbortOrigin use_kind)
    {
        resetPdu();
        switch ((abrtOrigin = use_kind)) {
        case abrtOrgDialog: pdu.ac = new(pduMem)TCAbrtPDU();
        break;
        case abrtOrgUser: pdu.ac = new(pduMem)TCExternal();
        break;
        default: //abrtOrgProvider
            abrtPrvd = p_maxCause;
        }
        return;
    }

    inline void Reset(void) { Init(abrtOrgProvider); }

    inline AbortOrigin Origin(void) const { return abrtOrigin; }

    inline P_AbortCause  PAbort(void) const { return abrtOrigin; }
    inline TCAbrtPDU *  DlgAbort(void) const { return abrtOrigin == abrtOrgDialog ? pdu.dlg : 0; }
    inline TCExternal * UsrAbort(void) const { return abrtOrigin == abrtOrgUser ? pdu.usr : 0; }
};


//TCAP Begin/Continue/End messages dialogue portion.
//May be the one of APDUs defined in _ac_tcap_strDialogue_as.
class TCMsgDlgPortion : public ASTypeAC {
public:
    enum PDUKind_e { pduNone = -1, pduAARQ = 0, pduAARE = 1, pduABRT = 4 };

private:
    uint8_t pduMem[eyelinecom::util::MaxSizeOf3_T< TCReqPDU,
                                                TCRespPDU, TCAbrtPDU >::VALUE];
    PDUKind_e pduKind;
    union {
        TCDlgPdu_AC *   ac;     //APDU's base class
        TCReqPDU *      req;    //[APPLICATION 0]
        TCRespPDU *     resp;   //[APPLICATION 1]
        TCAbrtPDU *     abrt;   //[APPLICATION 4]
    } pdu;

protected:
    inline void resetPdu(void)
    {
        if (pdu.ac) {
            pdu.ac->~ASTypeAC();
            pdu.ac = 0;
        }
    }

public:
    TCMsgDlgPortion(PDUKind_e use_kind = pduNone)
        : AbstractSyntax(_ac_tcap_strDialogue_as)
    {
        pdu.ac = 0;
        Init(use_kind);
    }
    ~TCMsgDlgPortion()
    {
        resetPdu();
    }

    ASTypeAC * Init(PDUKind_e use_kind)
    {
        resetPdu();
        switch ((pduKind = use_kind)) {
        case pduAARQ: pdu.ac = new(pduMem)TCReqPDU();
        break;
        case pduAARE: pdu.ac = new(pduMem)TCRespPDU();
        break;
        case pduABRT: pdu.ac = new(pduMem)TCAbrtPDU();
        break
        default:; //pduNone
        }
        return pdu.ac;
    }

    inline void Reset(void) { Init(pduNone); }

    inline PDUKind_e PDUKind(void) const { return pduKind; }
    inline TCDlgPdu_AC * Get(void) const { return pdu.ac; }

    inline TCReqPDU * Req(void) const { return pduKind == pduAARQ ? pdu.req : 0; }
    inline TCRespPDU * Resp(void) const { return pduKind == pduAARE ? pdu.resp : 0; }
    inline TCAbrtPDU * Abrt(void) const { return pduKind == pduABRT ? pdu.abrt : 0; }
};


//TCAP message: aggregates {TransactionId, DialoguePortion, ComponentPortion}.
//DialoguePortion may be one of two alternatives:
// 1) normal dialogue portion (T_Begin/T_Cont/T_End)
// 2) abort dialogue portion (T_Abort)
//
//NOTE: UNIDialogue PDUs are unsupported for now.
//
//TODO: add helper methods for initialization as:
// 1) T_Begin with AC
// 2) T_Cont with AC
// 3) T_Abort by provider
// 4) T_Abort by user
class TCAPMessage : public ASTypeAC {
public:
    enum MKind {
        t_none = -1
        //uniDialogue-a messages:
        , t_unidirection = 2    //[APPLICATION 1]
        //dialogue-as messages:
        , t_begin = 2           //[APPLICATION 2]
        , t_end = 4             //[APPLICATION 4]
        , t_continue = 5        //[APPLICATION 5]
        , t_abort = 7           //[APPLICATION 7]
    };

private:
    uint8_t pduMem[eyelinecom::util::MaxSizeOf2_T< TCMsgAbrtPortion, 
                                                    TCMsgDlgPortion >::VALUE];
    union {
        ASTypeAC *          ac; //base class
        TCMsgDlgPortion *   dlg;
        TCMsgAbrtPortion *  abrt;
    } pdu;
    MKind               msgKind;

protected:
    inline void resetPdu(void)
    {
        if (pdu.ac) {
            pdu.ac->~ASTypeAC();
            pdu.ac = 0;
        }
    }

public:
    TransactionId               trId;
    std::list<ROSComponentAC*>  compPart; //TODO: add possibility to delete comps
                                          //in TCAPMessage destructor

    TCAPMessage(MKind use_kind = t_none)
        : msgKind(use_kind)
    {
        pdu.ac = 0;
        Init(use_kind);
    }
    ~TCAPMessage()
    {
        resetPdu();
    }

    ASTypeAC * Init(MKind use_kind)
    {
        resetPdu();
        switch ((msgKind = use_kind)) {
        case t_begin:
        case t_continue:
        case t_end:
            pdu.ac = new(pduMem)TCMsgDlgPortion();
        break;
        case t_abort:
            pdu.ac = new(pduMem)TCMsgAbrtPortion();
        break
        default:; //t_none, t_unidirection
        }
        return pdu.ac;
    }

    inline MKind MsgKind(void) const { return msgKind; }
    inline void Reset(void) { Init(t_none); }

    inline TCMsgDlgPortion * DlgPortion(void)
    {
        return ((msgKind == t_begin) || (msgKind == t_continue)
                || (msgKind == t_end)) ? pdu.dlg : 0;
    }
    inline TCMsgAbrtPortion * AbrtPortion(void)
    {
        return (msgKind == t_begin) ? pdu.abrt : 0; 
    }

    inline const EncodedOID * ACDefined(void) const
    {
        TCMsgDlgPortion * dlg = DlgPortion();
        return (dlg && dlg->Get()) ? dlg->Get()->ACDefined() : 0;
    }
};

} //tcap
} //eyelinecom

#endif /* __TCAP_MESSAGE_DEFS_HPP */

