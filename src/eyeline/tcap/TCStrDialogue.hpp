/* ************************************************************************** *
 * Classes implementing TCAP structured dialogue PDUs according to
 * itu-t recommendation q 773 modules(2) dialoguePDUs(2) version1(1).
 * ************************************************************************** */
#ifndef __TC_STR_DIALOGUE_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_STR_DIALOGUE_DEFS_HPP

#include "eyeline/tcap/TCUserInfo.hpp"

namespace eyelinecom {
namespace tcap {

using eyelinecom::asn1::EncodedOID;

//using eyelinecom::asn1::AbstractSyntax;

extern EncodedOID _ac_tcap_strDialogue_as;

class TCDlgPdu_AC : public ASTypeAC {
public:
    TCDlgPdu_AC(ASTag::TagClass tag_class,  uint16_t tag_val)
        : ASTypeAC(tag_class, tag_val)
    { }

    virtual const EncodedOID * ACDefined(void) const = 0;
    virtual TCUserInformation * UsrInfo(void) = 0;
};

//Structured TC Dialogue request APDU (AARQ)
struct TCReqPDU : public TCDlgPdu_AC {
    EncodedOID          ac_id;
    TCUserInformation   usrInfo;    //optional

    TCReqPDU() : TCDlgPdu_AC(ASTag::tagApplication, 0);
    { }
    ~TCReqPDU()
    { }

    // -- TCDlgPdu_AC interface methods
    const EncodedOID * ACDefined(void) const { return ac_id.length() ? &ac_id : 0; } 
    TCUserInformation * UsrInfo(void) { return usrInfo.extVals.empty() ? 0 : &usrInfo; }
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
struct TCRespPDU : public TCDlgPdu_AC {
    enum AssociateResult {
        dlgAccepted = 0, dlgRejectPermanent = 1
    };
    EncodedOID          ac_id;
    TCUserInformation   usrInfo;    //optional

    AssociateResult             result;
    AssociateSourceDiagnostic   diagnostic;

    TCRespPDU() : TCDlgPdu_AC(ASTag::tagApplication, 1);
    { }
    ~TCRespPDU()
    { }

    // -- TCDlgPdu_AC interface methods
    const EncodedOID * ACDefined(void) const { return ac_id.length() ? &ac_id : 0; } 
    TCUserInformation * UsrInfo(void) { return usrInfo.extVals.empty() ? 0 : &usrInfo; }
};

//Structured TC Dialogue abort APDU (ABRT)
class TCAbrtPDU : public TCDlgPdu_AC {
public:
    enum Source { dialogue_service_user = 0, dialogue_service_provider = 1 };

    Source              abrtSource;
    TCUserInformation   usrInfo;

    TCAbrtPDU() : TCDlgPdu_AC(ASTag::tagApplication, 4)
        , abrtSource(dialogue_service_provider)
    { }
    ~TCAbrtPDU()
    { }

    // -- TCDlgPdu_AC interface methods
    const EncodedOID * ACDefined(void) const { return 0; } 
    TCUserInformation * UsrInfo(void) { return usrInfo.extVals.empty() ? 0 : &usrInfo; }
};

} //tcap
} //eyelinecom

#endif /* __TC_STR_DIALOGUE_DEFS_HPP */

