/* ************************************************************************* *
 * TCAP Message Encoder: TC Provider Abort cause type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_PABORT_CAUSE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_ENC_PABORT_CAUSE_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* AbortCause is defined in IMPLICIT tagging environment as follow:
P-AbortCause ::= [APPLICATION 10]  INTEGER(0..127)
*/
class TEPAbortCause : public asn1::ber::EncoderOfINTEGER {
public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 10] IMPLICIT

  explicit TEPAbortCause(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfINTEGER(_typeTags, use_rule)
  { }
  explicit TEPAbortCause(PAbort::Cause_t use_cause,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfINTEGER(_typeTags, use_rule)
  {
    setValue(use_cause);
  }
  ~TEPAbortCause()
  { }

  //Base class provides:
  //void setValue(PAbort::Cause_t use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_PABORT_CAUSE_HPP */

