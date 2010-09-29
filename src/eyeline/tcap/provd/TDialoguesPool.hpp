/* ************************************************************************** *
 * TCProvider: TCAP dialogues pool.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_POOL_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_POOL_HPP

#include "eyeline/util/NCObjPoolT.hpp"

#include "eyeline/tcap/provd/TDialogueFSM.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TDialoguesPool {
public:
  class TDialogueGuard {
  protected:
    TDialoguesPool *  _owner;
    ObjRefGuard       _refGrd;
    TDialogueFSM      _dlgFsm;

    friend class TDialoguesPool;
    void bindPool(TDialoguesPool * use_owner) { _owner = use_owner; }

  public:
    TDialogueGuard() : _owner(0), _dlgFsm(_refGrd)
    { }
    ~TDialogueGuard()
    { }

    //Returns pointer to designated object
    TDialogueFSM * get(void) { return &_dlgFsm; }
    //Returns pointer to designated object
    const TDialogueFSM * get(void) const { return &_dlgFsm; }

    //Returns number of references to designated object
    unsigned RefCount(void) const { return _refGrd.RefCount(); }
    //Increases references counter
    //NOTE: Returns zero in case of references counter overloading!
    unsigned AddRef(void) { return _refGrd.AddRef(); }
    //Decreases references counter, utilizes designated object
    //if no more references exist
    unsigned UnRef(void)
    {
      unsigned cnt = _refGrd.UnRef();
      if (!cnt)
        _owner->rlseDialogue(this);
      return cnt;
    }

    TDialogueFSM * operator->() { return get(); }
    const TDialogueFSM * operator->() const { return get(); }
  };

protected:
  typedef eyeline::util::NCOPool_T<TDialogueGuard, uint32_t> DlgGuardPool;

  DlgGuardPool _pool;

  friend class TDialoguesPool::TDialogueGuard;
  void rlseDialogue(TDialogueGuard * dlg_grd)
  {
    _pool.rlseObj(dlg_grd);
  }

public:
  TDialoguesPool() : _pool(true)
  { }
  ~TDialoguesPool()
  { }

  //Returns total number of allocated objects managed by pool
  uint32_t capacity(void) const { return _pool.capacity(); }

  //Ensures that pool capacity() isn't less then 'num_to_reserve'.
  void reserve(uint32_t num_to_reserve) { _pool.reserve(num_to_reserve); }

  //Returns NULL if maximum available number of dialogues is already allocated.
  TDialogueGuard * allcDialogue(void)
  {
    TDialogueGuard * dlgGrd = _pool.allcObj();
    if (dlgGrd)
      dlgGrd->bindPool(this);
    return dlgGrd;
  }
};

typedef TDialoguesPool::TDialogueGuard TDialogueGuard;

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_POOL_HPP */

