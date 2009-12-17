/* ************************************************************************** *
 * ROSComponentPrimitive list with single-direction iterators
 * ************************************************************************** */
#ifndef __ROS_COMPONENTS_LIST_HPP
#ident "@(#)$Id$"
#define __ROS_COMPONENTS_LIST_HPP

#include <list>
#include "eyeline/ros/ROSComponent.hpp"
#include "eyeline/util/MTRefPtr.hpp"

namespace eyeline {
namespace ros {

//typedef eyeline::util::RFPtr_T<ROSComponentPrimitive> ROSComponentRfp;
//typedef eyeline::util::URFPtr_T<ROSComponentPrimitive> ROSComponentURfp;
//typedef eyeline::util::CRFPtr_T<ROSComponentPrimitive> ROSComponentCRfp;

typedef std::list<ROSComponentPrimitive*> ROSComponentsList;

#if 0
class ROSComponentsList {
protected:
  //NOTE: component utilization depends on RFP kind(CRFP or URFP)
  std::list<ROSComponentPrimitive*> _comps;

public:
  ROSComponentsList()
  { }
  ~ROSComponentsList()
  {
    //TODO: clean up _comps
  }

  class iterator {
  protected:
    friend class ROSComponentsList;
    typedef std::list<ROSComponentPrimitive*>::iterator XIterator;
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

    ROSComponentPrimitive& operator*() const { return *_rIt; }
    ROSComponentPrimitive* operator->() const { return _rIt.operator->(); }

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
    friend class ROSComponentsList;
    typedef std::list<ROSComponentPrimitive*>::const_iterator XIterator;
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

    const ROSComponentRfp & operator*() const { return *_rIt; }
    const ROSComponentRfp * operator->() const { return _rIt.operator->(); }

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
      const_iterator  tmp = *this;
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


  //NOTE: value referenced by iterator is undefined if list is empty!
  iterator begin(void)
  {
    return iterator(_comps.begin(), _comps.end());
  }
  //NOTE: value referenced by iterator is undefined if list is empty!
  const_iterator begin(void) const
  {
    return const_iterator(_comps.begin(), _comps.end());
  }

  iterator end(void)
  {
    return iterator(_comps.end(), _comps.end());
  }

  const_iterator end(void) const
  {
    return const_iterator(_comps.end(), _comps.end());
  }

  unsigned size(void) const { return (unsigned)_comps.size(); }
  bool empty(void) const { return _comps.empty(); }

  void clear(void) { _comps.clear(); }
  void push_back(ROSComponentPrimitive* use_comp) { _comps.push_back(use_comp); }

  //TODO: add searching helpers (by opcode, ROS kind, etc)

};
#endif

} //ros
} //eyeline

#endif /* __ROS_COMPONENTS_LIST_HPP */

