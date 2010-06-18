/* ************************************************************************* *
 * Class(es) implementing ASN.1 1997 EXTERNAL type
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_EXTERNAL_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_EXTERNAL_DEFS__

#include <string>

#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/asn1/EmbdEncoding.hpp"

namespace eyeline {
namespace asn1 {

class DataIdentification {
  enum IdMask_e {
    maskAS = 0x01, maskPC = 0x02, maskTS = 0x04
  };
  enum Kind_e {
      idFixed = 0     //Data is the value of a fixed ASN.1 type
                      //(and hence known to both sender and receiver)
    , idASyntax = maskAS    //Data is identified by abstract syntaz with
                      //predefined transfer syntax
    , idPrsContext = maskPC //The negotiated OSI presentation context identifies
                      //the abstract and transfer syntaxes
    , idTSyntax = maskTS    //The data is the value of a fixed ASN.1 type
                      //(and hence known to both sender and receiver)
                      //represented according to given transfer syntax
    , idSyntaxes  = (maskAS | maskTS)  //Data is identified by both abstract
                      //syntax and transfer syntax
    , idCtxNegotiation = (maskPC | maskTS) //Data is identified by abstract 
                      //syntax, predefined for given presentation context,
                      //and transfer syntax
  };

protected:
  Kind_e    _kind;

public:
  DataIdentification() : _kind(idFixed)
  { }

  Kind_e getKind(void) const { return _kind; }

  //returns true if data identification uses AbstractSyntax OID
  bool hasASyntaxOID(void) const { return (_kind & (unsigned)maskAS) != 0; }
  //returns true if data identification uses TransferSyntax OID
  bool hasTSyntaxOID(void) const { return (_kind & (unsigned)maskTS) != 0; }
  //returns true if data identification uses presentation context Id
  bool hasPrsContextId(void) const { return (_kind & (unsigned)maskPC) != 0; }

  void setASyntaxOID(void)
  {
     _kind = static_cast<Kind_e>((unsigned)_kind | maskAS);
  }
  void setTSyntaxOID(void)
  {
     _kind = static_cast<Kind_e>((unsigned)_kind | maskTS);
  }
  void setPrsContext(void)
  {
     _kind = static_cast<Kind_e>((unsigned)_kind | maskPC);
  }

  void identifyByFixed(void)          { _kind = idFixed; }
  void identifyByASyntax(void)        { _kind = idASyntax; }
  void identifyByPrsContext(void)     { _kind = idPrsContext; }
  void identifyByTSyntax(void)        { _kind = idTSyntax; }
  void identifyBySyntaxes(void)       { _kind = idSyntaxes; }
  void identifyByCtxNegotiation(void) { _kind = idCtxNegotiation; }

  void resetIdentification(void) { _kind = idFixed; }
};


/* -------------------------------------------------------------------------- *
 * Generic associated type for all ASN types dealing with externally defined
 * data values (Empbedded Presentation Data Value, X.680 cl.33)
 * -------------------------------------------------------------------------- */
class EmbeddedPDV {
public:
  DataIdentification  _ident;
  EmbeddedEncoding    _enc;
  EncodedOID          _asOid;
  EncodedOID          _tsOid;
  int32_t             _prsCtxId;

  EmbeddedPDV() : _prsCtxId(0)
  { }
  ~EmbeddedPDV()
  { }

  void identifyByFixed(void)
  {
    _ident.resetIdentification();
    _asOid.reset();
    _tsOid.reset();
    _prsCtxId = 0;
  }

  void identifyByASyntax(const EncodedOID & as_oid)
  {
    _ident.identifyByASyntax();
    _asOid = as_oid;
    _tsOid.reset();
    _prsCtxId = 0;
  }
  void identifyByPrsContext(int32_t prs_ctx_id)
  {
    _ident.identifyByPrsContext();
    _asOid.reset();
    _tsOid.reset();
    _prsCtxId = prs_ctx_id;
  }
  void identifyByTSyntax(const EncodedOID & ts_oid)
  {
    _ident.identifyByTSyntax();
    _asOid.reset();
    _tsOid = ts_oid;
    _prsCtxId = 0;
  }
  void identifyBySyntaxes(const EncodedOID & as_oid, const EncodedOID & ts_oid)
  {
    _ident.identifyBySyntaxes();
    _asOid = as_oid;
    _prsCtxId = 0;
    _tsOid = ts_oid;
  }
  void identifyByCtxNegotiation(int32_t prs_ctx_id, const EncodedOID & ts_oid)
  {
    _ident.identifyByCtxNegotiation();
    _asOid.reset();
    _tsOid = ts_oid;
    _prsCtxId = prs_ctx_id; 
  }
};

/* -------------------------------------------------------------------------- *
 * ASN.1 type EXTERNAL - deals with types defined by some AbstractSyntax
 * (X.680 cl.34).
 * -------------------------------------------------------------------------- */
class ASExternal {
protected:
  //NOTE: idFixed & TransferSyntaxOID can't be used as value identification!!!
  DataIdentification  _ident;

public:
  EmbeddedEncoding    _enc;
  EncodedOID          _asOid;
  int32_t             _prsCtxId;
  std::string         _descr;

  ASExternal() : _prsCtxId(0)
  { }
  ~ASExternal()
  { }

  void reset(void)
  {
    _ident.identifyByFixed();
    _enc.reset();
    _asOid.reset();
    _prsCtxId = 0;
    _descr.clear();
  }

  const DataIdentification & getIdentification(void) const { return _ident; }

  //returns true if data identification uses AbstractSyntax OID
  bool hasASyntaxOID(void) const { return _ident.hasASyntaxOID(); }
  //returns true if data identification uses presentation context Id
  bool hasPrsContextId(void) const { return _ident.hasPrsContextId(); }

  void identifyByASyntax(void)      { _ident.identifyByASyntax(); }
  void identifyByASyntax(const EncodedOID & as_oid)
  {
    _ident.identifyByASyntax();
    _asOid = as_oid;
    _prsCtxId = 0;
  }

  void identifyByPrsContext(void)   { _ident.identifyByPrsContext(); }
  void identifyByPrsContext(int32_t prs_ctx_id)
  {
    _ident.identifyByPrsContext();
    _asOid.reset();
    _prsCtxId = prs_ctx_id;
  }

  void setASyntaxOID(void) { _ident.setASyntaxOID(); }
  void setASyntaxOID(const EncodedOID & as_oid)
  {
    _ident.setASyntaxOID();
    _asOid = as_oid;
  }

  void setPrsContext(void) { _ident.setPrsContext(); }
  void setPrsContext(int32_t prs_ctx_id)
  {
    _ident.setPrsContext();
    _prsCtxId = prs_ctx_id;
  }

};

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_EXTERNAL_DEFS__ */

