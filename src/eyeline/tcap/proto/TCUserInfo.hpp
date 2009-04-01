/* ************************************************************************** *
 * Classes implementing TC messages user information.
 * ************************************************************************** */
#ifndef __TC_USRINFO_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_USRINFO_DEFS_HPP

#include "eyeline/asn1/UniversalID.hpp"
#include "eyeline/asn1/AbstractSyntax.hpp"


namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::asn1::ASTag;
using eyeline::asn1::ASTypeAC;
using eyeline::asn1::ASTypeRfp;
using eyeline::asn1::AbstractSyntax;
using eyeline::asn1::AbstractSyntaxRfp;

using eyeline::asn1::EncodedOID;
using eyeline::asn1::BITBuffer;



//ASN.1 1997 EXTERNAL type adopted for TC.
//keeps TC DialoguePDU or UniDialoguePDU in case of global(direct)
//type reference, any type in case of local(indirect) reference.
class TCExternal : public ASTypeAC {
private:
    struct {
        AbstractSyntaxRfp   astype;   //set if typeRef is uidGlobal 
        BITBuffer           encoding; //set if typeref is uidLocal
    } value;
    UniversalID     typeRef;
    std::string     typeDescriptor; //OPTIONAL

public:
    TCExternal()
        : ASTypeAC(ASTag::tagUniversal, 8)
    { }
    ~TCExternal()
    { }

    void setValueEncoding(uint32_t use_uid, const BITBuffer & use_enc)
    {
        typeRef.setLocal(use_uid);
        value.encoding = use_enc;
    }
    void setValueASyntax(const AbstractSyntaxRfp & use_astyp)
    {
        typeRef.setGlobal(use_astyp->_asId);
        value.astype = use_astyp;
    }
    void setValueDescriptor(const std::string use_descr) { typeDescriptor = use_descr; }

    UniversalID::UIDKind ValueKind(void) const
    {
        return typeRef.uidKind();
    }

    const BITBuffer * ValueEncoding(void) const
    {
        return (typeRef.uidKind() == UniversalID::uidLocal) ? &value.encoding : 0;
    }
    const AbstractSyntaxRfp & ValueASyntax(void) const
    {
        return value.astype;
    }
    const std::string & ValueDescriptor(void) const { return typeDescriptor; }
};

class TCUserInformation : public ASTypeAC {
public:
    std::list<TCExternal> extVals;

    TCUserInformation() : ASTypeAC(ASTag::tagContextSpecific, 30)
    { }

    TCExternal & addUIASyntax(const AbstractSyntaxRfp & use_astyp)
    {
        extVals.push_back(TCExternal());
        extVals.back().setValueASyntax(use_astyp);
        return extVals.back();
    }
    TCExternal & addUIEncoding(uint32_t use_uid, const BITBuffer & use_enc)
    {
        extVals.push_back(TCExternal());
        extVals.back().setValueEncoding(use_uid, use_enc);
        return extVals.back();
    }
};


} //proto
} //tcap
} //eyeline

#endif /* __TC_USRINFO_DEFS_HPP */

