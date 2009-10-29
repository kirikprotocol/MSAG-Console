#ifndef __EYELINE_TCAP_PROTO_ENC_ABRTAPDU_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_ABRTAPDU_HPP__

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"
# include "eyeline/tcap/provd/SUARequests.hpp"
# include "eyeline/asn1/ASNTags.hpp"
# include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class ABRT_APdu : public asn1::ber::EncoderOfSequence {
public:
  enum AbrtSource_e {
    dlgServiceUser = 0, dlgServiceProvider = 1
  };

  enum {
    ABRT_Tag_Value = 4, ABRT_Source_Tag_Value = 0
  };

private:
  asn1::ber::EncoderOfINTEGER _abortSource;
  const TDlgUserInfo* _usrInfo;

  static  asn1::ASTagging formPduTags() {
    asn1::ASTagging pduTags(2, asn1::ASTag(asn1::ASTag::tagApplication, ABRT_Tag_Value),
                            asn1::_tagSEQOF);
    pduTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);

    return pduTags;
  }

  using EncoderOfSequence::addField;

public:
  explicit ABRT_APdu(AbrtSource_e abort_source,
                     TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : EncoderOfSequence(formPduTags(), use_rule),
      _abortSource(abort_source), _usrInfo(NULL)
  {
    asn1::ASTagging fieldTags(2, asn1::ASTag(asn1::ASTag::tagContextSpecific, ABRT_Source_Tag_Value),
                              asn1::_tagINTEGER);
    fieldTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);
    _abortSource = asn1::ber::EncoderOfINTEGER(abort_source, fieldTags);
    addField(&_abortSource);
  }

  void setUserInfo(const TDlgUserInfo* usr_info) {
    _usrInfo = usr_info;
  }
};

}}}}

#endif
