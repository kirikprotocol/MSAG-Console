/* ************************************************************************** *
 * Basic class of generic Mulrithread-safe Intrusive pool, that additionally                                                                      .
 * provides direct access to maintained objects using index assigned by pool                                                                      .
 * to memory block containing object.                                                                                                                                               .
 * Pooled objects access/releasing is guarded by reference counting pointers.
 * Memory for pooled objects is allocated once and then just reused.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_INTRUSIVE_POOL_CORE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_INTRUSIVE_POOL_CORE

#include <vector>

#include "util/OptionalObjT.hpp"
#include "core/buffers/FifoList.hpp"
#include "core/synchronization/Mutex.hpp"

#ifdef INTRUSIVE_POOL_DEBUG
#include "logger/Logger.h"
using smsc::logger::Logger;
#endif /* INTRUSIVE_POOL_DEBUG */

namespace smsc {
namespace core {
namespace buffers {

// ---------------------------------------------------------------------------
// Basic class of Mulrithread-safe Intrusive pool, that maintains direct
// access to arbitrary objects.
// ---------------------------------------------------------------------------
template <
  typename  _IndexTypeArg //MUST be an unsigned integer type
                          //implicitly restricts pool capacity.
>
class IDAPoolCoreAC_T {
public:
  typedef _IndexTypeArg size_type;

  class NodeHeader : public FifoLink {
  private:
    NodeHeader(const NodeHeader & use_obj);
    NodeHeader & operator=(const NodeHeader & use_obj);

  protected:
    const size_type     mNodeIdx; //unique index of this node
    mutable size_type   mRefs;
    IDAPoolCoreAC_T *   mPool;

    friend class IDAPoolCoreAC_T;
    //
    NodeHeader(IDAPoolCoreAC_T & use_pool, size_type use_idx)
      : FifoLink(), mNodeIdx(use_idx), mRefs(0), mPool(&use_pool)
    { }

  public:
    ~NodeHeader()
    { }

    //Returns false if maximum number of refs is already reached
    bool addRef(void) const
    {
      if (!++mRefs) {
        --mRefs;
        return false;
      }
      return true;
    }
    //Returns true if last ref is just reset
    bool unRef(void) const
    {
      if (mRefs)
        --mRefs;
      return (mRefs == 0);
    }

    size_type getRefs(void) const { return mRefs; }

    size_type getNodeIdx(void) const { return mNodeIdx; }

    //Returns current number of refs
     size_type getRefsSynced(void) const { return mPool->getNodeRefs(*this); }
    //Returns false if maximum number of refs is already reached
    bool addRefSynced(void) const { return mPool->refNode(*this); }
    //Returns true if last ref is just reset
    bool unRefSynced(void) { return mPool->unrefNode(*this); }

#ifdef INTRUSIVE_POOL_DEBUG
    void logThis(Logger * use_log, const char * log_id) const
    {
      smsc_log_debug(use_log, "%s: pNode=%p {prev=%p, next=%p, mNodeIdx=%u, mRefs=%u}",
                   log_id, this, mPrev, mNext, (unsigned)mNodeIdx, (unsigned)mRefs);
    }
#endif /* INTRUSIVE_POOL_DEBUG */
  };

  //Guards pooled object destruction
  class ObjGuard {
  protected:
    NodeHeader * mpNodeHdr;

    void setRef(void)
    {
      if (mpNodeHdr && !mpNodeHdr->addRefSynced())
        mpNodeHdr = 0;
    }

    friend class IDAPoolCoreAC_T;
    //NOTE: constructor MUST BE called under referenced pool sync locked !!!
    explicit ObjGuard(NodeHeader * use_node) : mpNodeHdr(use_node)
    {
      if (mpNodeHdr && !mpNodeHdr->addRef()) //Note: non-synced variant here !!!
        mpNodeHdr = 0;
    }

  public:
    ObjGuard() : mpNodeHdr(0)
    { }
    ObjGuard(const ObjGuard & cp_obj) : mpNodeHdr(cp_obj.mpNodeHdr)
    {
      setRef();
    }
    //
    ~ObjGuard()
    {
      release();
    }

    bool empty(void) const { return mpNodeHdr == 0; }

    void release(void)
    {
      if (mpNodeHdr) {
        mpNodeHdr->unRefSynced();
        mpNodeHdr = 0;
      }
    }

    size_type getPoolIdx(void) const { return mpNodeHdr ? mpNodeHdr->getNodeIdx() : 0; }

    size_type getRefs(void) const { return mpNodeHdr ? mpNodeHdr->getRefsSynced() : 0; }

    ObjGuard & operator= (const ObjGuard & cp_obj)
    {
      if (this != &cp_obj) {
        release();
        mpNodeHdr = cp_obj.mpNodeHdr;
        setRef();
      }
      return *this;
    }
  };

  /* --  -- */
  IDAPoolCoreAC_T()
#ifdef INTRUSIVE_POOL_DEBUG
    : mLogId(0), mLogger(0)
#endif /* INTRUSIVE_POOL_DEBUG */
  { }
  virtual ~IDAPoolCoreAC_T()
  {
    //NOTE: successor's destructor MUST call 
    //  this->destroyAll();
  }

#ifdef INTRUSIVE_POOL_DEBUG
  void debugInit(const char * log_id, Logger * use_log)
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
    mLogId = log_id; mLogger = use_log;
  }
#endif /* INTRUSIVE_POOL_DEBUG */

  //Returns total number of allocated objects managed by pool
  size_type capacity(void) const
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
    return (size_type)mStore.size(); 
  }

  //Returns number of currently used objects.
  size_type usage(void) const
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
    return (size_type)(mStore.size() - mPool.size());
  }

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void reserve(size_type num_to_reserve)
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
    extend(num_to_reserve);
  }

private:
  class NodeArray : public std::vector<NodeHeader *> {
  protected:
    using std::vector<NodeHeader *>::clear;

    NodeArray(const NodeArray & cp_obj);
    NodeArray & operator=(const NodeArray & cp_obj);

  public:
    NodeArray() : std::vector<NodeHeader *>()
    { }
    ~NodeArray()
    { }
  };
  //
  typedef QueueOf_T<NodeHeader, size_type> NodeQueue;

  /* -- DATA members: -- */
  mutable smsc::core::synchronization::Mutex  mSync;
  /* */
  NodeArray   mStore;   //store of all allocated nodes.
  NodeQueue   mPool;    //queue of unused nodes

#ifdef INTRUSIVE_POOL_DEBUG
  const char *  mLogId;
  Logger *      mLogger;
#endif /* INTRUSIVE_POOL_DEBUG */

protected:
  smsc::core::synchronization::Mutex & poolSync(void) const { return mSync; }

  // --------------------------------------------------
  // -- IDAPoolCoreAC_T<> interface methods
  // --------------------------------------------------
  virtual NodeHeader * allocateNode(size_type use_idx) = 0;
  virtual void clearNode(NodeHeader & p_node) = 0;
  virtual void destroyNode(NodeHeader & p_node) = 0;


  void destroyAll(void)
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
#ifdef INTRUSIVE_POOL_DEBUG
    if (mLogger) {
      unsigned long numUsed = (unsigned long)(mStore.size() - mPool.size());
      if (numUsed) {
        smsc_log_error(mLogger, "%s: %lu objects still in use upon pool destruction", mLogId, numUsed);
      }
    }
#endif /* INTRUSIVE_POOL_DEBUG */
    for (typename NodeArray::size_type i = 0; i < mStore.size(); ++i) {
      if (mStore.at(i)) {
        destroyNode(*mStore.at(i));
        mStore.at(i) = 0;
      }
    }
  }

  // --------------------------------------------------------------
  // NOTE: following methods MUST be called under mSync locked!!!
  // --------------------------------------------------------------

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void extend(size_type num_to_reserve)
  {
    if (mStore.size() < num_to_reserve) {
      size_type i = (size_type)mStore.size();
      mStore.resize(num_to_reserve, NULL);
      for (; i < num_to_reserve; ++i)
        mPool.push_back(*(mStore[i] = allocateNode(i)));
    }
  }

  //Searches pool for non-used node, allocates a new one if necessary.
  //Returns NULL in case of maximum number of nodes is already reached!
  NodeHeader * allcNode(void)
  {
    if (mPool.empty()) {
      size_type newSz = (size_type)mStore.size() + 1;
      //check for size_type overloading
      if (newSz < (size_type)mStore.size())
        return NULL;
      extend(newSz);
    }
    return mPool.pop_front();
  }

  //Returns node with given unique index, allocating it if necessary.
  //Returns NULL in case of given index is out of possible range.
  NodeHeader * atNode(size_type obj_idx)
  {
    if (obj_idx >= mStore.size()) {
      size_type newSz = obj_idx + 1;
      //check for size_type overloading
      if (newSz < obj_idx)
        return NULL;
      extend(newSz); //allocates a bunch of new objects
    }
    NodeHeader * pNode = mStore[obj_idx];
    if (mPool.isLinked(*pNode))
      mPool.unlink(*pNode);
    return pNode;
  }

  //Releases (marks as unused) a given node
  void rlseNode(NodeHeader & p_node)
  {
    if (!mPool.isLinked(p_node)) {
#ifdef INTRUSIVE_POOL_DEBUG
      if (mLogger) {
        smsc_log_debug(mLogger, "%s: rlseNode[%u] numRefs = %u to pool", mLogId,
                       p_node.getNodeIdx(), p_node.getRefs());
      }
#endif /* INTRUSIVE_POOL_DEBUG */
      mPool.push_back(p_node);
      clearNode(p_node);
    }
#ifdef INTRUSIVE_POOL_DEBUG
    else if (mLogger) {
      smsc_log_error(mLogger, "%s: rlseNode[%u] numRefs = %u is already in pool", mLogId,
                     p_node.getNodeIdx(), p_node.getRefs());
    }
#endif /* INTRUSIVE_POOL_DEBUG */
  }

  // --------------------------------------------------------------
  // Exposed synchronized methods
  // --------------------------------------------------------------
  friend class NodeHeader;
  //Returns current number of refs to node
  size_type getNodeRefs(const NodeHeader & p_node) const
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
    return p_node.getRefs();
  }
  //Returns false if maximum number of refs is already reached
  bool refNode(const NodeHeader & p_node) const
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
#ifdef INTRUSIVE_POOL_DEBUG
    if (mLogger) {
      smsc_log_debug(mLogger, "%s: refNode[%u] numRefs = %u", mLogId,
                     p_node.getNodeIdx(), p_node.getRefs());
    }
#endif /* INTRUSIVE_POOL_DEBUG */
    return p_node.addRef();
  }
  //Returns true if given node is finally released.
  bool unrefNode(NodeHeader & p_node)
  {
    smsc::core::synchronization::MutexGuard  grd(mSync);
#ifdef INTRUSIVE_POOL_DEBUG
    if (mLogger) {
      smsc_log_debug(mLogger, "%s: unrefNode[%u] numRefs = %u", mLogId,
                     p_node.getNodeIdx(), p_node.getRefs());
    }
#endif /* INTRUSIVE_POOL_DEBUG */
    if (p_node.unRef()) {
      rlseNode(p_node);
      return true;
    }
    return false;
  }
};

} //buffers
} //core
} //smsc

#endif /* __CORE_BUFFERS_INTRUSIVE_POOL_CORE */

