#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSION_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSION_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeEOID.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PrivateExtension.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

class MEPrivateExtension : public asn1::ber::EncoderOfSequence_T<2> {
public:
  explicit MEPrivateExtension(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(use_rule),
    _extId(use_rule), _extType(NULL)
  {}

  MEPrivateExtension(const PrivateExtension& private_ext, asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(use_rule),
    _extId(use_rule), _extType(NULL)
  {
    setValue(private_ext);
  }

  MEPrivateExtension(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
                     asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(use_tag, tag_env, use_rule)
  {}

  ~MEPrivateExtension() {
    if (_extType)
      _extType->~EncoderOfASType();
  }

  void setValue(const PrivateExtension& private_ext) {
    uint16_t idx=0;
    _extId.setValue(private_ext._extId);
    setField(idx++, _extId);
    const asn1::TransferSyntax* extType= private_ext.getExtType();
    if (extType) {
      _extType= new (_memAlloc_extType.buf) asn1::ber::EncoderOfASType(getTSRule());
      _extType->setValue(*extType);
      setField(idx, *_extType);
    }
  }

private:
  asn1::ber::EncoderOfEOID _extId;

  union {
    void * aligner;
    uint8_t buf[sizeof(asn1::ber::EncoderOfASType)];
  } _memAlloc_extType;

  asn1::ber::EncoderOfASType* _extType;
};

}}}}

#endif
