/* ************************************************************************** *
 * Variants of multithreadsafe accumulative pools of objects.
 * Pooled objects access/releasing is guarded by reference counting pointers.
 * Only pooled objects interface is exposed.
 * Memory for pooled objects is allocated once on heap and then just reused, 
 * providing fast objects allocation/releasing.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_GUARDED_OBJECTS_POOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_GUARDED_OBJECTS_POOL

#include "inman/common/GrdIfacePool_T.hpp"

namespace smsc {
namespace util {

template <
  class _TArg                       // must have default constructor defined.
, typename _SizeTypeArg = unsigned  //implicitly restricts pool capacity
> 
class GrdObjPool_T : public GrdIfacePoolAC_T<PooledObj_T<_TArg>, _SizeTypeArg> {
public:
  typedef PooledObj_T<_TArg>  PooledObj;
  typedef _SizeTypeArg        size_type;

  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  explicit GrdObjPool_T(bool erase_on_rlse = true)
    : GrdIfacePoolAC_T<PooledObj, _SizeTypeArg>(erase_on_rlse)
  { }
  explicit GrdObjPool_T(size_type num_to_reserve, bool erase_on_rlse = true)
    : GrdIfacePoolAC_T<PooledObj, _SizeTypeArg>(erase_on_rlse)
  {
    reserve(num_to_reserve);
  }
  virtual ~GrdObjPool_T()
  { }

protected:
  class PooledObjImpl : public PooledObj {
  public:
    PooledObjImpl() : PooledObj()
    { }
    ~PooledObjImpl()
    { }
  };
  typedef GrdIfacePoolAC_T<PooledObj, _SizeTypeArg> Base_T;
  typedef typename Base_T::template AnchoredNode_T<PooledObjImpl> AnchoredNode;

  // ------------------------------------------------
  // -- GrdIfacePoolAC_T interface methods
  // ------------------------------------------------
  virtual typename Base_T::PoolAnchorAC * allocateNode(size_type node_idx)
  {
    return new AnchoredNode(*this, node_idx);
  }
};


template <
  class _IfaceArg     //MUST have no public destructor !!!
, class _IfaceImplArg // : public _IfaceArg {}, may have only default constructor defined.
, typename _SizeTypeArg = unsigned  //implicitly restricts pool capacity 
>
class GrdIfaceImplPool_T : public GrdIfacePoolAC_T<_IfaceArg, _SizeTypeArg> {
public:
  typedef _SizeTypeArg size_type;

  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  explicit GrdIfaceImplPool_T(bool erase_on_rlse = true)
    : GrdIfacePoolAC_T<_IfaceArg, _SizeTypeArg>(erase_on_rlse)
  { }
  explicit GrdIfaceImplPool_T(size_type num_to_reserve, bool erase_on_rlse = true)
    : GrdIfacePoolAC_T<_IfaceArg, _SizeTypeArg>(erase_on_rlse)
  {
    reserve(num_to_reserve);
  }
  virtual ~GrdIfaceImplPool_T()
  { }

protected:
  typedef GrdIfacePoolAC_T<_IfaceArg, _SizeTypeArg> Base_T;
  typedef typename Base_T::template AnchoredNode_T<_IfaceImplArg> AnchoredNode;

  // ------------------------------------------------
  // -- GrdIfacePoolAC_T interface methods
  // ------------------------------------------------
  virtual typename Base_T::PoolAnchorAC * allocateNode(size_type node_idx)
  {
    return new AnchoredNode(*this, node_idx);
  }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_GUARDED_OBJECTS_POOL */

