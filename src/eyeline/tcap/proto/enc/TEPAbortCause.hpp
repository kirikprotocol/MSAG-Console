/* ************************************************************************* *
 * TCAP Message Encoder: TC Provider Abort cause type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_PABORT_CAUSE_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_PABORT_CAUSE_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* AbortCause is defined in IMPLICIT tagging environment as follow:
P-AbortCause ::= [APPLICATION 10]  INTEGER(0..127)
*/
class TEPAbortCause : public asn1::ber::EncoderOfINTEGER {
public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 10] IMPLICIT

  explicit TEPAbortCause(PAbort::Cause_t use_cause = tcap::PAbort::p_resourceLimitation,
                       TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfINTEGER(_typeTags, TSGroupBER::getTSRule(use_rule))
  {
    setValue(use_cause);
  }
  ~TEPAbortCause()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_PABORT_CAUSE_HPP */

