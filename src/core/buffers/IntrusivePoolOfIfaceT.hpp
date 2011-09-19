/* ************************************************************************** *
 * Template of Multithread-safe Intrusive pool of objects, which implement some                                                               .
 * interface. Pool provides additionally direct access to maintained objects.
 * Pooled objects access/releasing is guarded by reference counting pointers.
 * Memory for pooled objects is allocated once and then just reused.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_INTRUSIVE_POOL_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_INTRUSIVE_POOL_DEFS

#include "core/buffers/IntrusivePoolCore.hpp"

namespace smsc {
namespace core {
namespace buffers {

// --------------------------------------------------------------------------
// Basic class of intrusive pool's node containing pooled object, that
// implements specified interface.
// --------------------------------------------------------------------------
template <
  class _IfaceArg           //Interface implemented by Pooled objects,
                            //MUST have no public destructor !!!
, typename _SizeTypeArg     //MUST be an unsigned type,
              = unsigned    //NOTE: implicitly restricts pool capacity
>
class IDAPoolNodeIface_T : public IDAPoolCoreAC_T<_SizeTypeArg>::NodeHeader {
public:
  // ----------------------------------------
  // -- IDAPoolNodeIface_T<> interface methods
  // ----------------------------------------
  //Returns iface  pooled object, constructing it if necessary
  virtual _IfaceArg * getIface(void) = 0;

protected:
  IDAPoolNodeIface_T(IDAPoolCoreAC_T<_SizeTypeArg> & use_pool, _SizeTypeArg use_idx)
    : IDAPoolCoreAC_T<_SizeTypeArg>::NodeHeader(use_pool, use_idx)
  { }
  virtual ~IDAPoolNodeIface_T()
  { }

private:
  IDAPoolNodeIface_T(const IDAPoolNodeIface_T & cp_obj);
  IDAPoolNodeIface_T & operator=(const IDAPoolNodeIface_T & cp_obj);
};

// --------------------------------------------------------------------------
// Pooled object reference: guards pooled object destruction,
// exposes pooled object interface.
// --------------------------------------------------------------------------
template <
  class _IfaceArg           //Interface implemented by Pooled objects,
                            //MUST have no public destructor !!!
, typename _SizeTypeArg     //MUST be an unsigned type,
              = unsigned    //NOTE: implicitly restricts pool capacity
>
class IDAPoolObjRef_T : public IDAPoolCoreAC_T<_SizeTypeArg>::ObjGuard {
private:
  typedef IDAPoolNodeIface_T<_IfaceArg, _SizeTypeArg> RefNodeIface;
  typedef typename IDAPoolCoreAC_T<_SizeTypeArg>::ObjGuard Base_T;
  using Base_T::operator=; //prevent usage of ObjGuard created by pool of another objects.

protected:
  RefNodeIface * fpNode(void) const
  {
    return static_cast<RefNodeIface *>(this->mpNodeHdr);
  }

  //NOTE: constructor MUST BE called under referenced pool sync locked !!!
  explicit IDAPoolObjRef_T(RefNodeIface * use_node) : Base_T(use_node)
  { }

public:
  IDAPoolObjRef_T() : Base_T()
  { }
  IDAPoolObjRef_T(const IDAPoolObjRef_T & cp_obj) : Base_T(cp_obj)
  { }
  //
  ~IDAPoolObjRef_T()
  { }

  _IfaceArg * get(void) const { return fpNode() ? fpNode()->getIface() : 0; }

  _IfaceArg * operator->(void) const { return get(); }

  _IfaceArg & operator*(void) const { return *get(); }

  IDAPoolObjRef_T & operator= (const IDAPoolObjRef_T & cp_obj)
  {
    if (this != &cp_obj) {
      *(Base_T*)this = cp_obj;
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

// ------------------------------------------------------------------------------
// Basic abstract class of intrusive pool of objects, which implement specified
// interface.
// ------------------------------------------------------------------------------
template <
  class _IfaceArg           //Interface implemented by Pooled objects,
                            //MUST have no public destructor !!!
, typename _SizeTypeArg     //MUST be an unsigned type,
              = unsigned    //NOTE: implicitly restricts pool capacity
>
class IntrusivePoolAC_T : public IDAPoolCoreAC_T<_SizeTypeArg> {
public:
  typedef _SizeTypeArg  size_type;
  typedef IDAPoolObjRef_T<_IfaceArg, _SizeTypeArg> ObjRef;

  virtual ~IntrusivePoolAC_T()
  {
    //NOTE: successor's destructor MUST call
    //  this->destroyAll();
  }

  //Gets an unsed object, allocates a new one if necesary.
  //Returns empty ObjRef in case of maximum number of objects is already reached!
  ObjRef allcObj(void)
  {
    ObjRefComposer  pObj;
    {
      smsc::core::synchronization::MutexGuard  grd(this->poolSync());
      pObj.initRef(this->allcNode());
    }
    return pObj;
  }

  //Returns pooled object with given unique index, allocating it if necessary.
  //Returns empty ObjRef in case of given index is out of possible range.
  ObjRef atObj(size_type obj_idx)
  {
    ObjRefComposer  pObj;
    {
      smsc::core::synchronization::MutexGuard  grd(this->poolSync());
      pObj.initRef(this->atNode(obj_idx));
    }
    return pObj;
  }

protected:
  typedef IDAPoolNodeIface_T<_IfaceArg, _SizeTypeArg> NodeIface;

  class ObjRefComposer : public ObjRef {
  public:
    ObjRefComposer() : ObjRef()
    { }
    ~ObjRefComposer()
    { }

    using ObjRef::initRef;
  };

  //
  IntrusivePoolAC_T() : IDAPoolCoreAC_T<_SizeTypeArg>()
  { }
};

// ------------------------------------------------------------------------------
// Multithread-safe intrusive pool of objects, which implement specified
// interface.
// ------------------------------------------------------------------------------
template <
  class _IfaceArg                   //Interface implemented by Pooled objects,
                                    //MUST have no public destructor !!!
, class _IfaceImplArg               // : public _IfaceArg, only default constructor is required.
, typename  _SizeTypeArg = unsigned //implicitly restricts pool capacity 
, bool      _EraseOnRlseArg = true  //object release mode: destroy or just mark as unused,
                                    //by default pooled objects are destroyed upon release.
>
class IntrusivePoolOfIfaceImpl_T : public IntrusivePoolAC_T<_IfaceArg, _SizeTypeArg> {
public:
  typedef _SizeTypeArg size_type;

  //NOTE: by default pooled objects are destroyed upon release.
  //      Set 'erase_on_rlse' to false if pooled objects are reusable.
  IntrusivePoolOfIfaceImpl_T() : IntrusivePoolAC_T<_IfaceArg, _SizeTypeArg>()
  { }
  explicit IntrusivePoolOfIfaceImpl_T(size_type num_to_reserve)
    : IntrusivePoolAC_T<_IfaceArg, _SizeTypeArg>()
  {
    reserve(num_to_reserve);
  }
  virtual ~IntrusivePoolOfIfaceImpl_T()
  {
    this->destroyAll();
  }

protected:
   // Aggregates pool's node header and memory for pooled object implementation.
  class AnchoredNode : public IDAPoolNodeIface_T<_IfaceArg, _SizeTypeArg> {
  private:
    AnchoredNode(const AnchoredNode & use_obj);
    AnchoredNode & operator=(const AnchoredNode & use_obj);

  protected:
    smsc::util::OptionalObj_T<_IfaceImplArg>  mObj;

  public:
    AnchoredNode(IDAPoolCoreAC_T<_SizeTypeArg> & use_pool, _SizeTypeArg use_idx)
      : IDAPoolNodeIface_T<_IfaceArg, _SizeTypeArg>(use_pool, use_idx)
    { }
    //
    virtual ~AnchoredNode()
    { }

    //Destroys contained object of this node
    void clear(void) { mObj.clear(); }
    // --------------------------------------
    // -- IntrPoolNodeAC_T<> interface method
    // --------------------------------------
    //Returns pooled object iface, constructing it if necessary
    virtual _IfaceArg * getIface(void)
    {
      if (mObj.empty())
        mObj.init();
      return static_cast<_IfaceArg *>(mObj.get());
    }
  };
  
  typedef IntrusivePoolAC_T<_IfaceArg, _SizeTypeArg> Base_T;
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

#endif /* __CORE_BUFFERS_INTRUSIVE_POOL_DEFS */

