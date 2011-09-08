/* ************************************************************************* *
 * Two variants of pool of objects, which have no copying constructor defined. 
 * ************************************************************************* */
#ifndef __EYELINE_UTIL_NONCOPYING_OBJ_POOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_UTIL_NONCOPYING_OBJ_POOL

#include <vector>
#include "eyeline/util/FifoQueue.hpp"

namespace eyeline {
namespace util {

/* ******************************************************************* *
 * Pool, that maintains access to objects by unique index, that is
 *       assigned by pool while object creation,
 *  
 * NOTE: Object class must have a following constructor: 
 *        explicit _IndexedTArg(_SizeTypeArg use_idx);
 * ******************************************************************* */
template <
  class _IndexedTArg                 // : public IndexedNCObj_T<>
  , typename _SizeTypeArg = unsigned //restricts capacity of pool
>
class NCOIndexedPool_T {
public:
  typedef _SizeTypeArg  size_type;

protected:
  class IndexedNode : public FifoLink {
  private:
    union {
      void *        _aligner;
      unsigned char _buf[sizeof(_IndexedTArg)];
    } _mem;

    size_type       _idx; //unique id of this element assigned by pool
    _IndexedTArg *  _pObj;

    IndexedNode(const IndexedNode & use_obj);
    IndexedNode & operator=(const IndexedNode & use_obj);

  protected:
    friend class NCOIndexedPool_T;

    explicit IndexedNode(size_type  use_idx) 
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

    size_type getIdx(void) const { return _idx; }

    bool empty(void) const { return _pObj == NULL; }

    const _IndexedTArg  * get(void) const { return _pObj; }

    _IndexedTArg  * get(void)
    {
      if (!_pObj)
        _pObj = new (_mem._buf) _IndexedTArg(_idx);
      return _pObj;
    }

    void clear(void)
    {
      if (_pObj) {
        _pObj->~_IndexedTArg();
        _pObj = NULL;
      }
    }
  };

  class NodeArray : public std::vector<IndexedNode *> {
  protected:
    NodeArray(const NodeArray & cp_obj);
    NodeArray & operator=(const NodeArray & cp_obj);

  public:
    NodeArray() : std::vector<IndexedNode *>()
    { }
    ~NodeArray()
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
  explicit NCOIndexedPool_T(bool erase_on_rlse = true)
    : _doErase(erase_on_rlse)
  { }
  explicit NCOIndexedPool_T(size_type num_to_reserve, bool erase_on_rlse = true)
    : _doErase(erase_on_rlse)
  {
    reserve(num_to_reserve);
  }
  ~NCOIndexedPool_T()
  { }

  //Returns total number of allocated objects managed by pool
  size_type capacity(void) const { return (size_type)_store.size(); }

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void reserve(size_type num_to_reserve)
  {
    if (_store.capacity() < num_to_reserve) {
      _store.reserve(num_to_reserve);
      memset(&_store[_store.size()], 0, sizeof(IndexedNode*)*(_store.capacity() - _store.size()));
    }
    if (_store.size() < num_to_reserve) {
      for (size_type i = (size_type)_store.size(); i < num_to_reserve; ++i)
        _pool.push_back(_store[i] = new IndexedNode(i));
    }
  }

  //Allocates an object.
  //Returns NULL in case maximum number of objects is already reached!
  _IndexedTArg * allcObj(void)
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
  void rlseObj(_IndexedTArg * p_obj)
  {
    rlseNode(p_obj->getIdx());
  }

  //Returns pooled object with given unique index that was previously allocated.
  //NOTE: Returns NULL if object wasn't allocated or was destroyed.
  _IndexedTArg * at(size_type obj_idx) const
  {
    if (obj_idx < _store.size()) {
      IndexedNode * pNode = _store[obj_idx];
      if (!_pool.isLinked(pNode) && pNode->get())
        return pNode;
    }
    return NULL;
  }

  //Returns pooled object with given unique index, allocating it if necessary.
  _IndexedTArg * at(size_type obj_idx)
  {
    if (obj_idx >= _store.size())
      _store.reserve(obj_idx + 1);

    IndexedNode * pNode = _store[obj_idx];
    if (_pool.isLinked(pNode))
      _pool.unlink(pNode);
    return pNode->get();
  }
};

/* ******************************************************************* *
 * Ordinary pool (no access to objects by index).
 * ******************************************************************* */

// Helper for treating 'non-copying' object as indexed one.
template <
  class _TArg             //only default constructor is required!
  , typename _IndexTypeArg = unsigned
>
class IndexedNCObj_T : public _TArg {
private:
  const _IndexTypeArg _idx; //unique index of this object

  IndexedNCObj_T(const IndexedNCObj_T & cp_obj);
  IndexedNCObj_T & operator=(const IndexedNCObj_T & cp_obj);

public:
  explicit IndexedNCObj_T(_IndexTypeArg use_idx) : _TArg(), _idx(use_idx)
  { }
  ~IndexedNCObj_T()
  { }

  _IndexTypeArg   getIdx(void) const { return _idx; }
};

template <
  class _TArg                         //only default constructor is required!
  , typename _SizeTypeArg = unsigned  //restricts capacity of pool
>
class NCOPool_T : protected NCOIndexedPool_T<IndexedNCObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg> {
protected:
  typedef IndexedNCObj_T<_TArg, _SizeTypeArg> IndexedObj;

public:
  typedef _SizeTypeArg  size_type;

  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  explicit NCOPool_T(bool erase_on_rlse = true)
    : NCOIndexedPool_T<IndexedNCObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>(erase_on_rlse)
  { }
  NCOPool_T(size_type num_to_reserve, bool erase_on_rlse = true)
    : NCOIndexedPool_T<IndexedNCObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>(num_to_reserve, erase_on_rlse)
  { }
  //
  ~NCOPool_T()
  { }

  using NCOIndexedPool_T<IndexedNCObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>::capacity;
  using NCOIndexedPool_T<IndexedNCObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>::reserve;

  //Allocates an object.
  //Returns NULL in case maximum number of objects is already reached!
  _TArg * allcObj(void)
  {
    IndexedObj * pooledObj = NCOIndexedPool_T<IndexedNCObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>::allcObj();
    return static_cast<_TArg *>(pooledObj);
  }

  //Releases (marks as unused) given pooled object
  void rlseObj(_TArg * p_obj)
  {
    IndexedObj * pooledObj = static_cast<IndexedObj *>(p_obj);
    NCOIndexedPool_T<IndexedNCObj_T<_TArg, _SizeTypeArg>, _SizeTypeArg>::rlseObj(pooledObj);
  }
};

} //util
} //eyeline

#endif /* __EYELINE_UTIL_NONCOPYING_OBJ_POOL */

