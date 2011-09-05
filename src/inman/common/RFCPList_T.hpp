/* ************************************************************************** *
 * Multithread safe Reference Counting List of Pointers.
 * Allows safe modification of various nodes performed by several threads               .
 * simultaneously.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_MT_RFC_PTR_LIST_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_MT_RFC_PTR_LIST_HPP


#include "inman/common/RFCPList.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace util {

using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;

/* RFCPList_T<> USAGE Samples:
 *
 * class Sample : RFCPList_T< SampleObj > {
 * public:
 *    
 *   void safeModify(void)    //sample of list traversing
 *   {
 *     for (RFCPList_T<SampleObj>::iterator it = begin(); !it.isEnd(); ++it)
 *       it->anyObjMethod(this); //(*this) may be safely modified by this call.
 *   }
 * };
 */
template < class _TArg >
class RFCPListGuarded_T {
protected:
  typedef RFCPList::iterator       RfcIterator;
  typedef RFCPList::const_iterator RfcConstIterator;

  struct GuardedList {
    Mutex *   _sync;
    RFCPList  _rfcList;

    explicit GuardedList(Mutex * use_sync = NULL)
      : _sync(use_sync)
    { }
  };

  GuardedList _grdList;

public:
  explicit RFCPListGuarded_T(Mutex * use_sync = NULL)
    : _grdList(use_sync)
  { }
  ~RFCPListGuarded_T()
  { }

  void init(Mutex & use_sync) { _grdList._sync = &use_sync; }

  class iterator {
  protected:
    friend class RFCPListGuarded_T;

    GuardedList * _pList; //referenced list with guard
    RfcIterator   _rIt;   //iterator of referenced node

    //NOTE.1: targeted list MUST be locked!
    //NOTE.2: Constructor cann't be called for iterator referencing erased node!
    iterator(GuardedList & use_list, const RfcIterator & use_it)
      : _pList(&use_list), _rIt(use_it)
    {
      _pList->_rfcList.markNode(_rIt);
    }

    void releaseNode(void)
    {
      MutexGuard grd(*_pList->_sync);
      _pList->_rfcList.releaseNode(_rIt);
    }

    iterator insertNode(_TArg * p_obj)
    {
      MutexGuard grd(*_pList->_sync);
      RfcIterator nit = _pList->_rfcList.insert(_rIt, p_obj);
      return iterator(*_pList, nit);
    }

  public:
    iterator() : _pList(NULL)
    { }
    iterator(const iterator & use_it)
      : _pList(use_it._pList), _rIt(use_it._rIt)
    {
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.markNode(_rIt);
      }
    }
    ~iterator()
    {
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.unmarkNode(_rIt);
      }
    }

    _TArg * operator->() const { return static_cast<_TArg *>(_rIt->_value); }
    _TArg * & operator*() { return *reinterpret_cast<_TArg **>((&(_rIt->_value))); }

    bool  isEnd(void) const { return (_rIt == _pList->_rfcList.end()); }

    iterator & operator=(const iterator & cp_it)
    {
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.unmarkNode(_rIt);
      }
      _pList = cp_it._pList;
      _rIt = cp_it._rIt;
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.markNode(_rIt);
      }
      return *this;
    }

    iterator & operator++() //preincrement
    {
      MutexGuard grd(*_pList->_sync);
      _pList->_rfcList.unmarkNode(_rIt);
      _pList->_rfcList.nextNode(_rIt);
      _pList->_rfcList.markNode(_rIt);
      return *this;
    }
    iterator  operator++(int) //postincrement
    {
      iterator  tmp = *this;
      ++(*this);
      return tmp;
    }

    iterator & operator--() //preiderement
    {
      MutexGuard grd(*_pList->_sync);
      _pList->_rfcList.unmarkNode(_rIt);
      _pList->_rfcList.prevNode(_rIt);
      _pList->_rfcList.markNode(_rIt);
      return *this;
    }
    iterator  operator--(int) //postdecrement
    {
      iterator  tmp = *this;
      --(*this);
      return tmp;
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
    friend class RFCPListGuarded_T;

    const GuardedList * _pList; //referenced list with guard
    RfcConstIterator    _rIt;   //const_iterator of referenced node

    //NOTE.1: targeted list MUST be locked!
    //NOTE.2: Constructor cann't be called for iterator referencing erased node!
    const_iterator(const GuardedList & use_list, const RfcConstIterator & use_it)
      : _pList(&use_list), _rIt(use_it)
    {
      _pList->_rfcList.markNode(_rIt);
    }

  public:
    const_iterator() : _pList(NULL)
    { }
    const_iterator(const const_iterator & use_it)
      : _pList(use_it._pList), _rIt(use_it._rIt)
    {
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.markNode(_rIt);
      }
    }
    const_iterator(const iterator & use_it)
      : _pList(use_it._pList), _rIt(use_it._rIt)
    {
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.markNode(_rIt);
      }
    }
    ~const_iterator()
    {
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.unmarkNode(_rIt);
      }
    }

    _TArg * operator->() const { return static_cast<_TArg *>(_rIt->_value); }
    _TArg * const & operator*() const { return *reinterpret_cast<_TArg **>((&(_rIt->_value))); }

    bool  isEnd(void) const { return (_rIt == _pList->_rfcList.end()); }

    const_iterator & operator=(const const_iterator & cp_it)
    {
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.unmarkNode(_rIt);
      }
      _pList = cp_it._pList;
      _rIt = cp_it._rIt;
      if (_pList) {
        MutexGuard grd(*_pList->_sync);
        _pList->_rfcList.markNode(_rIt);
      }
      return *this;
    }

    const_iterator & operator++() //preincrement
    {
      MutexGuard grd(*_pList->_sync);
      _pList->_rfcList.unmarkNode(_rIt);
      _pList->_rfcList.nextNode(_rIt);
      _pList->_rfcList.markNode(_rIt);
      return *this;
    }
    const_iterator  operator++(int) //postincrement
    {
      iterator  tmp = *this;
      ++(*this);
      return tmp;
    }

    const_iterator & operator--() //preiderement
    {
      MutexGuard grd(*_pList->_sync);
      _pList->_rfcList.unmarkNode(_rIt);
      _pList->_rfcList.prevNode(_rIt);
      _pList->_rfcList.markNode(_rIt);
      return *this;
    }
    const_iterator  operator--(int) //postdecrement
    {
      const_iterator  tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const const_iterator & i2) const
    {
      return (_rIt == i2._rIt);
    }
    bool operator!=(const const_iterator & i2) const
    {
      return (_rIt != i2._rIt);
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

  iterator begin(void)
  {
    MutexGuard grd(*_grdList._sync);
    return iterator(_grdList, _grdList._rfcList.begin());
  }

  const_iterator begin(void) const
  {
    MutexGuard grd(*_grdList._sync);
    return const_iterator(_grdList, _grdList._rfcList.begin());
  }

  iterator end(void)
  {
    MutexGuard grd(*_grdList._sync);
    return iterator(_grdList, _grdList._rfcList.end());
  }

  const_iterator end(void) const
  {
    MutexGuard grd(*_grdList._sync);
    return const_iterator(_grdList, _grdList._rfcList.end());
  }

  //erases node referenced by 'it'
  void erase(iterator it) { it.releaseNode(); }

  //inserts 'p_obj' right before node referenced by 'it'
  iterator insert(iterator it, _TArg * p_obj) { return it.insertNode(p_obj); }

  //
  void push_front(_TArg * p_obj)
  {
    MutexGuard tmp(*_grdList._sync);
    _grdList._rfcList.push_front(p_obj);
  }
  //
  void push_back(_TArg * p_obj)
  {
    MutexGuard tmp(*_grdList._sync);
    _grdList._rfcList.push_back(p_obj);
  }
  //
  void remove(_TArg * p_obj)
  {
    MutexGuard tmp(*_grdList._sync);
    RfcIterator it = _grdList._rfcList.findNode(p_obj);
    _grdList._rfcList.releaseNode(it);
  }
  //inserts 'next_obj' right before 'p_obj'
  void insert(_TArg * p_obj, _TArg * next_obj)
  {
    MutexGuard tmp(*_grdList._sync);
    RfcIterator it = _grdList._rfcList.findNode(p_obj);
    _grdList._rfcList.insert(it, next_obj);
  }
  //inserts 'next_obj' right after 'p_obj'
  void follow(_TArg * p_obj, _TArg * next_obj)
  {
    MutexGuard tmp(*_grdList._sync);
    RfcIterator it = _grdList._rfcList.findNode(p_obj);
    if (it != _grdList._rfcList.end())
      ++it;
    _grdList._rfcList.insert(it, next_obj);
  }

  bool empty(void) const
  {
    MutexGuard tmp(*_grdList._sync);
    return _grdList._rfcList.empty();
  }

  void clear(void)
  {
    MutexGuard tmp(*_grdList._sync);
    return _grdList._rfcList.clear();
  }
};


template < class _TArg, class _GuardTArg = Mutex >
class RFCPList_T : public RFCPListGuarded_T<_TArg> {
private:
  using RFCPListGuarded_T<_TArg>::init;

  _GuardTArg  _sync;

public:
  RFCPList_T() : RFCPListGuarded_T<_TArg>()
  {
    this->init(_sync);
  }
  ~RFCPList_T()
  { }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_MT_RFC_PTR_LIST_HPP */

