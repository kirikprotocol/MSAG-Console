/* ************************************************************************** *
 * TCProvider: registry of TCAP dialogues FSM objects.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_FSM_REGISTRY_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_FSM_REGISTRY_HPP

#include <map>

#include "eyeline/tcap/provd/TDialoguesPool.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TDlgFSMRegistry {
public:
  class TDlgRefPtr {
  private:
    TDialogueGuard * _refGrd;

    void unref(void)
    {
      if (_refGrd && !_refGrd->UnRef())
        _refGrd = NULL;
    }

  protected:
    friend class TDlgFSMRegistry;
    TDlgRefPtr(TDialogueGuard * fsm_grd) : _refGrd(fsm_grd)
    {
      if (_refGrd)
        _refGrd->AddRef();
    }

  public:
    TDlgRefPtr() : _refGrd(NULL)
    { }
    TDlgRefPtr(const TDlgRefPtr & fsm_ptr) : _refGrd(fsm_ptr._refGrd)
    {
      if (_refGrd)
        _refGrd->AddRef();
    }
    ~TDlgRefPtr()
    {
      unref();
    }

    TDialogueFSM * get(void) { return _refGrd ? _refGrd->get() : NULL; }
    const TDialogueFSM * get(void) const { return _refGrd ? _refGrd->get() : NULL; }

    TDlgRefPtr & operator=(TDialogueGuard * fsm_grd)
    {
      unref();
      (_refGrd = fsm_grd)->AddRef();
      return *this;
    }

    TDlgRefPtr & operator=(const TDlgRefPtr & fsm_ptr)
    {
      return (this != &fsm_ptr) ? this->operator=(fsm_ptr._refGrd) : *this;
    }

    bool operator==(const TDlgRefPtr & fsm_ptr) const
    {
      return (_refGrd == fsm_ptr._refGrd);
    }

    //NOTE: it's a caller responsibility to check for NULL
    //condition prior to accessing the designated object
    const TDialogueFSM & operator*() const { return *get(); }
    const TDialogueFSM * operator->() const { return get(); }

    TDialogueFSM & operator*() { return *get(); }
    TDialogueFSM * operator->() { return get(); }
  };

  //DialogueId reserved for interaction with remote peers by TMessage
  //doesn't associated with any established dialogue (T_PAbort sending, etc)
  static const TDialogueId  _TDLG_ID_RESERVED = 0;

  TDlgFSMRegistry() : _nextDlgId(0)
  { }
  ~TDlgFSMRegistry()
  { }

  //Creates TCAP dialogue initiated by local point
  TDialogueId createTDlgFSM(const TDlgLocalConfig & dlg_cfg,
                            TDlgCoordinatorIface * dlg_mgr) /*throw(std::exception)*/;
  //Creates TCAP dialogue initiated by remote point
  TDialogueId createTDlgFSM(const TDlgRemoteConfig & dlg_cfg,
                            TDlgCoordinatorIface * dlg_mgr) /*throw(std::exception)*/;

  //Searches registry for TCAP dialogue by its DialogueId
  //Return empty refPtr if no dialogue found.
  TDlgRefPtr getTDlgFSM(TDialogueId dlg_id) const /*throw()*/;

  //Removes TCAP dialogue with given DialogueId from set of active dialogue.
  //NOTE: dialogue object is preserved in background while it's referenced by
  //      other active entities (Timers, etc ..)
  void releaseTDlgFSM(TDialogueId dlg_id) /*throw()*/;

protected:
  typedef std::map<TDialogueId, TDlgRefPtr> TDialoguesMap;

  mutable Mutex   _sync;
  TDialogueId     _nextDlgId;   //NOTE: dlgId = localTrId
  TDialoguesMap   _dlgMap;      //map of currently active dialogues
  TDialoguesPool  _dlgPool;     //pool of dialogues (NOTE: dialogue object
                                //is preserved until last reference is freed)

  //Throws if dialogId cann't be allocated
  TDialogueId getNextDlgId(void) /*throw(std::exception)*/;
  //Throws if dialogue cann't be allocated
  TDlgRefPtr  allcDlgFSM(TDialogueId & dlg_id) /*throw(std::exception)*/;
};

typedef TDlgFSMRegistry::TDlgRefPtr TDlgRefPtr_t;

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_FSM_REGISTRY_HPP */

