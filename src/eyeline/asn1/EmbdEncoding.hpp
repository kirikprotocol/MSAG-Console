/* ************************************************************************* *
 * EmbeddedEncoding helper type: used in EXTERNAL/EMBEDDED PDV definitions.
 * ************************************************************************* */
#ifndef __ASN1_EMBEDDED_ENCODING_DEFS
#ident "@(#)$Id$"
#define __ASN1_EMBEDDED_ENCODING_DEFS

#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BITSTR.hpp"
#include "eyeline/asn1/OCTSTR.hpp"

#include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace asn1 {

//According to X.690 cl.8.18.1,
//EmbeddedEncoding type is defined in EXPLICIT tagging environment as follow:
//
//  EmbeddedEncoding ::= CHOICE {
//    single-ASN1-type  [0] ABSTRACT-SYNTAX.&Type,
//    octet-aligned     [1] IMPLICIT OCTET STRING,
//    arbitrary         [2] IMPLICIT BIT STRING
//  }
class EmbeddedEncoding  : public util::ChoiceOf3_T<TransferSyntax, OCTSTR, BITSTR> {
public:
  enum Kind_e {
    evASType = 0,     //encoding of value of defined single ASN.1 type
    evOctAligned = 1, //arbitrary octet aligned encoding
    evBitAligned = 2  //arbitrary bit aligned encoding
  };

  EmbeddedEncoding() : util::ChoiceOf3_T<TransferSyntax, OCTSTR, BITSTR>()
  { }
  ~EmbeddedEncoding()
  { }

  Kind_e getKind(void) const { return static_cast<Kind_e>(getChoiceIdx()); }

  bool isTS(void)   const { return (getKind() == evASType); }
  bool isOCTS(void) const { return (getKind() == evOctAligned); }
  bool isBITS(void) const { return (getKind() == evBitAligned); }


  Alternative_T<TransferSyntax> TS()    { return alternative0(); }
  Alternative_T<OCTSTR>         OCTS()  { return alternative1(); }
  Alternative_T<BITSTR>         BITS()  { return alternative2(); }

  ConstAlternative_T<TransferSyntax> TS()   const { return alternative0(); }
  ConstAlternative_T<OCTSTR>         OCTS() const { return alternative1(); }
  ConstAlternative_T<BITSTR>         BITS() const { return alternative2(); }
};

} //asn1
} //eyeline

#endif /* __ASN1_EMBEDDED_ENCODING_DEFS */

