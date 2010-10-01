#ifndef __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLECALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLECALLBARREDPARAM_HPP__

#include "eyeline/util/OptionalObjT.hpp"
#include "eyeline/asn1/UnknownExtensions.hpp"

# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {

/*  Type is defined in IMPLICIT tagging environment as follow:
  ExtensibleCallBarredParam ::= SEQUENCE {
        callBarringCause        CallBarringCause        OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ... ,
        unauthorisedMessageOriginator   [1] NULL                OPTIONAL
} */
class ExtensibleCallBarredParam {
public:
  ExtensibleCallBarredParam()
  : _callBarringCause(NULL), _extensionContainer(NULL),
    _unauthorisedMessageOriginator(NULL)
  {}

  ~ExtensibleCallBarredParam() {
    if (_callBarringCause)
      _callBarringCause->~CallBarringCause();
    if (_extensionContainer)
      _extensionContainer->~ExtensionContainer();
  }

  CallBarringCause& initCallBarringCause() {
    _callBarringCause= new (_memAlloc_callBarringCause.buf) CallBarringCause();
    return *_callBarringCause;
  }

  ext::ExtensionContainer& initExtensionContainer() {
    _extensionContainer= new (_memAlloc_extensionContainer.buf) ext::ExtensionContainer();
    return *_extensionContainer;
  }

  uint8_t& initUnauthorisedMessageOriginator() {
    _unauthorisedMessageOriginator= new (_memAlloc_unauthorisedMessageOriginator.buf) uint8_t();
    return *_unauthorisedMessageOriginator;
  }

  const CallBarringCause* getCallBarringCause() const {
    return _callBarringCause;
  }
  const ext::ExtensionContainer* getExtensionContainer() const {
    return _extensionContainer;
  }
  const uint8_t* getUnauthorisedMessageOriginator() const {
    return _unauthorisedMessageOriginator;
  }

  CallBarringCause* getCallBarringCause() {
    return _callBarringCause;
  }
  ext::ExtensionContainer* getExtensionContainer() {
    return _extensionContainer;
  }
  uint8_t* getUnauthorisedMessageOriginator() {
    return _unauthorisedMessageOriginator;
  }

private:
  union {
    void * aligner;
    uint8_t buf[sizeof(CallBarringCause)];
  } _memAlloc_callBarringCause;

  union {
    void * aligner;
    uint8_t buf[sizeof(ext::ExtensionContainer)];
  } _memAlloc_extensionContainer;

  CallBarringCause* _callBarringCause;
  ext::ExtensionContainer* _extensionContainer;

public:
  asn1::UnknownExtensions   _unkExt;

private:
  union {
    void * aligner;
    uint8_t buf[sizeof(uint8_t)];
  } _memAlloc_unauthorisedMessageOriginator;

  uint8_t* _unauthorisedMessageOriginator;
};

}}}

#endif
