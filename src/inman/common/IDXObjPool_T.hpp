/* ************************************************************************** *
 * Accumulative pool of indexed objects, which are allowed to have no copying 
 * constructor defined. Memory for pooled objects is allocated once on heap 
 * and then just reused.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_INDEXED_OBJ_POOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_INDEXED_OBJ_POOL

#include <inttypes.h>
#include <vector>
#include "inman/common/FifoQueue.hpp"

namespace smsc {
namespace util {

//object with unique index assigned
template <
  class _TArg       //only default constructor is required !!
  , typename _IndexTypeArg = unsigned
>
class IndexedObj_T : public _TArg {
private:
  const _IndexTypeArg _unqIdx; //unique index of this object

  IndexedObj_T(const IndexedObj_T & cp_obj);
  IndexedObj_T & operator=(const IndexedObj_T & cp_obj);

public:
  explicit IndexedObj_T(_IndexTypeArg use_idx)
    : _TArg(), _unqIdx(use_idx)
  { }
  ~IndexedObj_T()
  { }

  _IndexTypeArg getIndex(void) const { return _unqIdx; }
};


//Accumulative pool of indexed objects, which have no copying constructor defined
template <
  class _IndexedTArg  //IndexedObj_T<> interface is required !!
  , typename _IndexTypeArg = unsigned
>
class IDXObjPool_T {
public:
  class PooledObj : public _IndexedTArg {
  protected:
    friend class IDXObjPool_T;

    explicit PooledObj(_IndexTypeArg use_idx) : _IndexedTArg(use_idx)
    { }
    ~PooledObj()
    { }
  };
  //
  typedef _IndexTypeArg size_type;

protected:
  class IndexedNode : public FifoLink {
  private:
    union {
      void *  _aligner;
      uint8_t _buf[sizeof(PooledObj)];
    } _mem;

    size_type    _idx; //unique id of this element assigned by pool
    PooledObj *  _pObj;

    IndexedNode(const IndexedNode & use_obj);
    IndexedNode & operator=(const IndexedNode & use_obj);

  protected:
    friend class IDXObjPool_T;

    explicit IndexedNode(size_type use_idx)
      : FifoLink(), _idx(use_idx), _pObj(NULL)
    {
      _mem._aligner = 0;
    }

  public:
    //
    ~IndexedNode()
    {
      clear();
    }

    bool empty(void) const { return _pObj == NULL; }

    const PooledObj * get(void) const { return _pObj; }

    PooledObj * get(void)
    {
      if (!_pObj)
        _pObj = new (_mem._buf) PooledObj(_idx);
      return _pObj;
    }

    void clear(void)
    {
      if (_pObj) {
        _pObj->~PooledObj();
        _pObj = NULL;
      }
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
        delete at(i); at(i) = 0;
      }
    }
  };

  /* -- DATA members: -- */
  const bool  _doErase; //object release mode: destroy or just mark as unused
  NodeArray   _store;   //store of all allocated nodes.
  FifoQueue   _pool;    //queue of unused nodes

  //Releases (marks as unused) a node with given unique index
  void rlseNode(size_type obj_idx)
  {
    if (obj_idx < _store.size()) {
      IndexedNode * pNode = _store[obj_idx];
      if (!_pool.isLinked(pNode)) {
        _pool.push_back(pNode); //downcast IndexedNode to FifoLink
        if (_doErase)
          pNode->clear();
      }
    }
  }

public:
  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  explicit IDXObjPool_T(bool erase_on_rlse = true)
    : _doErase(erase_on_rlse)
  { }
  explicit IDXObjPool_T(size_type num_to_reserve, bool erase_on_rlse = true)
    : _doErase(erase_on_rlse)
  {
    reserve(num_to_reserve);
  }
  ~IDXObjPool_T()
  { }

  //Returns total number of allocated objects managed by pool
  size_type capacity(void) const { return (size_type)_store.size(); }

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void reserve(size_type num_to_reserve)
  {
    if (_store.size() < num_to_reserve) {
      size_type i = (size_type)_store.size();
      _store.resize(num_to_reserve, NULL);
      for (; i < num_to_reserve; ++i)
        _pool.push_back(_store[i] = new IndexedNode(i)); //downcast IndexedNode to FifoLink
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
  void rlseObj(PooledObj * p_obj)
  {
    rlseNode(p_obj->getIndex());
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
    if (obj_idx >= _store.size())
      reserve(obj_idx + 1); //allocates new objects

    IndexedNode * pNode = _store[obj_idx];
    if (_pool.isLinked(pNode))
      _pool.unlink(pNode);
    return pNode->get();
  }

};

/* ******************************************************************* *
 * Pool of unique objects, which have no copying constructor
 *
 * NOTE: Pool maintains access to objects by unique index, that is
 *       assigned by pool while 1st object creation.
 * ******************************************************************* */
template <
  class _TArg  //only default constructor is required!
  , typename _SizeTypeArg = unsigned
>
class UNQObjPool_T : public IDXObjPool_T<IndexedObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg> {
public:
  using IDXObjPool_T<IndexedObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>::size_type;
  using IDXObjPool_T<IndexedObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>::PooledObj;

  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  explicit UNQObjPool_T(bool erase_on_rlse = true)
    : IDXObjPool_T<IndexedObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>(erase_on_rlse)
  { }
  explicit UNQObjPool_T(_SizeTypeArg num_to_reserve, bool erase_on_rlse = true)
    : IDXObjPool_T<IndexedObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>(num_to_reserve, erase_on_rlse)
  { }
  //
  ~UNQObjPool_T()
  { }
};


} //util
} //smsc

#endif /* __SMSC_UTIL_INDEXED_OBJ_POOL */

