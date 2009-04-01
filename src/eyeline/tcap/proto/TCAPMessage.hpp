/* ************************************************************************** *
 * TCAP messages according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_DEFS_HPP

#include "eyeline/ros/ROSComponent.hpp"
#include "eyeline/tcap/TCStrDialogue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::ros::ROSComponentPrimitive;

typedef eyeline::util::RFPtr_T<ROSComponentPrimitive> ROSComponentRfp;
typedef eyeline::util::URFPtr_T<ROSComponentPrimitive> ROSComponentURfp;
typedef eyeline::util::CRFPtr_T<ROSComponentPrimitive> ROSComponentCRfp;

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
    uint8_t pduMem[eyeline::util::MaxSizeOf2_T<ABRT_apdu, TCExternal>::VALUE];
    union {
        ASTypeAC *   ac;    //PDU's base class
        TCAbrtPDU *  dlg;
        TCExternal * usr;
    } pdu;
    AbortOrigin     abrtOrigin;
    P_AbortCause    abrtPrvd;

protected:
    void resetPdu(void)
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

    void Reset(void) { Init(abrtOrgProvider); }

    AbortOrigin  Origin(void) const { return abrtOrigin; }
    P_AbortCause PAbort(void) const { return abrtOrigin; }
    TCAbrtPDU *  DlgAbort(void) const { return abrtOrigin == abrtOrgDialog ? pdu.dlg : 0; }
    TCExternal * UsrAbort(void) const { return abrtOrigin == abrtOrgUser ? pdu.usr : 0; }

    TCUserInformation * UsrInfo(void) const
    {
        return abrtOrigin == abrtOrgDialog ? pdu.dlg->UsrInfo() : 0;
    }
};


//TCAP Begin/Continue/End messages dialogue portion.
//May be the one of APDUs defined in _ac_tcap_strDialogue_as.
typedef TCStrDialogueAS TCMsgDlgPortion;


//TCAP message: aggregates {TransactionId, DialoguePortion, ComponentPortion}.
//DialoguePortion may be one of two alternatives:
// 1) normal dialogue portion (T_Begin/T_Cont/T_End)
// 2) abort dialogue portion (T_Abort)
//
//NOTE: UNIDialogue PDUs are unsupported for now.  (TODO: !!!)
class TCAPMessage : public ASTypeAC {
public:
    enum MKind_e {
        t_none = -1
        //uniDialogue-a messages:
        , t_unidirection = 1    //[APPLICATION 1]
        //dialogue-as messages:
        , t_begin = 2           //[APPLICATION 2]
        , t_end = 4             //[APPLICATION 4]
        , t_continue = 5        //[APPLICATION 5]
        , t_abort = 7           //[APPLICATION 7]
    };

    TransactionId               trId;

private:
    uint8_t pduMem[eyeline::util::MaxSizeOf2_T< TCMsgAbrtPortion, 
                                                    TCMsgDlgPortion >::VALUE];
    union {
        ASTypeAC *          ac; //base class
        TCMsgDlgPortion *   dlg;
        TCMsgAbrtPortion *  abrt;
    } pdu;
    MKind_e               msgKind;


protected:
    std::list<ROSComponentRfp>  compPart;

    void rlseComps(void) { compPart.clear(); }

    void resetPdu(void)
    {
        if (pdu.ac) {
            pdu.ac->~ASTypeAC();
            pdu.ac = 0;
        }
    }

    ASTypeAC * Init(MKind_e use_kind)
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
        //NOTE: if use_kind is not one of specified by constructor options,
        //currently selected tag is set to undefined
        asTags().selectOption(ASTag(ASTag::tagApplication, use_kind));
        return pdu.ac;
    }

public:
    TCAPMessage(MKind_e use_kind = t_none)
    {
        asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_begin));
        asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_end));
        asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_continue));
        asTags().addOption(ASTagging(ASTag::tagApplication, TCAPMessage::t_abort));
        //
        pdu.ac = 0;
        if ((msgKind = use_kind) != use_kind)
            Init(use_kind);
    }
    ~TCAPMessage()
    {
        Reset();
    }

    //
    void Reset(void)
    {
        Init(t_none); rlseComps();
    }

    MKind_e MsgKind(void) const { return msgKind; }
    //
    TCMsgDlgPortion * DlgPortion(void) const
    {
        return ((msgKind == t_begin) || (msgKind == t_continue)
                || (msgKind == t_end)) ? pdu.dlg : 0;
    }
    //
    TCMsgAbrtPortion * AbrtPortion(void) const
    {
        return (msgKind == t_abort) ? pdu.abrt : 0;
    }
    //
    const EncodedOID * ACDefined(void) const
    {
        TCMsgDlgPortion * dlg = DlgPortion();
        return (dlg && dlg->Get()) ? dlg->Get()->ACDefined() : 0;
    }

    TCUserInformation * UsrInfo(void) const
    {
        if (DlgPortion())
            return DlgPortion()->UsrInfo();
        return AbrtPortion() ? AbrtPortion()->UsrInfo() : 0;
    }

    // ***********************************************
    // * Message construction helper methods
    // ***********************************************

    //creates TCAPMessage for initiation of transaction.
    //Desired AC may be specified.
    TCMsgDlgPortion * TBegin(const EncodedOID * app_ctx = 0);
    //creates TCAPMessage for continuing just requested transaction (T_Begin
    //just got with AC). Desired AC must be specified (either the same as
    //requested or another one).
    TCMsgDlgPortion * TContAC(const EncodedOID & app_ctx);
    //creates TCAPMessage for continuing already established transaction
    TCMsgDlgPortion * TCont(void);
    //creates TCAPMessage for T_End(basic)
    TCMsgDlgPortion * TEnd(void);
    //creates TCAPMessage for aborting just requested transaction
    //(T_Begin just got with AC) because of requested AC is unsupported.
    //Suggested AC may be specified (see ITU Q.771 clause 3.1.2.2.2)
    TCMsgAbrtPortion * UAbortAC(const EncodedOID * sugg_ctx = 0);
    //
    TCMsgAbrtPortion * UAbort(void); //TODO: add abort reason

    //Adds refpointer to ROS component to message (utilization depends on RFP kind)
    bool addComponent(const ROSComponentRfp & use_comp)
    {
        compPart.push_back(use_comp);
    }

    // Adds value of type with direct reference (defined OID) to userInfo section
    bool addUserInfo(const AbstractSyntaxRfp & use_asval)
    {
        TCUserInformation * pInfo = UsrInfo();
        if (!pInfo)
            return false;
        pInfo->addUIASyntax(use_asval);
        return true;
    }
    // Adds encoding of arbitrary type to userInfo section
    bool addUserInfo(uint32_t use_id, const BITBuffer & use_enc)
    {
        TCUserInformation * pInfo = UsrInfo();
        if (!pInfo)
            return false;
        pInfo->addUIEncoding(use_id, use_enc);
        return true;
    }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_DEFS_HPP */

