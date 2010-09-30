#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLESYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLESYSTEMFAILUREPARAM_HPP__

# include "util/Exception.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/common/AdditionalNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/ERR/FailureCauseParam.hpp"

# include "eyeline/map/7_15_0/proto/common/enc/MEAdditionalNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/enc/MENetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MEFailureCauseParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
ExtensibleSystemFailureParam ::= SEQUENCE {
        networkResource NetworkResource OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        additionalNetworkResource       [0] AdditionalNetworkResource   OPTIONAL,
        failureCauseParam       [1] FailureCauseParam   OPTIONAL }
*/

class MEExtensibleSystemFailureParam : public asn1::ber::EncoderOfSequence_T<5> {
public:
  explicit MEExtensibleSystemFailureParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<5>(use_rule),
    _networkResource(NULL), _extensionContainer(NULL), _encoderOfUExt(NULL),
    _additionalNetworkResource(NULL), _failureCauseParam(NULL)
  {}

  explicit MEExtensibleSystemFailureParam(const ExtensibleSystemFailureParam& value,
                                          asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<5>(use_rule),
    _networkResource(NULL), _extensionContainer(NULL), _encoderOfUExt(NULL),
    _additionalNetworkResource(NULL), _failureCauseParam(NULL)
  {
    setValue(value);
  }

  MEExtensibleSystemFailureParam(const asn1::ASTag& outer_tag,
                                 const asn1::ASTagging::Environment_e tag_env,
                                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<5>(outer_tag, tag_env, use_rule),
    _networkResource(NULL), _extensionContainer(NULL), _encoderOfUExt(NULL),
    _additionalNetworkResource(NULL), _failureCauseParam(NULL)
  {}

  ~MEExtensibleSystemFailureParam() {
    if (_networkResource)
      _networkResource->~MENetworkResource();
    if (_extensionContainer)
      _extensionContainer->~MEExtensionContainer();
    if (_encoderOfUExt)
      _encoderOfUExt->~EncoderOfUExtension_T();
    if (_additionalNetworkResource)
      _additionalNetworkResource->~MEAdditionalNetworkResource();
    if (_failureCauseParam)
      _failureCauseParam->~MEFailureCauseParam();
  }

  void setValue(const ExtensibleSystemFailureParam& value) {
    uint16_t idx=0;
    const common::NetworkResource* networkResource= value.getNetworkResource();
    if (networkResource) {
      _networkResource= new (_memAlloc_NetworkResource.buf) common::enc::MENetworkResource(getTSRule());
      _networkResource->setValue(networkResource->_value);
      setField(idx++, *_networkResource);
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
    const common::AdditionalNetworkResource* additionalNetworkResource= value.getAdditionalNetworkResource();
    if (additionalNetworkResource) {
      _additionalNetworkResource= new (_memAlloc_NetworkResource.buf) common::enc::MEAdditionalNetworkResource(_tag_additionalNetworkResource, getTSRule());
      _additionalNetworkResource->setValue(additionalNetworkResource->_value);
      setField(idx++, *_additionalNetworkResource);
    }
    const FailureCauseParam* failureCause= value.getFailureCauseParam();
    if (failureCause) {
      _failureCauseParam= new (_memAlloc_FailureCauseParam.buf) MEFailureCauseParam(_tag_failureCauseParam, getTSRule());
      _failureCauseParam->setValue(failureCause->value);
      setField(idx++, *_failureCauseParam);
    }
  }

private:
  union {
    void* aligner;
    uint8_t buf[sizeof(common::enc::MENetworkResource)];
  } _memAlloc_NetworkResource;

  union {
    void* aligenr;
    uint8_t buf[sizeof(ext::enc::MEExtensionContainer)];
  } _memAlloc_ExtensionContainer;

  union {
    void* aligner;
    uint8_t buf[sizeof(common::enc::MEAdditionalNetworkResource)];
  } _memAlloc_AdditionalNetworkResource;

  union {
    void* aligner;
    uint8_t buf[sizeof(asn1::ber::EncoderOfUExtension_T<1>)];
  } _memAlloc_EncoderOfUExtension;

  union {
    void* aligner;
    uint8_t buf[sizeof(MEFailureCauseParam)];
  } _memAlloc_FailureCauseParam;

  static const asn1::ASTagging _tag_additionalNetworkResource;
  static const asn1::ASTagging _tag_failureCauseParam;

  common::enc::MENetworkResource* _networkResource;
  ext::enc::MEExtensionContainer* _extensionContainer;
  asn1::ber::EncoderOfUExtension_T<1>* _encoderOfUExt;
  common::enc::MEAdditionalNetworkResource* _additionalNetworkResource;
  MEFailureCauseParam* _failureCauseParam;
};

}}}}

#endif
