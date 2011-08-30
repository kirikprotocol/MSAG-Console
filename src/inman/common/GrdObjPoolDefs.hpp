/* ************************************************************************** *
 * Templates of housekeeping structures of generic accumulative pool of objects.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_GUARDED_OBJ_POOL_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_GUARDED_OBJ_POOL_DEFS

//#define __GRD_POOL_DEBUG__

#ifdef __GRD_POOL_DEBUG__
#include "logger/Logger.h"
using smsc::logger::Logger;
#endif /* __GRD_POOL_DEBUG__ */

#include "inman/common/FifoQueue.hpp"

namespace smsc {
namespace util {

// --------------------------------------------------------------------------
// Transforms any class to pooled object interface class.
// --------------------------------------------------------------------------
template <
  class _TArg
>
class PooledObj_T : public _TArg {
private:
  PooledObj_T(const PooledObj_T & cp_obj);
  PooledObj_T & operator=(const PooledObj_T & cp_obj);

protected:
  PooledObj_T() : _TArg()
  { }
  ~PooledObj_T()
  { }
};

// --------------------------------------------------------------------------
// Base class of pooled node housekeeping structure.
// --------------------------------------------------------------------------
template <
  class _IfaceArg         //MUST have no public destructor !!!
, typename _SizeTypeArg   //implicitly restricts pool capacity
>
class PoolAnchorAC_T : public FifoLink {
private:
  PoolAnchorAC_T(const PoolAnchorAC_T & cp_obj);
  PoolAnchorAC_T & operator=(const PoolAnchorAC_T & cp_obj);

protected:
  const _SizeTypeArg     _nodeIdx; //index of anchored node
  mutable _SizeTypeArg   _refs;

  explicit PoolAnchorAC_T(_SizeTypeArg use_idx)
    : FifoLink(), _nodeIdx(use_idx), _refs(0)
  { }

public:
  virtual ~PoolAnchorAC_T()
  { }

#ifdef __GRD_POOL_DEBUG__
    void logThis(Logger * use_log, const char * log_id) const
    {
      smsc_log_debug(use_log, "%s: _pNode=%p {_prev=%p, _next=%p, _nodeIdx=%u, _refs=%u}",
                     log_id, this, _prev, _next, (unsigned)_nodeIdx, (unsigned)_refs);
    }
#endif /* __GRD_POOL_DEBUG__ */

  //Returns false if maximum number of refs is already reached
  bool addRef(void) const
  {
    if (!++_refs) {
      --_refs; return false;
    }
    return true;
  }
  //Returns true if last ref is just reset
  bool unRef(void) const
  {
    if (_refs)
      --_refs;
    return (_refs == 0);
  }

  _SizeTypeArg getRefs(void) const { return _refs; }

  _SizeTypeArg getNodeIdx(void) const { return _nodeIdx; }

  // --------------------------------------
  // -- PoolAnchorAC_T<> interface method
  // --------------------------------------
  //Returns current number of refs
  virtual _SizeTypeArg getRefsGuarded(void) const = 0;
  //Returns false if maximum number of refs is already reached
  virtual bool addRefGuarded(void) const = 0;
  //Returns true if last ref is just reset
  virtual bool unRefGuarded(void)  = 0;
  //Returns pooled object iface, constructing it if necessary
  virtual _IfaceArg * getIface(void) = 0;
};

// --------------------------------------------------------------------------
// Pooled object reference: guards pooled object access/deallocation, 
// exposes pooled object interface.
// --------------------------------------------------------------------------
template <
  class _IfaceArg         //MUST have no public destructor !!!
, typename _SizeTypeArg   //implicitly restricts pool capacity
>
class PoolObjRefAC_T {
private:
  PoolAnchorAC_T<_IfaceArg, _SizeTypeArg> * _pNode;

  void setRef(void)
  {
    if (_pNode && !_pNode->addRefGuarded())
      _pNode = NULL;
  }

protected:
#ifdef __GRD_POOL_DEBUG__
  const PoolAnchorAC_T<_IfaceArg, _SizeTypeArg> * pNode(void) const { return _pNode; }
#endif /* __GRD_POOL_DEBUG__ */

  explicit PoolObjRefAC_T(PoolAnchorAC_T<_IfaceArg, _SizeTypeArg> & use_node)
    : _pNode(&use_node)
  {
    if (!_pNode->addRef()) //Note: non-guarded variant here !!!
      _pNode = NULL;
  }

public:
  PoolObjRefAC_T() : _pNode(NULL)
  { }
  PoolObjRefAC_T(const PoolObjRefAC_T & cp_obj) : _pNode(cp_obj._pNode)
  {
    setRef();
  }
  //
  ~PoolObjRefAC_T()
  {
    release();
  }

  void release(void)
  {
    if (_pNode) {
      _pNode->unRefGuarded(); _pNode = NULL;
    }
  }

  _SizeTypeArg getPoolIdx(void) const { return _pNode ? _pNode->getNodeIdx() : 0; }

  _SizeTypeArg getRefs(void) const { return _pNode ? _pNode->getRefsGuarded() : 0; }

  _IfaceArg * get(void) const { return _pNode ? _pNode->getIface() : NULL; }

  _IfaceArg * operator->(void) const { return get(); }

  _IfaceArg & operator*(void) const { return *get(); }

  PoolObjRefAC_T & operator= (const PoolObjRefAC_T & cp_obj)
  {
    if (this != &cp_obj) {
      release();
      _pNode = cp_obj._pNode;
      setRef();
    }
    return *this;
  }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_GUARDED_OBJ_POOL_DEFS */

