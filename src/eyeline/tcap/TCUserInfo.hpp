/* ************************************************************************** *
 * Classes implementing TC messages user information.
 * ************************************************************************** */
#ifndef __TC_USRINFO_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_USRINFO_DEFS_HPP

#include "eyeline/asn1/UniversalID.hpp"
#include "eyeline/asn1/ASNTypes.hpp"

namespace eyelinecom {
namespace tcap {

using eyelinecom::asn1::ASTag;
using eyelinecom::asn1::ASTypeAC;
using eyelinecom::asn1::ASTypeTagging;
using eyelinecom::asn1::EncodedOID;
using eyelinecom::asn1::BITBuffer;

//ASN.1 1997 EXTERNAL type adopted for TC.
//keeps TC DialoguePDU or UniDialoguePDU in case of global(direct)
//type reference, any type in case of local(indirect) reference.
class TCExternal : public ASTypeAC {
private:
    union {
        ASTypeAC *          astype;   //set if typeRef is uidGlobal 
        const BITBuffer *   encoding; //set if typeref is uidLocal
    } value;
    UniversalID     typeRef;
    std::string     typeDescriptor; //OPTIONAL

public:
    TCExternal() : ASTypeAC(ASTag::tagUniversal, 8)
    {
        value.encoding = 0;
    }
    ~TCExternal()
    { }

    void setValueEncoding(uint32_t use_uid, const BITBuffer * use_enc)
    {
        typeRef.setLocal(use_uid);
        value.encoding = use_enc;
    }
    void setValueASyntax(const EncodedOID & use_uid, ASTypeAC * use_astyp)
    {
        typeRef.setGlobal(use_uid);
        value.astype = use_astyp;
    }
    inline void setValueDescriptor(const std::string use_descr) { typeDescriptor = use_descr; }

    inline const BITBuffer * ValueEncoding(void) const
    {
        return (typeRef.UIDKind() == UniversalID::uidLocal) ? value.encoding : 0;
    }
    inline ASTypeAC * ValueASyntax(void) const
    {
        return (typeRef.UIDKind() == UniversalID::uidGlobal) ? value.type : 0;
    }
    inline const std::string & ValueDescriptor(void) const { return typeDescriptor; }
};

class TCUserInformation : public ASTypeAC {
public:
    std::list<TCExternal> extVals;

    TCUserInformation() : ASTypeAC(ASTag::tagContextSpecific, 30)
    { }
};


}; //tcap
}; //eyelinecom

#endif /* __TC_USRINFO_DEFS_HPP */

