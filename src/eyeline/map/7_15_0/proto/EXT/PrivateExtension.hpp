#ifndef __EYELINE_MAP_7F0_PROTO_EXT_PRIVATEEXTENSION_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_PRIVATEEXTENSION_HPP__

# include <sys/types.h>
# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace map {
namespace ext {

/*
  PrivateExtension ::= SEQUENCE {
    extId       MAP-EXTENSION.&extensionId ({ExtensionSet}),
    extType     MAP-EXTENSION.&ExtensionType ({ExtensionSet}{@extId}) OPTIONAL
  }
*/
class PrivateExtension {
public:
  PrivateExtension()
  : _extType(NULL)
  {}

  ~PrivateExtension() {
    if (_extType)
      _extType->~TransferSyntax();
  }

  asn1::TransferSyntax& initExtType() {
    _extType= new (_memAlloc_extType.buf) asn1::TransferSyntax();
    return *_extType;
  }

  const asn1::TransferSyntax* getExtType() const {
    return _extType;
  }

  asn1::TransferSyntax* getExtType() {
    return _extType;
  }

  asn1::EncodedOID _extId;

private:
  union {
    void * aligner;
    uint8_t buf[sizeof(asn1::TransferSyntax)];
  } _memAlloc_extType;

  asn1::TransferSyntax* _extType;
};

}}}

#endif
