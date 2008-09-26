/* ************************************************************************** *
 * Classes implementing TCAP UNIDialog messages.
 * ************************************************************************** */
#ifndef __TC_MESSAGE_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_MESSAGE_DEFS_HPP

#include "eyeline/tcap/TCComponent.hpp"

namespace eyelinecom {
namespace tcap {

using eyelinecom::asn1::AbstractSyntax;

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
    { }
};

}; //tcap
}; //eyelinecom

#endif /* __TC_MESSAGE_DEFS_HPP */

