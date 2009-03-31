/* ************************************************************************* *
 * Universal ASN.1 Object Identifier (OID or number).
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_UID_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_UID_DEFS__

#include "eyeline/asn1/EncodedOID.hpp"

namespace eyeline {
namespace asn1 {

class UniversalID {
public:
    enum UIDKind { uidLocal = 0, uidGlobal };

private:
    UIDKind     refKind;
    uint32_t    numRef;
    EncodedOID  oidRef;
    
public:
    UniversalID(uint32_t use_ref = 0)
        : refKind(uidLocal), numRef(use_ref)
    { }
    UniversalID(const EncodedOID & use_ref)
        : refKind(uidGlobal), numRef(0), oidRef(use_ref)
    { }

    void setLocal(uint32_t use_ref) { refKind = uidLocal; numRef = use_ref; }
    void setGlobal(const EncodedOID & use_ref) { refKind = uidGlobal; oidRef = use_ref; }

    UIDKind uidKind(void) const { return refKind; }
    const EncodedOID & Global(void) const { return oidRef; }
    uint32_t Local(void) const { return numRef; }
};

}; //asn1
}; //eyeline

#endif /* __ABSTRACT_SYNTAX_UID_DEFS__ */

