#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEUNEXPECTEDDATAPARAM_HPP__
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEUNEXPECTEDDATAPARAM_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/ERR/DataMissingParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
UnexpectedDataParam ::= SEQUENCE {
  extensionContainer      ExtensionContainer      OPTIONAL,
  ...
}
*/
class MEUnexpectedDataParam : public asn1::ber::EncoderOfSequence_T<2> {
public:
  explicit MEUnexpectedDataParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(use_rule),
    _extensionContainer(NULL)
  {}

  explicit MEUnexpectedDataParam(const DataMissingParam& value,
                                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(use_rule),
    _extensionContainer(NULL), _encoderOfUExt(NULL)
  {
    setValue(value);
  }

  MEUnexpectedDataParam(const asn1::ASTag& outer_tag,
                        const asn1::ASTagging::Environment_e tag_env,
                        asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(outer_tag, tag_env, use_rule),
    _extensionContainer(NULL), _encoderOfUExt(NULL)
  {}

  ~MEUnexpectedDataParam() {
    if (_extensionContainer)
      _extensionContainer->~MEExtensionContainer();
    if (_encoderOfUExt)
      _encoderOfUExt->~EncoderOfUExtension_T();
  }

  void setValue(const DataMissingParam& value) {
    uint16_t idx=0;
    const ext::ExtensionContainer* extContainer= value.getExtensionContainer();
    if (extContainer) {
      _extensionContainer= new (_memAlloc_ExtensionContainer.buf) ext::enc::MEExtensionContainer(*extContainer, getTSRule());
      setField(idx++, *_extensionContainer);
    }
    if ( !value._unkExt._tsList.empty() ) {
      asn1::ber::EncoderOfUExtension_T<1>* encoderOfUExt= new (_memAlloc_EncoderOfUExtension.buf) asn1::ber::EncoderOfUExtension_T<1>();
      encoderOfUExt->setValue(value._unkExt, *this, idx);
    }
  }

private:
  union {
    void* aligenr;
    uint8_t buf[sizeof(ext::enc::MEExtensionContainer)];
  } _memAlloc_ExtensionContainer;

  union {
    void* aligner;
    uint8_t buf[sizeof(asn1::ber::EncoderOfUExtension_T<1>)];
  } _memAlloc_EncoderOfUExtension;

  ext::enc::MEExtensionContainer* _extensionContainer;
  asn1::ber::EncoderOfUExtension_T<1>* _encoderOfUExt;
};

}}}}

#endif
