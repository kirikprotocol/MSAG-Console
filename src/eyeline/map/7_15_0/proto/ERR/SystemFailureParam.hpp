#ifndef __EYELINE_MAP_7F0_PROTO_ERR_SYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_SYSTEMFAILUREPARAM_HPP__

# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleSystemFailureParam.hpp"

namespace eyeline {
namespace map {
namespace err {

/*
  SystemFailureParam ::= CHOICE {
        networkResource NetworkResource,
        -- networkResource must not be used in version 3
        extensibleSystemFailureParam    ExtensibleSystemFailureParam
        -- extensibleSystemFailureParam must not be used in version <3
  }
*/
class SystemFailureParam {
public:
  enum Kind_e { KindNone, KindNetworkResource, KindExtensibleSystemFailureParam };

  SystemFailureParam()
  : _kind(KindNone)
  {
    _choice.networkResource= NULL;
  }

  ~SystemFailureParam() {
    clear();
  }

  common::NetworkResource& initNetworkResource() {
    clear();
    _kind= KindNetworkResource;
    _choice.networkResource= new (_memAlloc.buf) common::NetworkResource();
    return *_choice.networkResource;
  }

  ExtensibleSystemFailureParam& initExtensibleCallBarredParam() {
    clear();
    _kind= KindExtensibleSystemFailureParam;
    _choice.extensibleSystemFailureParam= new (_memAlloc.buf) ExtensibleSystemFailureParam();
    return *_choice.extensibleSystemFailureParam;
  }

  Kind_e getKind() const { return _kind; }

  const common::NetworkResource* getNetworkResource() const {
    return (_kind == KindNetworkResource) ? _choice.networkResource : NULL;
  }
  const ExtensibleSystemFailureParam* getExtensibleSystemFailureParam() const {
    return (_kind == KindExtensibleSystemFailureParam) ? _choice.extensibleSystemFailureParam : NULL;
  }

  common::NetworkResource* getNetworkResource() {
    return (_kind == KindNetworkResource) ? _choice.networkResource : NULL;
  }
  ExtensibleSystemFailureParam* getExtensibleSystemFailureParam() {
    return (_kind == KindExtensibleSystemFailureParam) ? _choice.extensibleSystemFailureParam : NULL;
  }

protected:
  void clear() {
    if (_kind == KindNetworkResource)
      _choice.networkResource->~NetworkResource();
    else if(_kind == KindExtensibleSystemFailureParam)
      _choice.extensibleSystemFailureParam->~ExtensibleSystemFailureParam();
    _kind= KindNone;
    _choice.networkResource= NULL;
  }

private:
  union {
    void * aligner;
    uint8_t buf[util::MaxSizeOf2_T<common::NetworkResource,
                                   ExtensibleSystemFailureParam>::VALUE];
  } _memAlloc;

  union {
    common::NetworkResource* networkResource;
    ExtensibleSystemFailureParam* extensibleSystemFailureParam;
  } _choice;

  Kind_e _kind;
};

}}}

#endif
