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

using eyeline::asn1::ber::TSGroupBER;

/* AbortCause is defined in IMPLICIT tagging environment as follow:
P-AbortCause ::= [APPLICATION 10]  INTEGER(0..127)
*/
class TDPAbortCause : public asn1::ber::DecoderOfINTEGER {
public:
  static const asn1::ASTag _typeTag; //[APPLICATION 10] IMPLICIT

  explicit TDPAbortCause(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfINTEGER(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                   TSGroupBER::getTSRule(use_rule))
  { }
  explicit TDPAbortCause(tcap::PAbort::Cause_t & use_cause,
                       TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfINTEGER(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                  TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::DecoderOfINTEGER::setValue(use_cause);
  }
  ~TDPAbortCause()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_PABORT_CAUSE_HPP */

