#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TDlgFSMRegistry.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using smsc::core::synchronization::MutexGuard;

/* ************************************************************************* *
 * class TDlgFSMRegistry implementation
 * ************************************************************************* */
TDialogueId
  TDlgFSMRegistry::getNextDlgId(void) /*throw(std::exception)*/
{
  static const unsigned _MAX_DLG_ID_ATTEMPTS = 6;
  unsigned    attNum = 0;
  TDialogueId dlgId = 0;

  do {
    if ((++attNum) > _MAX_DLG_ID_ATTEMPTS)
      throw smsc::util::Exception("tcap::provd::TDlgFSMRegistry: cann't allocate dialogueId");
    if ((dlgId = _nextDlgId++) == _TDLG_ID_RESERVED)
      dlgId = _nextDlgId++;
  } while (_dlgMap.find(dlgId) != _dlgMap.end());
  return dlgId;
}

TDlgRefPtr_t
  TDlgFSMRegistry::allcDlgFSM(TDialogueId & dlg_id) /*throw(std::exception)*/
{
  MutexGuard synchronize(_sync);

  TDlgRefPtr_t  pDlg(_dlgPool.allcDialogue());
  if (!pDlg.get())
    throw smsc::util::Exception("tcap::provd::TDlgFSMRegistry: cann't allocate dialogue");

  dlg_id = getNextDlgId(); //throws
  _dlgMap.insert(TDialoguesMap::value_type(dlg_id, pDlg));
  return pDlg;
}

  
//Creates dialog initiated by local point
TDialogueId
  TDlgFSMRegistry::createTDlgFSM(const TDlgLocalConfig & dlg_cfg,
                                 TDlgCoordinatorIface * dlg_mgr) /*throw(std::exception)*/
{
  TDialogueId   dlgId;
  TDlgRefPtr_t  pDlg = allcDlgFSM(dlgId); //throws

  pDlg->init(dlgId, dlg_cfg, dlg_mgr);
  return dlgId;
}

//Creates dialog initiated by remote point
TDialogueId
  TDlgFSMRegistry::createTDlgFSM(const TDlgRemoteConfig & dlg_cfg,
                                 TDlgCoordinatorIface * dlg_mgr) /*throw(std::exception)*/
{
  TDialogueId   dlgId;
  TDlgRefPtr_t  pDlg = allcDlgFSM(dlgId);

  pDlg->init(dlgId, dlg_cfg, dlg_mgr);
  return dlgId;
}


TDlgRefPtr_t
  TDlgFSMRegistry::getTDlgFSM(TDialogueId dlg_id) const /*throw()*/
{
  MutexGuard synchronize(_sync);
  TDialoguesMap::const_iterator it = _dlgMap.find(dlg_id);
  return (it == _dlgMap.end()) ? TDlgRefPtr_t() : TDlgRefPtr_t(it->second);
}

void TDlgFSMRegistry::releaseTDlgFSM(TDialogueId dlg_id) /*throw()*/
{
  MutexGuard synchronize(_sync);
  TDialoguesMap::iterator it = _dlgMap.find(dlg_id);
  if (it != _dlgMap.end())
    _dlgMap.erase(it);
}

}}}

