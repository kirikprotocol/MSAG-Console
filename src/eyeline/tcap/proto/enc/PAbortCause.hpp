#ifndef __EYELINE_TCAP_PROTO_ENC_PABORTCAUSE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_PABORTCAUSE_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class PAbortCause : public asn1::ber::EncoderOfINTEGER {
  enum {
    P_ABRT_CAUSE_Tag_Value = 10
  };

  static  asn1::ASTagging formFieldTags() {
    asn1::ASTagging pduTags(2, asn1::ASTag(asn1::ASTag::tagApplication, P_ABRT_CAUSE_Tag_Value),
                            asn1::_tagINTEGER);
    pduTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);

    return pduTags;
  }

public:
  explicit PAbortCause(PAbort::Cause_e p_abort_cause,
                         TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
  : EncoderOfINTEGER(p_abort_cause, formFieldTags())
  {
    setRule(use_rule);
  }
};

}}}}

#endif
