#ifndef __EYELINE_MAP_7F0_PROTO_EXT_EXTENSIONCONTAINER_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_EXTENSIONCONTAINER_HPP__

# include "eyeline/map/7_15_0/proto/EXT/PrivateExtensionList.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PCS_Extensions.hpp"
# include "eyeline/asn1/UnknownExtensions.hpp"

namespace eyeline {
namespace map {
namespace ext {

/*
  ExtensionContainer ::= SEQUENCE {
    privateExtensionList    [0] PrivateExtensionList OPTIONAL,
    pcs-Extensions          [1] PCS-Extensions       OPTIONAL,
    ...
  }
  NOTE: the '...' in ASN.1 type is a mandatory field
*/
class ExtensionContainer {
public:
  ExtensionContainer()
  : _privateExtensionList(NULL), _pcs_Extensions(NULL)
  {}

  ~ExtensionContainer() {
    if (_privateExtensionList)
      _privateExtensionList->~PrivateExtensionList();
    if (_pcs_Extensions)
      _pcs_Extensions->~PCS_Extensions();
  }

  PrivateExtensionList& initPrivateExtensionList() {
    _privateExtensionList= new (_memAlloc_privateExtensionList.buf) PrivateExtensionList();
    return *_privateExtensionList;
  }

  PCS_Extensions& initPCS_Extensions() {
    _pcs_Extensions= new (_memAlloc_pcs_Extensions.buf) PCS_Extensions();
    return *_pcs_Extensions;
  }

  const PrivateExtensionList* getPrivateExtensionList() const {
    return _privateExtensionList;
  }
  const PCS_Extensions* getPCS_Extensions() const {
    return _pcs_Extensions;
  }

  PrivateExtensionList* getPrivateExtensionList() {
    return _privateExtensionList;
  }
  PCS_Extensions* getPCS_Extensions() {
    return _pcs_Extensions;
  }

private:
  union {
    void * aligner;
    uint8_t buf[sizeof(PrivateExtensionList)];
  } _memAlloc_privateExtensionList;

  union {
    void * aligner;
    uint8_t buf[sizeof(PCS_Extensions)];
  } _memAlloc_pcs_Extensions;

  PrivateExtensionList* _privateExtensionList;
  PCS_Extensions* _pcs_Extensions;

public:
  asn1::UnknownExtensions _unkExt;
};

}}}

#endif
