#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DATAMISSINGPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_DATAMISSINGPARAM_HPP__

# include <inttypes.h>
# include "eyeline/asn1/UnknownExtensions.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {

/*
 DataMissingParam ::= SEQUENCE {
   extensionContainer      ExtensionContainer      OPTIONAL,
   ...
 }
 */
class DataMissingParam {
public:
  DataMissingParam()
  : _extensionContainer(NULL)
  {}

  ~DataMissingParam() {
    if (_extensionContainer)
      _extensionContainer->~ExtensionContainer();
  }

  ext::ExtensionContainer& initExtensionContainer() {
    _extensionContainer= new (_memAlloc_ExtensionContainer.buf) ext::ExtensionContainer();
    return *_extensionContainer;
  }

  const ext::ExtensionContainer* getExtensionContainer() const {
    return _extensionContainer;
  }

  ext::ExtensionContainer* getExtensionContainer() {
    return _extensionContainer;
  }

private:
  union {
    void * aligner;
    uint8_t buf[sizeof(ext::ExtensionContainer)];
  } _memAlloc_ExtensionContainer;

  ext::ExtensionContainer* _extensionContainer;

public:
  asn1::UnknownExtensions _unkExt;
};

}}}

#endif
