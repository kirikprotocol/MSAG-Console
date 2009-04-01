/* ************************************************************************** *
 * Classes implementing TCAP structured dialogue PDUs according to
 * itu-t recommendation q 773 modules(2) dialoguePDUs(2) version1(1).
 * ************************************************************************** */
#ifndef __TC_STR_DIALOGUE_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_STR_DIALOGUE_DEFS_HPP

#include "eyeline/tcap/TCUserInfo.hpp"
#include "eyeline/util/MaxSizeof.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

extern EncodedOID _ac_tcap_strDialogue_as;

//Base class for dialogue PDUs
class TCDlgPduAC : public ASTypeAC {
public:
    enum PDUKind_e { pduNone = -1, pduAARQ = 0, pduAARE = 1, pduABRT = 4 };

    TCUserInformation   usrInfo;    //optional

    TCDlgPduAC(PDUKind_e use_pdu)
        : ASTypeAC(ASTag::tagApplication, use_pdu)
    { }

    bool hasUsrInfo(void) const
    {
        return !usrInfo.extVals.empty();
    }

    // -- TCDlgPduAC interface methods:
    virtual const EncodedOID * ACDefined(void) const = 0;
};

//Structured TC Dialogue request APDU (AARQ)
class TCReqPDU : public TCDlgPduAC {
public:
    EncodedOID          ac_id; //optional

    TCReqPDU() : TCDlgPduAC(TCDlgPduAC::pduAARQ);
    { }
    ~TCReqPDU()
    { }

    // -- TCDlgPduAC interface methods
    const EncodedOID * ACDefined(void) const
    {
        return ac_id.length() ? &ac_id : 0;
    } 
};

struct AssociateSourceDiagnostic {
    enum DialogueServiceUser {
        dsu_null = 0, dsu_no_reason_given = 1, dsu_ac_not_supported = 2
    } dsuDiagnostic;
    enum DialogueServiceProvider {
        dsp_null = 0, dsp_no_reason_given = 1, dsp_no_dialogue_portion = 2
    } dspDiagnostic;
};

//Structured TC Dialogue response APDU (AARE)
class TCRespPDU : public TCDlgPduAC {
public:
    enum AssociateResult {
        dlgAccepted = 0, dlgRejectPermanent = 1
    };
    EncodedOID                  ac_id; //optional
    AssociateResult             result;
    AssociateSourceDiagnostic   diagnostic;

    TCRespPDU() : TCDlgPduAC(TCDlgPduAC::pduAARE);
    { }
    ~TCRespPDU()
    { }

    // -- TCDlgPduAC interface methods
    const EncodedOID * ACDefined(void) const
    {
        return ac_id.length() ? &ac_id : 0;
    } 
};

//Structured TC Dialogue abort APDU (ABRT)
class TCAbrtPDU : public TCDlgPduAC {
public:
    enum Source { dialogue_service_user = 0, dialogue_service_provider = 1 };

    Source              abrtSource;

    TCAbrtPDU() : TCDlgPduAC(TCDlgPduAC::pduABRT)
        , abrtSource(dialogue_service_provider)
    { }
    ~TCAbrtPDU()
    { }

    // -- TCDlgPduAC interface methods
    const EncodedOID * ACDefined(void) const { return 0; }
};

// TC Structured Dialogue ABSTRACT SYNTAX
class TCStrDialogueAS : public AbstractSyntax {
public:
    typedef TCDlgPduAC::PDUKind_e PDUKind_e;

private:
    uint8_t pduMem[eyeline::util::MaxSizeOf3_T< TCReqPDU,
                                                TCRespPDU, TCAbrtPDU >::VALUE];
    PDUKind_e pduKind;
    union {
        TCDlgPduAC *    ac;     //APDU's base class
        TCReqPDU *      req;    //[APPLICATION 0]
        TCRespPDU *     resp;   //[APPLICATION 1]
        TCAbrtPDU *     abrt;   //[APPLICATION 4]
    } pdu;

protected:
    void resetPdu(void)
    {
        if (pdu.ac) {
            pdu.ac->~ASTypeAC();
            pdu.ac = 0;
        }
    }

public:
    TCStrDialogueAS(PDUKind_e use_kind = pduNone)
        : AbstractSyntax(_ac_tcap_strDialogue_as)
    {
        asTags().addOption(ASTagging(ASTag::tagApplication, TCDlgPduAC::pduAARQ));
        asTags().addOption(ASTagging(ASTag::tagApplication, TCDlgPduAC::pduAARE));
        asTags().addOption(ASTagging(ASTag::tagApplication, TCDlgPduAC::pduABRT));
        //
        pdu.ac = 0;
        if ((pduKind = use_kind) != pduNone)
            Init(use_kind);
    }
    ~TCStrDialogueAS()
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
        asTags().selectOption(ASTag(ASTag::tagApplication, use_kind));
        return pdu.ac;
    }

    void Reset(void) { Init(pduNone); }

    PDUKind_e PDUKind(void) const { return pduKind; }
    TCDlgPduAC * Get(void) const { return pdu.ac; }

    TCReqPDU * Req(void) const { return pduKind == pduAARQ ? pdu.req : 0; }
    TCRespPDU * Resp(void) const { return pduKind == pduAARE ? pdu.resp : 0; }
    TCAbrtPDU * Abrt(void) const { return pduKind == pduABRT ? pdu.abrt : 0; }

    TCUserInformation * UsrInfo(void) const
    {
        return pdu.ac ? &(pdu.ac->usrInfo) : 0;
    }

};

} //proto
} //tcap
} //eyeline

#endif /* __TC_STR_DIALOGUE_DEFS_HPP */

