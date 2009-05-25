/* ************************************************************************** *
 * Classes implementing TC messages user information.
 * ************************************************************************** */
#ifndef __TC_USRINFO_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_USRINFO_DEFS_HPP

//#include <list>
//#include "eyeline/asn1/AbstractSyntax.hpp"
#include "eyeline/tcap/TDlgUserInfo.hpp"

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
using eyeline::asn1::BITBuffer;

using eyeline::tcap::UIValue;

//ASN.1 1997 EXTERNAL type adopted for TC.
//keeps TC DialoguePDU or UniDialoguePDU in case of global(direct)
//type reference, any type in case of local(indirect) reference.
class TCExternal : public UIValue, public ASTypeAC {
public:
  TCExternal()
    : ASTypeAC(ASTag::tagUniversal, 8)
  { }
  TCExternal(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
    : UIValue(use_astyp, use_descr), ASTypeAC(ASTag::tagUniversal, 8)
  { }
  TCExternal(uint32_t use_uid, const BITBuffer & use_enc, const char * use_descr = NULL)
    : UIValue(use_uid, use_enc, use_descr), ASTypeAC(ASTag::tagUniversal, 8)
  { }
  TCExternal(const UIValue & use_val)
    : UIValue(use_val), ASTypeAC(ASTag::tagUniversal, 8)
  { }
  ~TCExternal()
  { }

  static ENCResult EncodeUI(const UIValue & use_val, BITBuffer & use_buf,
                          EncodingRule use_rule = ruleDER) /*throw ASN1CodecError*/;

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  virtual ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
  {
    return EncodeUI(*this, use_buf, use_rule);
  }

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  virtual DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  virtual DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

};


class TCUserInformation : public ASTypeAC { // list of TCExternal
protected:
  std::list<TCExternal> _extVals;

public:
  TCUserInformation()
    : ASTypeAC(ASTag::tagContextSpecific, 30)
  { }

  unsigned export2TDlgUI(TDlgUserInfo & use_uil)
  {
    if (_extVals.empty())
      return 0;

    unsigned i = 0;
    for (std::list<TCExternal>::const_iterator 
         cit = _extVals.begin(); cit != _extVals.end(); ++cit, ++i) {
      use_uil.addUIValue(*cit);
    }
    return i;
  }

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

  iterator end(void)
  {
    return iterator(_extVals.end(), _extVals.end());
  }

  const_iterator end(void) const
  {
    return const_iterator(_extVals.end(), _extVals.end());
  }

  unsigned size(void) const { return (unsigned)_extVals.size(); }
  bool empty(void) const { return _extVals.empty(); }

  const TCExternal * first(void) const
  {
    return !empty() ? &_extVals.front() : 0;
  }


  void addUIValue(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
  {
    _extVals.push_back(TCExternal(use_astyp, use_descr));
  }
  void addUIValue(uint32_t use_uid, const BITBuffer & use_enc, const char * use_descr = NULL)
  {
    _extVals.push_back(TCExternal(use_uid, use_enc, use_descr));
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

