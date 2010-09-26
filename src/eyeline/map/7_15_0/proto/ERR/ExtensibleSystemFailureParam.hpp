#ifndef __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLESYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLESYSTEMFAILUREPARAM_HPP__

# include "eyeline/asn1/UnknownExtensions.hpp"
# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/AdditionalNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/FailureCauseParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/ERR/FailureCauseParam.hpp"

namespace eyeline {
namespace map {
namespace err {

/*
  ExtensibleSystemFailureParam ::= SEQUENCE {
        networkResource NetworkResource OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        additionalNetworkResource       [0] AdditionalNetworkResource   OPTIONAL,
        failureCauseParam       [1] FailureCauseParam   OPTIONAL }
 */
class ExtensibleSystemFailureParam {
public:
  ExtensibleSystemFailureParam()
  : _networkResource(NULL), _extensionContainer(NULL),
    _additionalNetworkResource(NULL), _failureCauseParam(NULL)
  {}

  ~ExtensibleSystemFailureParam() {
    if (_networkResource)
      _networkResource->~NetworkResource();
    if (_extensionContainer)
      _extensionContainer->~ExtensionContainer();
    if (_additionalNetworkResource)
      _additionalNetworkResource->~AdditionalNetworkResource();
    if (_failureCauseParam)
      _failureCauseParam->~FailureCauseParam();
  }

  common::NetworkResource& initNetworkResource() {
    _networkResource= new (_memAlloc_networkResource.buf) common::NetworkResource();
    return *_networkResource;
  }
  ext::ExtensionContainer& initExtensionContainer() {
    _extensionContainer= new (_memAlloc_extensionContainer.buf) ext::ExtensionContainer();
    return *_extensionContainer;
  }
  common::AdditionalNetworkResource& initAdditionalNetworkResource() {
    _additionalNetworkResource= new (_memAlloc_AdditionalNetworkResource.buf) common::AdditionalNetworkResource();
    return *_additionalNetworkResource;
  }
  FailureCauseParam& initFailureCauseParam() {
    _failureCauseParam= new (_memAlloc_FailureCauseParam.buf) FailureCauseParam();
    return *_failureCauseParam;
  }

  const common::NetworkResource* getNetworkResource() const {
    return _networkResource;
  }
  const ext::ExtensionContainer* getExtensionContainer() const {
    return _extensionContainer;
  }
  const common::AdditionalNetworkResource* getAdditionalNetworkResource() const {
    return _additionalNetworkResource;
  }
  const FailureCauseParam* getFailureCauseParam() const {
    return _failureCauseParam;
  }

  common::NetworkResource* getNetworkResource() {
    return _networkResource;
  }
  ext::ExtensionContainer* getExtensionContainer() {
    return _extensionContainer;
  }
  common::AdditionalNetworkResource* getAdditionalNetworkResource() {
    return _additionalNetworkResource;
  }
  FailureCauseParam* getFailureCauseParam() {
    return _failureCauseParam;
  }

private:
  union {
    void * aligner;
    uint8_t buf[sizeof(common::NetworkResource)];
  } _memAlloc_networkResource;

  union {
    void * aligner;
    uint8_t buf[sizeof(ext::ExtensionContainer)];
  } _memAlloc_extensionContainer;

  common::NetworkResource* _networkResource;
  ext::ExtensionContainer* _extensionContainer;

public:
  asn1::UnknownExtensions _unkExt;

private:
  union {
    void * aligner;
    uint8_t buf[sizeof(common::AdditionalNetworkResource)];
  } _memAlloc_AdditionalNetworkResource;

  union {
    void * aligner;
    uint8_t buf[sizeof(FailureCauseParam)];
  } _memAlloc_FailureCauseParam;

  common::AdditionalNetworkResource* _additionalNetworkResource;
  FailureCauseParam* _failureCauseParam;
};

}}}

#endif
