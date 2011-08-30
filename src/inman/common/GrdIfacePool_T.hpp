/* ************************************************************************** *
 * Base abstract class of multithreadsafe accumulative pool of objects.
 * implementing some interface.
 * Pooled objects access/releasing is guarded by reference counting pointers.
 * Only pooled objects interface is exposed.
 * Memory for pooled objects is allocated once on heap and then just reused, 
 * providing fast objects allocation/releasing.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_GUARDED_IFACE_POOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_GUARDED_IFACE_POOL

#include <vector>

#include "inman/common/GrdObjPoolDefs.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace util {

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;


template <
  class _IfaceArg                   //MUST have no public destructor !!!
, typename _SizeTypeArg = unsigned  //implicitly restricts pool capacity 
>
class GrdIfacePoolAC_T {
public:
  typedef _SizeTypeArg size_type;

protected:
  // Pooled node housekeeping structure.
  class PoolAnchorAC : public PoolAnchorAC_T<_IfaceArg, _SizeTypeArg> {
  protected:
    GrdIfacePoolAC_T *  _pool;

    PoolAnchorAC(GrdIfacePoolAC_T & use_pool, _SizeTypeArg use_idx)
      : PoolAnchorAC_T<_IfaceArg, _SizeTypeArg>(use_idx), _pool(&use_pool)
    { }

  public:
    virtual ~PoolAnchorAC()
    { }

    //destroys pooled object of anchored node
    virtual void clearIface(void) = 0;

    // --------------------------------------
    // -- PoolAnchorAC_T<> interface method
    // --------------------------------------
    //Returns current number of refs
    virtual _SizeTypeArg getRefsGuarded(void) const { return _pool->getNodeRefs(this); }
    //Returns false if maximum number of refs is already reached
    virtual bool addRefGuarded(void) const { return _pool->refNode(this); }
    //Returns true if last ref is just reset
    virtual bool unRefGuarded(void) { return _pool->unrefNode(this); }
    //Returns pooled object iface, constructing it if necessary
    virtual _IfaceArg * getIface(void) = 0;
  };

  //Aggregates node anchor and memory for pooled object implementation.
  template <
    class _IfaceImplArg // : public _IfaceArg {}, may have only default constructor defined.
  >
  class AnchoredNode_T : public PoolAnchorAC {
  private:
    union {
      void *        _aligner;
      unsigned char _buf[sizeof(_IfaceImplArg)];
    } _mem;

    bool  _hasObj;

    AnchoredNode_T(const AnchoredNode_T & use_obj);
    AnchoredNode_T & operator=(const AnchoredNode_T & use_obj);

  protected:
    _IfaceImplArg * pObj(void) { return (_IfaceImplArg *)(_mem._buf); }

  public:
    AnchoredNode_T(GrdIfacePoolAC_T & use_pool, size_type use_idx)
      : PoolAnchorAC(use_pool, use_idx), _hasObj(false)
    {
      _mem._aligner = 0;
    }
    //
    virtual ~AnchoredNode_T()
    {
      clearIface();
    }

    // --------------------------------------
    // -- PoolAnchorAC interface method
    // --------------------------------------
    //destroys pooled object of anchored node
    virtual void clearIface(void)
    {
      if (_hasObj) {
        pObj()->~_IfaceImplArg();
        _hasObj = false;
      }
    }

    // --------------------------------------
    // -- PoolAnchorAC_T<> interface method
    // --------------------------------------
    //Returns pooled object iface, constructing it if necessary
    virtual _IfaceArg * getIface(void)
    {
      if (!_hasObj) {
        new (_mem._buf)_IfaceImplArg();
        _hasObj = true;
      }
      return pObj();
    }
  };

private:
  //Array of pointers to allocated nodes
  class NodeArray : public std::vector<PoolAnchorAC *> {
  protected:
    using std::vector<PoolAnchorAC *>::clear;

    NodeArray(const NodeArray & cp_obj);
    NodeArray & operator=(const NodeArray & cp_obj);

  public:
    NodeArray() : std::vector<PoolAnchorAC *>()
    { }
    ~NodeArray()
    {
      destroy();
    }

    void destroy(void)
    {
      typename std::vector<PoolAnchorAC *>::size_type i = 0;
      for (; i < this->size(); ++i) {
        this->at(i)->clearIface();
        delete this->at(i);
        this->at(i) = 0;
      }
    }
  };

  typedef smsc::util::FifoQueue_T<PoolAnchorAC, _SizeTypeArg> NodeQueue;

  /* -- DATA members: -- */
  mutable Mutex _sync;
  const bool    _doErase; //object release mode: destroy or just mark as unused
  NodeArray     _store;   //store of all allocated nodes.
  NodeQueue     _pool;    //queue of unused nodes

protected:
#ifdef __GRD_POOL_DEBUG__
  const char *  _logId;
  Logger *      _logger;
#endif /* __GRD_POOL_DEBUG__ */

  // ------------------------------------------------
  // -- GrdIfacePoolAC_T interface methods
  // ------------------------------------------------
  virtual PoolAnchorAC * allocateNode(size_type node_idx) = 0;

  //Releases (marks as unused) a given node
  void rlseNode(PoolAnchorAC * p_node)
  {
    if (!_pool.isLinked(p_node)) {
#ifdef __GRD_POOL_DEBUG__
      smsc_log_debug(_logger, "%s: rlseNode[%u] numRefs = %u to pool", _logId,
                     p_node->getNodeIdx(), p_node->getRefs());
#endif /* __GRD_POOL_DEBUG__ */

      _pool.push_back(p_node); //downcast PoolAnchorAC to FifoLink
      if (_doErase)
        p_node->clearIface();
    }
#ifdef __GRD_POOL_DEBUG__
    else {
      smsc_log_error(_logger, "%s: rlseNode[%u] numRefs = %u is already in pool", _logId,
                     p_node->getNodeIdx(), p_node->getRefs());
    }
#endif /* __GRD_POOL_DEBUG__ */
  }

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void extend(size_type num_to_reserve)
  {
    if (_store.size() < num_to_reserve) {
      size_type i = (size_type)_store.size();
      _store.resize(num_to_reserve, NULL);
      for (; i < num_to_reserve; ++i)
        _pool.push_back(_store[i] = allocateNode(i));
    }
  }

  friend class PoolAnchorAC;
  //Returns current number of refs to node
  size_type getNodeRefs(const PoolAnchorAC * p_node) const
  {
    MutexGuard  grd(_sync);
    return p_node->getRefs();
  }
  //Returns false if maximum number of refs is already reached
  bool refNode(const PoolAnchorAC * p_node) const
  {
    MutexGuard  grd(_sync);
#ifdef __GRD_POOL_DEBUG__
    smsc_log_debug(_logger, "%s: refNode[%u] numRefs = %u", _logId,
                   p_node->getNodeIdx(), p_node->getRefs());
#endif /* __GRD_POOL_DEBUG__ */
    return p_node->addRef();
  }
  //Returns true if given node is finally released.
  bool unrefNode(PoolAnchorAC * p_node)
  {
    MutexGuard  grd(_sync);
#ifdef __GRD_POOL_DEBUG__
    smsc_log_debug(_logger, "%s: unrefNode[%u] numRefs = %u", _logId,
                   p_node->getNodeIdx(), p_node->getRefs());
#endif /* __GRD_POOL_DEBUG__ */
    if (p_node->unRef()) {
      rlseNode(p_node);
      return true;
    }
    return false;
  }

public:
  //Guards pooled object access/deallocation, exposes pooled object interface.
  class ObjRef : public PoolObjRefAC_T<_IfaceArg, _SizeTypeArg> {
  protected:
    friend class GrdIfacePoolAC_T;

    explicit ObjRef(PoolAnchorAC & use_node)
      : PoolObjRefAC_T<_IfaceArg, _SizeTypeArg>(use_node)
    { }

  public:
    ObjRef() : PoolObjRefAC_T<_IfaceArg, _SizeTypeArg>()
    { }
    ObjRef(const ObjRef & cp_obj)
      : PoolObjRefAC_T<_IfaceArg, _SizeTypeArg>(cp_obj)
    { }
    //
    ~ObjRef()
    { }

#ifdef __GRD_POOL_DEBUG__
    void logThis(Logger * use_log, const char * log_id) const
    {
      this->pNode()->logThis(use_log, log_id);
    }
#endif /* __GRD_POOL_DEBUG__ */
  };

  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  explicit GrdIfacePoolAC_T(bool erase_on_rlse = true) : _doErase(erase_on_rlse)
#ifdef __GRD_POOL_DEBUG__
                 , _logId(0), _logger(0)
#endif /* __GRD_POOL_DEBUG__ */
  { }
  virtual ~GrdIfacePoolAC_T()
  {
    MutexGuard  grd(_sync);
  }

#ifdef __GRD_POOL_DEBUG__
  void debugInit(const char * log_id, Logger * use_log)
  {
    _logId = log_id; _logger = use_log;
  }
#endif /* __GRD_POOL_DEBUG__ */

  //Returns total number of allocated objects managed by pool
  size_type capacity(void) const
  {
    MutexGuard  grd(_sync);
    return (size_type)_store.size();
  }

  //Returns number of currently used objects.
  size_type usage(void) const
  {
    MutexGuard  grd(_sync);
    return (size_type)_store.size() - _pool.size();
  }

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void reserve(size_type num_to_reserve)
  {
    MutexGuard  grd(_sync);
    extend(num_to_reserve);
  }

  //Allocates an object.
  //Returns empty ObjRef in case of maximum number of objects is already reached!
  ObjRef allcObj(void)
  {
    MutexGuard  grd(_sync);
    if (_pool.empty()) {
      size_type newSz = (size_type)_store.size() + 1;
      //check for size_type overloading
      if (newSz < (size_type)_store.size())
        return ObjRef();
      extend(newSz);
    }
    return ObjRef(*(_pool.pop_front()));
  }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_GUARDED_IFACE_POOL */

