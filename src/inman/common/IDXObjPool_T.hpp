/* ************************************************************************** *
 * Pool of unique objects (cann't be copied),  that additionally provides 
 * direct access to them using index asigned by pool to object while its creation.
  * ************************************************************************** */
#ifndef __SMSC_UTIL_INDEXED_OBJ_POOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_INDEXED_OBJ_POOL

#include <inttypes.h>
#include <vector>

#include "util/UniqueObjT.hpp"
#include "core/buffers/FifoList.hpp"

namespace smsc {
namespace util {

template <
  class _UniqueTArg                 //UniqueObj_T<> interface is required !!
, typename _IndexTypeArg// = unsigned
, bool     _EraseOnRlseArg //= true //object release mode: destroy or just mark as unused,
                                    //by default pooled objects are destroyed upon release.
>
class IDXObjPool_T {
public:
  typedef _IndexTypeArg size_type;
  typedef _UniqueTArg   PooledObj;

protected:
  class PooledObjImpl : public _UniqueTArg {
  public:
    explicit PooledObjImpl(_IndexTypeArg use_idx) : _UniqueTArg(use_idx)
    { }
    ~PooledObjImpl()
    { }
  };

  class IndexedNode : public smsc::core::buffers::FifoLink {
  private:
    IndexedNode(const IndexedNode & use_obj);
    IndexedNode & operator=(const IndexedNode & use_obj);

  protected:
    const size_type    mNodeIdx; //unique id of this element assigned by pool
    smsc::util::OptionalObj_T<PooledObjImpl, size_type>  mObj;

    friend class IDXObjPool_T;

    explicit IndexedNode(size_type use_idx)
      : smsc::core::buffers::FifoLink(), mNodeIdx(use_idx)
    { }

  public:
    //
    ~IndexedNode()
    { }

    bool empty(void) const { return mObj.empty(); }

    //Destroys contained object of this node
    void clear(void) { mObj.clear(); }

    //Returns pooled object iface, constructing it if necessary
    PooledObj * get(void)
    {
      if (mObj.empty())
        mObj.init(mNodeIdx); //UniqueObj_T<> interface
      return static_cast<PooledObj *>(mObj.get());
    }

    const PooledObj * get(void) const
    {
      return static_cast<const PooledObj *>(mObj.get());
    }
  };

  class NodeArray : public std::vector<IndexedNode *> {
  protected:
    using std::vector<IndexedNode *>::clear;

    NodeArray(const NodeArray & cp_obj);
    NodeArray & operator=(const NodeArray & cp_obj);

  public:
    NodeArray() : std::vector<IndexedNode *>()
    { }
    ~NodeArray()
    {
      clearAll();
    }

    void clearAll(void)
    {
      typename std::vector<IndexedNode *>::size_type i = 0;
      for (; i < this->size(); ++i) {
        if (this->at(i)) {
          delete this->at(i);
          this->at(i) = 0;
        }
      }
    }
  };

  typedef smsc::core::buffers::QueueOf_T<IndexedNode, size_type> NodeQueue;
  /* -- DATA members: -- */
  NodeArray   _store;   //store of all allocated nodes.
  NodeQueue   _pool;    //queue of unused nodes

  //Releases (marks as unused) a node with given unique index
  void rlseNode(size_type obj_idx)
  {
    if (obj_idx < _store.size()) {
      IndexedNode & pNode = *_store[obj_idx];
      if (!_pool.isLinked(pNode)) {
        _pool.push_back(pNode); //downcast IndexedNode to FifoLink
        if (_EraseOnRlseArg)
          pNode.clear();
      }
    }
  }

public:
  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  IDXObjPool_T()
  { }
  explicit IDXObjPool_T(size_type num_to_reserve)
  {
    reserve(num_to_reserve);
  }
  ~IDXObjPool_T()
  { }

  //Returns total number of allocated objects managed by pool
  size_type capacity(void) const { return (size_type)_store.size(); }

  //Returns number of currently used objects.
  size_type usage(void) const
  {
    return (size_type)_store.size() - _pool.size();
  }

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void reserve(size_type num_to_reserve)
  {
    if (_store.size() < num_to_reserve) {
      size_type i = (size_type)_store.size();
      _store.resize(num_to_reserve, NULL);
      for (; i < num_to_reserve; ++i)
        _pool.push_back(*(_store[i] = new IndexedNode(i))); //downcast IndexedNode to FifoLink
    }
  }

  //Allocates an object.
  //Returns NULL in case maximum number of objects is already reached!
  PooledObj * allcObj(void)
  {
    if (_pool.empty()) {
      size_type newSz = (size_type)_store.size() + 1;
      //check for size_type overloading
      if (newSz < (size_type)_store.size())
        return NULL;
      reserve(newSz);
    }
    //upcast FifoLink to IndexedNode
    IndexedNode * pNode = static_cast<IndexedNode *>(_pool.pop_front());
    return pNode->get();
  }

  //Releases (marks as unused) given pooled object
  void rlseObj(PooledObj & p_obj)
  {
    rlseNode(p_obj.getUIdx());
  }

  //Returns pooled object with given unique index that was previously allocated.
  //NOTE: Returns NULL if object wasn't allocated or was destroyed.
  const PooledObj * at(size_type obj_idx) const
  {
    if (obj_idx < _store.size()) {
      IndexedNode * pNode = _store[obj_idx];
      if (!_pool.isLinked(pNode) && pNode->get())
        return pNode->get();
    }
    return NULL;
  }

  //Returns pooled object with given unique index, allocating it if necessary.
  PooledObj * at(size_type obj_idx)
  {
    if (obj_idx >= _store.size()) {
      //check size_type overloading
      if ((obj_idx + 1) < obj_idx)
        return NULL;
      reserve(obj_idx + 1); //allocates new objects
    }

    IndexedNode * pNode = _store[obj_idx];
    if (_pool.isLinked(pNode))
      _pool.unlink(pNode);
    return pNode->get();
  }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_INDEXED_OBJ_POOL */

