/* ************************************************************************** *
 * Classes implementing TCAP dialogue user information.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_USRINFO_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_USRINFO_HPP

#include <string>
#include <list>

#include "eyeline/util/MaxSizeof.hpp"
#include "eyeline/asn1/ASExternal.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::AbstractSyntaxRfp;
using eyeline::asn1::ASExternalValue;
using eyeline::asn1::BITBuffer;


class TDlgUserInfo { //list of ASExternalValue
protected:
  std::list<ASExternalValue>    _listUI;

public:
  TDlgUserInfo()
  { }
  ~TDlgUserInfo()
  { }

  void addUIValue(const ASExternalValue & use_ui)
  {
    _listUI.push_back(use_ui);
  }
  void addUIValue(const AbstractSyntaxRfp & use_astyp, const char * use_descr = NULL)
  {
    _listUI.push_back(ASExternalValue(use_astyp, use_descr));
  }
  void addUIValue(uint32_t use_uid, const BITBuffer & use_enc, const char * use_descr = NULL)
  {
    _listUI.push_back(ASExternalValue(use_uid, use_enc, use_descr));
  }

  unsigned size(void) const { return (unsigned)_listUI.size(); }
  //
  bool empty(void) const { return _listUI.empty(); }
  //
  void clear(void) { _listUI.clear(); }

  //Returns pointer to 1st element if it exists
  const ASExternalValue * first(void) const
  {
    return !_listUI.empty() ? &_listUI.front() : 0;
  }

  class iterator {
  protected:
    friend class TDlgUserInfo;
    typedef std::list<ASExternalValue>::iterator XIterator;
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

    ASExternalValue & operator*() const { return *_rIt; }
    ASExternalValue * operator->() const { return _rIt.operator->(); }

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
    typedef std::list<ASExternalValue>::const_iterator XIterator;
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

    const ASExternalValue & operator*() const { return *_rIt; }
    const ASExternalValue * operator->() const { return _rIt.operator->(); }

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

