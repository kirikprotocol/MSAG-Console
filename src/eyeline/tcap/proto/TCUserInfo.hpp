/* ************************************************************************** *
 * Classes implementing TC messages user information.
 * ************************************************************************** */
#ifndef __TC_USRINFO_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_USRINFO_DEFS_HPP

#include "eyeline/asn1/UniversalID.hpp"
#include "eyeline/asn1/AbstractSyntax.hpp"
#include <list>

namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::asn1::ASTag;
using eyeline::asn1::ASTagging;
using eyeline::asn1::ASTypeAC;
using eyeline::asn1::ASTypeRfp;
using eyeline::asn1::AbstractSyntax;
using eyeline::asn1::AbstractSyntaxRfp;

using eyeline::asn1::EncodedOID;
using eyeline::asn1::UniversalID;
using eyeline::asn1::BITBuffer;


//ASN.1 1997 EXTERNAL type adopted for TC.
//keeps TC DialoguePDU or UniDialoguePDU in case of global(direct)
//type reference, any type in case of local(indirect) reference.
class TCExternal : public ASTypeAC {
private:
  struct {
    AbstractSyntaxRfp   astype;   //set if typeRef is uidGlobal 
    BITBuffer           encoding; //set if typeref is uidLocal
  } value;
  UniversalID     typeRef;
  std::string     typeDescriptor; //OPTIONAL

public:
  TCExternal()
    : ASTypeAC(ASTag::tagUniversal, 8)
  { }
  ~TCExternal()
  { }

  void setValueEncoding(uint32_t use_uid, const BITBuffer & use_enc)
  {
    typeRef.setLocal(use_uid);
    value.encoding = use_enc;
  }
  void setValueASyntax(const AbstractSyntaxRfp & use_astyp)
  {
    typeRef.setGlobal(use_astyp->_asId);
    value.astype = use_astyp;
  }
  void setValueDescriptor(const std::string use_descr) { typeDescriptor = use_descr; }

  UniversalID::UIDKind ValueKind(void) const
  {
    return typeRef.uidKind();
  }

  const BITBuffer * ValueEncoding(void) const
  {
    return (typeRef.uidKind() == UniversalID::uidLocal) ? &value.encoding : 0;
  }
  const AbstractSyntaxRfp & ValueASyntax(void) const
  {
    return value.astype;
  }
  const std::string & ValueDescriptor(void) const { return typeDescriptor; }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};

class TCUserInformation : public ASTypeAC {
protected:
  std::list<TCExternal> _extVals;

public:
  TCUserInformation()
    : ASTypeAC(ASTag::tagContextSpecific, 30)
  { }

  class iterator {
  protected:
    friend class TCUserInformation;
    typedef std::list<TCExternal>::iterator XIterator;
    XIterator _rIt;
    XIterator _endIt;

    iterator(const XIterator & use_it, const XIterator & end_it)
      : _rIt(use_it), _endIt(end_it)
    { }

  public:
    iterator()
    { }
    ~iterator()
    { }

    TCExternal & operator*() const { return *_rIt; }
    TCExternal * operator->() const { return _rIt.operator->(); }

    bool  hasNext(void) const
    {
      XIterator tmp = _rIt;
      return !((tmp == _endIt) || ((++tmp) == _endIt));
    }

    iterator & operator++() //preincrement
    {
      ++_rIt; return *this;
    }
    iterator  operator++(int) //postincrement
    {
      iterator  tmp = *this;
      ++_rIt; return tmp;
    }
    bool operator==(const iterator & i2) const
    {
      return (_rIt == i2._rIt);
    }
    bool operator!=(const iterator & i2) const
    {
      return (_rIt != i2._rIt);
    }
  };

  class const_iterator {
  protected:
    friend class TCUserInformation;
    typedef std::list<TCExternal>::const_iterator XIterator;
    XIterator _rIt;
    XIterator _endIt;

    const_iterator(const XIterator & use_it, const XIterator & end_it)
      : _rIt(use_it), _endIt(end_it)
    { }

  public:
    const_iterator()
    { }
    const_iterator(const iterator & use_it)
      : _rIt(use_it._rIt), _endIt(use_it._endIt)
    { }
    ~const_iterator()
    { }

    const TCExternal & operator*() const { return *_rIt; }
    const TCExternal * operator->() const { return _rIt.operator->(); }

    bool  hasNext(void) const
    {
      XIterator tmp = _rIt;
      return !((tmp == _endIt) || ((++tmp) == _endIt));
    }

    const_iterator & operator++() //preincrement
    {
      ++_rIt; return *this;
    }
    const_iterator  operator++(int) //postincrement
    {
      const_iterator tmp = *this;
      ++_rIt; return tmp;
    }
    bool operator==(const const_iterator & i2) const
    {
      return (_rIt == i2._rIt);
    }
    bool operator!=(const const_iterator & i2) const
    {
      return (_rIt != i2._rIt);
    }
  };

  iterator begin(void)
  {
    return iterator(_extVals.begin(), _extVals.end());
  }

  const_iterator begin(void) const
  {
    return const_iterator(_extVals.begin(), _extVals.end());
  }

  unsigned size(void) const { return (unsigned)_extVals.size(); }
  bool empty(void) const { return _extVals.empty(); }

  const TCExternal * first(void) const
  {
    return !empty() ? &_extVals.front() : 0;
  }


  TCExternal & addUIASyntax(const AbstractSyntaxRfp & use_astyp)
  {
    _extVals.push_back(TCExternal());
    _extVals.back().setValueASyntax(use_astyp);
    return _extVals.back();
  }
  TCExternal & addUIEncoding(uint32_t use_uid, const BITBuffer & use_enc)
  {
    _extVals.push_back(TCExternal());
    _extVals.back().setValueEncoding(use_uid, use_enc);
    return _extVals.back();
  }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_USRINFO_DEFS_HPP */

