#ifndef __EYELINE_TCAP_PROTO_ENC_PABORTCAUSE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_PABORTCAUSE_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/ASNTags.hpp"
# include "eyeline/tcap/TDialogueDefs.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class PAbortCause : public asn1::ber::EncoderOfINTEGER {
public:
  explicit PAbortCause(PAbort::Cause_e p_abort_cause,
                       asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER)
  : EncoderOfINTEGER(p_abort_cause, _typeTags, use_rule)
  {}

private:
  enum TypeTags_e {
    P_ABRT_CAUSE_Tag = 10
  };

  static const asn1::ASTagging _typeTags;
};

}}}}

#endif
