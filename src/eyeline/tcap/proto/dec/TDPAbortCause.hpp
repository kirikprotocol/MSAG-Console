/* ************************************************************************* *
 * TCAP Message Decoder: TC Provider Abort cause type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_PABORT_CAUSE_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_PABORT_CAUSE_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* AbortCause is defined in IMPLICIT tagging environment as follow:
P-AbortCause ::= [APPLICATION 10]  INTEGER(0..127)
*/
class TDPAbortCause : public asn1::ber::DecoderOfINTEGER {
public:
  static const asn1::ASTag _typeTag; //[APPLICATION 10] IMPLICIT

  explicit TDPAbortCause(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfINTEGER(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  { }
  explicit TDPAbortCause(tcap::PAbort::Cause_t & use_cause,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfINTEGER(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  {
    asn1::ber::DecoderOfINTEGER::setValue(use_cause);
  }
  ~TDPAbortCause()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_PABORT_CAUSE_HPP */

