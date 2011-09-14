/* ************************************************************************** *
 * Template of Multithread-safe Intrusive pool, that additionally provides                                    .
 * direct access to maintained objects.
 * Pooled objects access/releasing is guarded by reference counting pointers.
 * Memory for pooled objects is allocated once and then just reused.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_INTRUSIVE_POOL_T
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_INTRUSIVE_POOL_T

#include "core/buffers/IntrusivePoolCore.hpp"

namespace smsc {
namespace core {
namespace buffers {

// --------------------------------------------------------------------------
// Transforms any class to pooled object interface class.
// --------------------------------------------------------------------------
template <
  class _TArg //Only default constructor is required.
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

// ----------------------------------------------------------------------------
// Multithread-safe intrusive pool of arbitrary objects.
// ----------------------------------------------------------------------------
template <
  class     _TArg                     //Only default constructor is required.
, typename  _SizeTypeArg  = unsigned  ////MUST be an unsigned type,
                                      ///NOTE: implicitly restricts pool capacity
, bool      _EraseOnRlseArg = true    //object release mode: destroy or just mark as unused,
                                      //by default pooled objects are destroyed upon release.
>
class IntrusivePoolOf_T : public IDAPoolCoreAC_T<_SizeTypeArg> {
public:
  typedef _SizeTypeArg                  size_type;
  typedef PooledObj_T<_TArg>            PooledObj;
  typedef IDAPoolCoreAC_T<_SizeTypeArg> Base_T;

protected:
  class PooledObjImpl : public PooledObj {
  public:
    PooledObjImpl() : PooledObj()
    { }
    ~PooledObjImpl()
    { }
  };

  // Aggregates pool's node header and memory for pooled object implementation.
  class AnchoredNode : public Base_T::NodeHeader {
  private:
    AnchoredNode(const AnchoredNode & use_obj);
    AnchoredNode & operator=(const AnchoredNode & use_obj);

  protected:
    smsc::util::OptionalObj_T<PooledObjImpl>  mObj;

  public:
    AnchoredNode(Base_T & use_pool, size_type use_idx)
      : Base_T::NodeHeader(use_pool, use_idx)
    { }
    //
    ~AnchoredNode()
    { }

    //Destroys contained object of this node
    void clear(void) { mObj.clear(); }

    //Returns pooled object iface, constructing it if necessary
    PooledObj * getIface(void)
    {
      if (mObj.empty())
        mObj.init();
      return static_cast<PooledObj *>(mObj.get());
    }
  };

public:
  // Pooled object reference: guards pooled object destruction,
  // exposes pooled object interface.
  class ObjRef : public Base_T::ObjGuard {
  private:
    using Base_T::ObjGuard::operator=; //prevent usage of ObjGuard created by pool of another objects.

  protected:
    AnchoredNode * fpNode(void) const
    {
      return static_cast<AnchoredNode *>(this->mpNodeHdr);
    }

    friend class IntrusivePoolOf_T;
    //NOTE: constructor MUST BE called under referenced pool sync locked !!!
    explicit ObjRef(typename Base_T::NodeHeader * use_node)
      : Base_T::ObjGuard(use_node)
    { }

  public:
    ObjRef() : Base_T::ObjGuard()
    { }
    ObjRef(const ObjRef & cp_obj) : Base_T::ObjGuard(cp_obj)
    { }
    //
    ~ObjRef()
    { }

    PooledObj * get(void) const { return fpNode() ? fpNode()->getIface() : 0; }

    PooledObj * operator->(void) const { return get(); }

    PooledObj & operator*(void) const { return *get(); }

    ObjRef & operator= (const ObjRef & cp_obj)
    {
      if (this != &cp_obj) {
        *(typename Base_T::ObjGuard *)this = cp_obj;
      }
      return *this;
    }

  #ifdef INTRUSIVE_POOL_DEBUG
    void logThis(Logger * use_log, const char * log_id) const
    {
      fpNode()->logThis(use_log, log_id);
    }
  #endif /* INTRUSIVE_POOL_DEBUG */
  };


  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  IntrusivePoolOf_T() : IDAPoolCoreAC_T<_SizeTypeArg>()
  { }
  explicit IntrusivePoolOf_T(size_type num_to_reserve)
    : IDAPoolCoreAC_T<_SizeTypeArg>()
  {
    reserve(num_to_reserve);
  }
  virtual ~IntrusivePoolOf_T()
  {
    this->destroyAll();
  }

  //Gets an unsed object, allocates a new one if necesary.
  //Returns empty ObjRef in case of maximum number of objects is already reached!
  ObjRef allcObj(void)
  {
    smsc::core::synchronization::MutexGuard  grd(this->poolSync());
    return ObjRef(this->allcNode());
  }

  //Returns pooled object with given unique index, allocating it if necessary.
  //Returns empty ObjRef in case of given index is out of possible range.
  ObjRef atObj(size_type obj_idx)
  {
    smsc::core::synchronization::MutexGuard  grd(this->poolSync());
    return ObjRef(this->atNode(obj_idx));
  }

protected:
  // --------------------------------------------------
  // -- IDAPoolCoreAC_T<> interface methods
  // --------------------------------------------------
  virtual typename Base_T::NodeHeader * allocateNode(size_type use_idx)
  {
    return static_cast<typename Base_T::NodeHeader *>(new AnchoredNode(*this, use_idx));
  }
  virtual void clearNode(typename Base_T::NodeHeader & p_node)
  {
    if (_EraseOnRlseArg)
      static_cast<AnchoredNode &>(p_node).clear();
  }
  virtual void destroyNode(typename Base_T::NodeHeader & p_node)
  {
    delete static_cast<AnchoredNode *>(&p_node);
  }
};

} //buffers
} //core
} //smsc

#endif /* __CORE_BUFFERS_INTRUSIVE_POOL_T */

