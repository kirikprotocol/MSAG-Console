/* ************************************************************************** *
 * Classes implementing TCAP dialogue user information.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_USRINFO_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_USRINFO_HPP

#include <string>
#include <list>

#include "eyeline/util/MaxSizeof.hpp"
#include "eyeline/asn1/AbstractSyntax.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::AbstractSyntaxRfp;
using eyeline::asn1::BITBuffer;

struct UIEncoding {
  uint32_t  typeId;
  BITBuffer typeEnc;

  UIEncoding() : typeId(0)
  { }
};

struct UIValue {
public:
  enum ValueKind_e { valNone = 0, valAS, valEnc };

private:
  uint8_t objMem[eyeline::util::MaxSizeOf2_T<AbstractSyntaxRfp, UIEncoding>::VALUE];

protected:
  ValueKind_e _kind;
  std::string _descr;
  union {
    void              * ptr;
    AbstractSyntaxRfp * pAs;   //set if typeRef is uidGlobal 
    UIEncoding        * pEnc; //set if typeref is uidLocal
  } _pVal;

  void resetObj(ValueKind_e use_kind = valNone)
  {
    if (_pVal.ptr ) {
      if (_kind == valAS)
        _pVal.pAs->~AbstractSyntaxRfp();
      else if (_kind == valEnc)
        _pVal.pEnc->~UIEncoding();
      _pVal.ptr = 0;
    }
    if ((_kind = use_kind) == valAS)
      _pVal.pAs = new(objMem)AbstractSyntaxRfp();
    else if (_kind == valEnc)
      _pVal.pEnc = new(objMem)UIEncoding();
  }

public:
  UIValue() : _kind(valNone)
  {
    _pVal.ptr = 0;
  }
  UIValue(const UIValue & use_ui)
    : _kind(valNone), _descr(use_ui._descr)
  {
    _pVal.ptr = 0;
    resetObj(use_ui._kind);
  }

  UIValue(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
    : _kind(valNone)
  {
    _pVal.ptr = 0;
    setASyntax(use_astyp, use_descr);
  }
  UIValue(uint32_t use_uid, const BITBuffer & use_enc, const char * use_descr = NULL)
    :  _kind(valNone)
  {
    _pVal.ptr = 0;
    setEncoding(use_uid, use_enc, use_descr);
  }

  void setASyntax(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
  {
    resetObj(valAS);
    *_pVal.pAs = use_astyp;
    if (use_descr)
      _descr = use_descr;
  }
  void setEncoding(uint32_t use_uid, const BITBuffer & use_enc, const char * use_descr = NULL)
  {
    resetObj(valEnc);
    _pVal.pEnc->typeId = use_uid;
    _pVal.pEnc->typeEnc = use_enc;
    if (use_descr)
      _descr = use_descr;
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
  const UIEncoding * getEncoding(void) const
  {
    return (_kind == valEnc) ? _pVal.pEnc : 0;
  }
};


class TDlgUserInfo { //list of UIValue
protected:
  std::list<UIValue>    _listUI;

public:
  TDlgUserInfo()
  { }
  ~TDlgUserInfo()
  { }

  void addUIValue(const UIValue & use_ui)
  {
    _listUI.push_back(use_ui);
  }
  void addUIValue(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
  {
    _listUI.push_back(UIValue(use_astyp, use_descr));
  }
  void addUIValue(uint32_t use_uid, const BITBuffer & use_enc, const char * use_descr = NULL)
  {
    _listUI.push_back(UIValue(use_uid, use_enc, use_descr));
  }

  unsigned size(void) const { return (unsigned)_listUI.size(); }
  //
  bool empty(void) const { return _listUI.empty(); }

  //Returns pointer to 1st element if it exists
  const UIValue * first(void) const
  {
    return !_listUI.empty() ? &_listUI.front() : 0;
  }

  class iterator {
  protected:
    friend class TDlgUserInfo;
    typedef std::list<UIValue>::iterator XIterator;
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

    UIValue & operator*() const { return *_rIt; }
    UIValue * operator->() const { return _rIt.operator->(); }

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
    friend class TDlgUserInfo;
    typedef std::list<UIValue>::const_iterator XIterator;
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

    const UIValue & operator*() const { return *_rIt; }
    const UIValue * operator->() const { return _rIt.operator->(); }

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
    return iterator(_listUI.begin(), _listUI.end());
  }

  const_iterator begin(void) const
  {
    return const_iterator(_listUI.begin(), _listUI.end());
  }

  iterator end(void)
  {
    return iterator(_listUI.end(), _listUI.end());
  }

  const_iterator end(void) const
  {
    return const_iterator(_listUI.end(), _listUI.end());
  }

};


} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_USRINFO_HPP */

