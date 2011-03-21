/* ************************************************************************* *
 * Primitive MAP dialog basic class (performs a single ROS request).
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_DIALOG__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_MAP_DIALOG__

#include "util/URCdb/URCRegistry.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "inman/common/RefCountersMaskT.hpp"
#include "inman/inap/dialog.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::util::RCHash;
using smsc::core::synchronization::EventMonitor;

class ObjFinalizerIface {
protected:
  virtual ~ObjFinalizerIface() //forbid interface destruction
  { }

public:
  //Dialog allocation status
  enum AllcStatus_e { objActive = 0, objFinalized, objDestroyed };

  //
  virtual AllcStatus_e finalizeObj(void) = 0;
};
typedef ObjFinalizerIface::AllcStatus_e ObjAllcStatus_e;


class MapDlgUserIface {
protected:
  virtual ~MapDlgUserIface() //forbid interface destruction
  { }

public:
  //if err_code != 0, no result has been got from MAP service,
  //NOTE: dialog user may destroy reporting MAP dialog from this callback in two ways:
  //   1) by calling MapDialogAC::releaseThis() if dialog is allocated on heap
  //   2) by calling ObjFinalizerIface::finalizeObj() and then MapDialogAC destructor
  virtual ObjAllcStatus_e onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code = 0) = 0;
  //
  virtual void Awake(void) = 0;
};


//Primitive MAP dialog: initiates a single ROS request to remote side,
//awiats result, in case of success reports it to MAP User.
class MapDialogAC : protected TCDialogUserITF, protected ObjFinalizerIface {
public:
  static const size_t _maxIdentSZ = 32;

  //NOTE: destructor blocks until last reference to this object is reset.
  virtual ~MapDialogAC();

  //Marks dialog as released. Object will be destroyed by thread that will
  //reset the last reference. if no references present this method immediately
  //calls destructor.
  //NOTE: this method is applicable only for objects allocated on heap.
  //NOTE: calling this method is mutually exclusive with explicit destructor call.
  ObjAllcStatus_e releaseThis(void);

  //Ends/Aborts MAP dialog. Releases TCAP dialog if requested.
  //NOTE: method may block on TCAP dialog releasing.
  void endDialog(bool do_release = true);

  //Attempts to unbind MAP User.
  //Returns true on succsess, or false if this object has established reference
  //to MAP User (waits for its mutex). In that case, MAP User should wait on
  //its mutex and then repeat this call.
  bool unbindUser(void);

protected:
  explicit MapDialogAC(const char * use_ident, Logger * use_log = NULL);

  // -----------------------------------------
  // -- ObjFinalizerIface interface methods:
  // -----------------------------------------
  //Unrefs and unlocks result handler
  virtual ObjAllcStatus_e finalizeObj(void);

  // -----------------------------------------
  // -- MapDialogAC interface methods:
  // -----------------------------------------
  //Releases TC dialog object
  virtual void  rlseTCDialog(void) = 0;

  friend class smsc::inman::inap::Dialog;
  // -----------------------------------------
  // -- TCDialogUserITF interface methods:
  // -----------------------------------------
  virtual void Awake(void) { _sync.notify(); }
  //DialogListener methods
  virtual void onDialogInvoke(Invoke * op, bool lastComp);
  virtual void onDialogContinue(bool compPresent);
  virtual void onDialogPAbort(uint8_t abortCause);
  virtual void onDialogREnd(bool compPresent);
  virtual void onDialogUAbort(uint16_t abortInfo_len, uint8_t * pAbortInfo,
                              uint16_t userInfo_len, uint8_t * pUserInfo);
  virtual void onDialogNotice(uint8_t reportCause,
                              TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                              uint8_t invId = 0, uint8_t opCode = 0);

  //InvokeListener methods
  virtual void onInvokeResultNL(InvokeRFP pInv, TcapEntity * res) = 0;
  virtual void onInvokeResult(InvokeRFP pInv, TcapEntity * res) = 0;
  virtual void onInvokeError(InvokeRFP pInv, TcapEntity * resE);
  virtual void onInvokeLCancel(InvokeRFP pInv);

private:
  volatile  bool  _delThis;  //delete 'this' on last reference reset

protected:
  enum ROSState_e { operNone = 0, operInited = 1, operFailed = 2, operDone = 3 };

  union ContractState_u {
    unsigned char value;
    struct  {
      unsigned char ctrInited : 2;
      unsigned char ctrResulted : 2; // operInited - ResultNL, operDone - ResultL
      unsigned char ctrAborted : 1;
      unsigned char ctrFinished : 1;
    } s;
  };

  //Ids of external entities, which may refer this object
  enum RefEntities_e {
      refIdItself = 0x00
    , refIdHandler = 0x01
    , refIdTCDialog = 0x02
  };

  typedef smsc::util::RefCountersMask_T<3, 2> RefCountersMask;
  typedef smsc::core::synchronization::MTRefWrapper_T<MapDlgUserIface>
    MapUserRef;

  mutable EventMonitor  _sync;
  ContractState_u   _ctrState;
  RefCountersMask   _thisRefs;

  const char *      _logPfx;
  char              _logId[_maxIdentSZ + sizeof("[%u:%Xh]") + 2*sizeof(unsigned)*3 + 1];

  MapUserRef        _resHdl;
  Logger *          _logger;
  Dialog *          _tcDlg;    //TCAP dialog

  //Sets MAP user for this dialog
  void bindUser(MapDlgUserIface & res_hdl);
  //Binds TC Dialog
  void bindTCDialog(uint16_t timeout_sec = 0);
  //ends TC dialog
  void endTCDialog(void);
  //ends TC dialog and releases TC dialog object
  void unbindTCDialog(void);
  //Locks reference to MAP user.
  //Returns false if result handler is not set.
  bool doRefUser(void);
  //Unlocks reference to MAP user.
  void doUnrefUser(void);
  //Unrefs and unlocks result handler and destroys this object if it's released
  void unRefAndDie(void);
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_DIALOG__ */

