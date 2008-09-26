/* ************************************************************************** *
 * Classes implementing TCAP structured dialog messages.
 * ************************************************************************** */
#ifndef __TC_MESSAGE_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_MESSAGE_DEFS_HPP

#include "eyeline/tcap/TCComponent.hpp"
#include "eyeline/util/MaxSizeof.hpp"

namespace eyelinecom {
namespace tcap {

using eyelinecom::asn1::AbstractSyntax;

extern EncodedOID _ac_tcap_strDialogue_as;

//Dialogue request
struct TCReqPDU : public ASTypeAC {
public:
    TC_AARQ_PDU() : ASTypeAC(ASTag::tagApplication, 0);
    { }
    ~TC_AARQ_PDU()
    { }
    //TODO:
};

//Dialogue response
struct TCRespPDU : public ASTypeAC {
public:
    TC_AARQ_PDU() : ASTypeAC(ASTag::tagApplication, 1);
    { }
    ~TC_AARQ_PDU()
    { }
    //TODO:
};

//Dialogue abort
struct TCAbrtPDU : public ASTypeAC {
public:
    TC_AARQ_PDU() : ASTypeAC(ASTag::tagApplication, 4);
    { }
    TC_AARQ_PDU()
    { }
    //TODO:
};

//TCAP structured dialogue PDU, one of three alternatives: REQ, RESP, ABRT
class TCDialoguePDU : public AbstractSyntax {    //dialogue-as
public:
    enum PDUKind { pduNone = -1, pduAARQ = 0, pduAARE = 1, pduABRT = 4 };

private:
    uint8_t pduMem[eyelinecom::util::MaxSizeOf3_T<TCReqPDU, TCRespPDU, TCAbrtPDU>::VALUE];
    PDUKind kind;
    union {
        ASTypeAC *  ac;     //PDU's base class
        TCReqPDU *  req;    //[APPLICATION 0]
        TCRespPDU * resp;   //[APPLICATION 1]
        TCAbrtPDU * abrt;   //[APPLICATION 4]
    } pdu;

public:
    DialoguePDU(PDUKind use_kind = pduNone)
        : AbstractSyntax(_ac_tcap_strDialogue_as)
    {
        pdu.ac = 0;
        init(use_kind);
    }
    ~DialoguePDU()
    { }

    ASTypeAC * init(PDUKind use_kind)
    {
        if (pdu.ac) {
            pdu.ac->~ASTypeAC();
            pdu.ac = 0;
        }
        switch (kind = use_kind) {
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

    inline void Reset(void) { init(pduNone); }

    inline PDUKind Kind(void) const { return kind; }
    inline ASTypeAC * get(void) const { return pdu.ac; }

    inline TCReqPDU * Req(void) const { return kind == pduAARQ ? pdu.req : 0; }
    inline TCRespPDU * Resp(void) const { return kind == pduAARE ? pdu.resp : 0; }
    inline TCAbrtPDU * Abrt(void) const { return kind == pduABRT ? pdu.abrt : 0; }
};

//TCAP structured dialogue message
class TCAPMessage {
    enum MSGKind {
        t_none = -1
        //dialogue-as messages:
        , t_begin = 2           //[APPLICATION 2]
        , t_end = 4             //[APPLICATION 4]
        , t_continue = 5        //[APPLICATION 5]
        , t_abort = 7           //[APPLICATION 7]
    };

    MSGKind         kind;
    TransactionId   trId;
    TCDialoguePDU   dlgPart;
    std::list<ROSComponentAC*> compPart;

    TCAPMessage(MSGKind use_kind = t_none/*...*/)
        : kind(use_kind)
    { }
    ~TCAPMessage()
    { }
    //TODO:
};

}; //tcap
}; //eyelinecom

#endif /* __TC_MESSAGE_DEFS_HPP */

