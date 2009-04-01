/* ************************************************************************* *
 * Class(es) implementing ASN.1 ABSTRACT-SYNTAX class
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_DEFS__

#include <stdarg.h>
#include "eyeline/asn1/ASNTypes.hpp"
#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/util/MTRefPtr.hpp"

namespace eyeline {
namespace asn1 {

//AbstractSyntax : Abstract Type wirh unique OID assigned
class AbstractSyntax : public ASTypeAC {
public:
  const EncodedOID &  _asId; //associated ABSTRACT-SYNTAX.&id

  AbstractSyntax(const EncodedOID & use_asId)
    : _asId(use_asId), ASTypeAC()
  { }
  AbstractSyntax(const EncodedOID & use_asId, ASTag::TagClass tag_class, uint16_t tag_val)
    : _asId(use_asId), ASTypeAC(tag_class, tag_val)
  { }
  AbstractSyntax(const EncodedOID & use_asId, const ASTagging & use_tags)
    : _asId(use_asId), ASTypeAC(use_tags)
  { }

};

typedef eyeline::util::RFPtr_T<ASTypeAC> ASTypeRfp;
typedef eyeline::util::RFPtr_T<AbstractSyntax> AbstractSyntaxRfp;

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_DEFS__ */

