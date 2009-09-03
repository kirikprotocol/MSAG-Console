/* ************************************************************************* *
 * Class(es) implementing ASN.1 1997 EXTERNAL type
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_EXTERNAL_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_EXTERNAL_DEFS__

#include "eyeline/asn1/AbstractSyntax.hpp"
#include "eyeline/util/MaxSizeof.hpp"

namespace eyeline {
namespace asn1 {

//EXTERNAL type value
class ASExternalValue {
public:
  enum ValueKind_e { valNone = 0, valAS, valEnc };

  //encoding of indirectly referenced type
  struct IRefEncoding {
    uint32_t  typeId;
    BITBuffer typeEnc;

    IRefEncoding() : typeId(0)
    { }
    ~IRefEncoding()
    { }
  };

private:
  uint8_t objMem[eyeline::util::MaxSizeOf2_T<AbstractSyntaxRfp, IRefEncoding>::VALUE];

protected:
  ValueKind_e _kind;
  std::string _descr;
  union {
    void              * ptr;
    AbstractSyntaxRfp * pAs;  //direct typeRef (OID)
    IRefEncoding      * pEnc; //indirect typeref (some integer ID)
  } _pVal;

  void resetDescr(const char * use_descr = NULL)
  {
    if (use_descr)
      _descr = use_descr;
    else
      _descr.clear();
  }
  void resetObj(ValueKind_e use_kind = valNone)
  {
    if (_pVal.ptr ) {
      if (_kind == valAS)
        _pVal.pAs->~AbstractSyntaxRfp();
      else if (_kind == valEnc)
        _pVal.pEnc->~IRefEncoding();
      _pVal.ptr = 0;
    }
    if ((_kind = use_kind) == valAS)
      _pVal.pAs = new(objMem)AbstractSyntaxRfp();
    else if (_kind == valEnc)
      _pVal.pEnc = new(objMem)IRefEncoding();
  }

public:
  ASExternalValue() : _kind(valNone)
  {
    _pVal.ptr = 0;
  }
  ASExternalValue(const ASExternalValue & use_ext)
    : _kind(valNone)
  {
    _pVal.ptr = 0;
    if (use_ext._kind == valEnc)
      setEncoding(*(use_ext._pVal.pEnc), use_ext._descr.c_str());
    else if (use_ext._kind == valAS)
      setASyntax(*(use_ext._pVal.pAs), use_ext._descr.c_str());
    else
      resetObj(valNone);
  }

  ASExternalValue(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
    : _kind(valNone)
  {
    _pVal.ptr = 0;
    setASyntax(use_astyp, use_descr);
  }
  ASExternalValue(uint32_t use_irId, const BITBuffer & use_enc, const char * use_descr = NULL)
    :  _kind(valNone)
  {
    _pVal.ptr = 0;
    setEncoding(use_irId, use_enc, use_descr);
  }

  ~ASExternalValue()
  {
    resetObj(valNone);
  }

  void setASyntax(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
  {
    resetObj(valAS);
    *_pVal.pAs = use_astyp;
    resetDescr(use_descr);
  }
  void setEncoding(uint32_t use_irId, const BITBuffer & use_enc, const char * use_descr = NULL)
  {
    resetObj(valEnc);
    _pVal.pEnc->typeId = use_irId;
    _pVal.pEnc->typeEnc = use_enc;
    resetDescr(use_descr);
  }
  void setEncoding(const IRefEncoding & use_enc, const char * use_descr = NULL)
  {
    resetObj(valEnc);
    *_pVal.pEnc = use_enc;
    resetDescr(use_descr);
  }

  ValueKind_e Kind(void) const { return _kind; }
  //
  const std::string & getDescriptor(void) const { return _descr; }
  //
  const AbstractSyntaxRfp * getASyntax(void) const
  {
    return (_kind == valAS) ? _pVal.pAs : 0;
  }
  //
  const IRefEncoding * getEncoding(void) const
  {
    return (_kind == valEnc) ? _pVal.pEnc : 0;
  }
};


//ASN.1 1997 EXTERNAL type implementation
class ASExternal : public ASExternalValue, public ASTypeAC {
public:
  ASExternal()
    : ASExternalValue(), ASTypeAC(ASTag::tagUniversal, 8)
  { }
  ASExternal(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
    : ASExternalValue(use_astyp, use_descr), ASTypeAC(ASTag::tagUniversal, 8)
  { }
  ASExternal(uint32_t use_uid, const BITBuffer & use_enc, const char * use_descr = NULL)
    : ASExternalValue(use_uid, use_enc, use_descr), ASTypeAC(ASTag::tagUniversal, 8)
  { }
  ASExternal(const ASExternalValue & use_val)
    : ASExternalValue(use_val), ASTypeAC(ASTag::tagUniversal, 8)
  { }
  virtual ~ASExternal()
  { }
};


} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_EXTERNAL_DEFS__ */

