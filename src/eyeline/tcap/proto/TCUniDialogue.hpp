/* ************************************************************************** *
 * Classes implementing TCAP UNIDialogue PDUs.
 * ************************************************************************** */
#ifndef __TC_DIALOGUE_UNI_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_DIALOGUE_UNI_DEFS_HPP

#include "eyeline/tcap/TCUserInfo.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

extern EncodedOID _ac_tcap_uniDialogue_as;


struct TCAudtPDU : public ASTypeAC {
public:
    TC_AARQ_PDU() : ASTypeAC(ASTag::tagApplication, 0)
    { }
    ~TC_AARQ_PDU()
    { }
    //TODO:
};

struct TCUNIDialoguePDU : public AbstractSyntax { //uniDialogue-as
    enum Kind { pduAUDT = 0} kind;
    TCAudtPDU pdu;

    DialoguePDU()
        : kind(pduAUDT), AbstractSyntax(_ac_tcap_uniDialogue_as)
    {
        asTags().addOption(*pdu.Tagging(), true);
    }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_DIALOGUE_UNI_DEFS_HPP */

