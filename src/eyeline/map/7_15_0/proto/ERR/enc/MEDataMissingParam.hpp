#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEDATAMISSINGPARAM_HPP__
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEDATAMISSINGPARAM_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/ERR/DataMissingParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
DataMissingParam ::= SEQUENCE {
  extensionContainer      ExtensionContainer      OPTIONAL,
  ...
}
*/
class MEDataMissingParam : public asn1::ber::EncoderOfSequence_T<2> {
public:
  explicit MEDataMissingParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(use_rule),
    _extensionContainer(NULL)
  {}

  explicit MEDataMissingParam(const DataMissingParam& value,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(use_rule),
    _extensionContainer(NULL)
  {
    setValue(value);
  }

  MEDataMissingParam(const asn1::ASTag& outer_tag,
                     const asn1::ASTagging::Environment_e tag_env,
                     asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<2>(outer_tag, tag_env, use_rule),
    _extensionContainer(NULL)
  {}

  ~MEDataMissingParam() {
    if (_extensionContainer)
      _extensionContainer->~MEExtensionContainer();
//    if (_encoderOfUExt)
//      _encoderOfUExt->~EncoderOfUExtension();
  }

  void setValue(const DataMissingParam& value) {
    const ext::ExtensionContainer* extContainer= value.getExtensionContainer();
    if (extContainer) {
      _extensionContainer= new (_memAlloc_ExtensionContainer.buf) ext::enc::MEExtensionContainer(*extContainer, getTSRule());
      setField(0, *_extensionContainer);
    }
//    if ( !value._unkExt._tsList.empty() ) {
//      asn1::ber::EncoderOfUExtension* encoderOfUExt= new asn1::ber::EncoderOfUExtension();
//      encoderOfUExt->setValue(value._unkExt);
//    }
  }

private:
  union {
    void* aligenr;
    uint8_t buf[sizeof(ext::enc::MEExtensionContainer)];
  } _memAlloc_ExtensionContainer;

  ext::enc::MEExtensionContainer* _extensionContainer;
  //asn1::ber::EncoderOfUExtension* _encoderOfUExt;
};

}}}}

#endif
