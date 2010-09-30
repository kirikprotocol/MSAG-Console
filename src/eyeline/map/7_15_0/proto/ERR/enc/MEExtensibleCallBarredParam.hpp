#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLECALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLECALLBARREDPARAM_HPP__

# include "util/Exception.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MECallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

# include "eyeline/util/MaxSizeof.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
 ExtensibleCallBarredParam ::= SEQUENCE {
        callBarringCause        CallBarringCause        OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ... ,
        unauthorisedMessageOriginator   [1] NULL                OPTIONAL }
*/
class MEExtensibleCallBarredParam : public asn1::ber::EncoderOfSequence_T<4> {
public:
  explicit MEExtensibleCallBarredParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<4>(use_rule),
    _callBarringCause(NULL), _extensionContainer(NULL),
    _encoderOfUExt(NULL), _unauthMsgOrig(NULL)
  {}

  explicit MEExtensibleCallBarredParam(const ExtensibleCallBarredParam& value,
                                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<4>(use_rule),
    _callBarringCause(NULL), _extensionContainer(NULL),
    _encoderOfUExt(NULL), _unauthMsgOrig(NULL )
  {
    setValue(value);
  }

  MEExtensibleCallBarredParam(const asn1::ASTag& outer_tag,
                              const asn1::ASTagging::Environment_e tag_env,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<4>(outer_tag, tag_env, use_rule),
    _callBarringCause(NULL), _extensionContainer(NULL),
    _encoderOfUExt(NULL), _unauthMsgOrig(NULL)
  {}

  ~MEExtensibleCallBarredParam() {
    if (_callBarringCause)
      _callBarringCause->~MECallBarringCause();
    if (_extensionContainer)
      _extensionContainer->~MEExtensionContainer();
    if (_encoderOfUExt)
      _encoderOfUExt->~EncoderOfUExtension_T();
    if (_unauthMsgOrig)
      _unauthMsgOrig->~EncoderOfNULL();
  }

  void setValue(const ExtensibleCallBarredParam& value) {
    const CallBarringCause* callBarringCause= value.getCallBarringCause();
    uint16_t idx= 0;
    if (callBarringCause) {
      _callBarringCause= new (_memAlloc_MECallBarrinCause.buf) MECallBarringCause(getTSRule());
      _callBarringCause->setValue(callBarringCause->value);
      setField(idx++, *_callBarringCause);
    }
    const ext::ExtensionContainer* extContainer= value.getExtensionContainer();
    if (extContainer) {
      _extensionContainer= new (_memAlloc_ExtensionContainer.buf) ext::enc::MEExtensionContainer(*extContainer, getTSRule());
      setField(idx++, *_extensionContainer);
    }
    if ( !value._unkExt._tsList.empty() ) {
      asn1::ber::EncoderOfUExtension_T<1>* encoderOfUExt= new (_memAlloc_EncoderOfUExtension.buf) asn1::ber::EncoderOfUExtension_T<1>();
      encoderOfUExt->setValue(value._unkExt, *this, idx);
    }
    const uint8_t* unauthMsgOrig= value.getUnauthorisedMessageOriginator();
    if (unauthMsgOrig) {
      _unauthMsgOrig= new (_memAlloc_UnauthMsgOrig.buf) asn1::ber::EncoderOfNULL(_tag_unauthMsgOrig, getTSRule());
      setField(idx, *_unauthMsgOrig);
    }
  }

private:
  union {
    void* aligner;
    uint8_t buf[sizeof(MECallBarringCause)];
  } _memAlloc_MECallBarrinCause;

  union {
    void* aligenr;
    uint8_t buf[sizeof(ext::enc::MEExtensionContainer)];
  } _memAlloc_ExtensionContainer;

  union {
    void* aligner;
    uint8_t buf[sizeof(asn1::ber::EncoderOfUExtension_T<1>)];
  } _memAlloc_EncoderOfUExtension;

  union {
    void* aligner;
    uint8_t buf[sizeof(asn1::ber::EncoderOfNULL)];
  } _memAlloc_UnauthMsgOrig;

  static const asn1::ASTagging _tag_unauthMsgOrig/*=asn1::ASTagging(asn1::ASTag::tagContextSpecific, 1,
                                                                  asn1::ASTagging::tagsIMPLICIT)*/;
  MECallBarringCause* _callBarringCause;
  ext::enc::MEExtensionContainer* _extensionContainer;
  asn1::ber::EncoderOfUExtension_T<1>* _encoderOfUExt;
  asn1::ber::EncoderOfNULL* _unauthMsgOrig;
};

}}}}

#endif
