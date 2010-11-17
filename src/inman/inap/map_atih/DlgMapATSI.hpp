/* ************************************************************************* *
 * mapATIH service: ATSI dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_ATSI__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_MAP_ATSI__

#include "core/synchronization/EventMonitor.hpp"

#include "inman/inap/HDSSnSession.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/map_atih/MapATSIComps.hpp"

namespace smsc {
namespace inman {
namespace inap {
namespace atih {

using smsc::core::synchronization::EventMonitor;
using smsc::util::RCHash;

using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::TCDialogUserITF;

using smsc::inman::comp::atih::RequestedSubscription;
using smsc::inman::comp::atih::ATSIArg;
using smsc::inman::comp::atih::ATSIRes;


class ATSIhandlerITF { // SCF <- HLR */
protected:
  virtual ~ATSIhandlerITF() //forbid interface destruction
  { }

public: 
  virtual void onATSIResult(ATSIRes & arg) = 0;
  //dialog finalization/error handling:
  //if ercode != 0, no result has been got from MAP service,
  //NOTE: MAP dialog may be deleted only from this callback !!!
  virtual void onEndATSI(RCHash ercode = 0) = 0;

  virtual void Awake(void) = 0;
};

typedef union {
  unsigned short value;
  struct {
    unsigned int ctrInited : 2;
    unsigned int ctrResulted : 1;
    unsigned int ctrAborted : 1;
    unsigned int ctrFinished : 1;
  } s;
} ATSIState;

//NOTE: DlgATSI doesn't maintain own timer for operations, it uses instead the 
//innate timer of the SS7 stack for Invoke lifetime.
class MapATSIDlg : TCDialogUserITF { // SCF -> HLR
public:
  MapATSIDlg(TCSessionMA* pSession, ATSIhandlerITF * atsi_handler, Logger * uselog = NULL);

  void destroy(void);

  enum MapOperState { operInited = 1, operFailed = 2, operDone = 3 };

  //requests specified subscription information
  void subsciptionInterrogation(const RequestedSubscription & req_cfg,
                                const TonNpiAddress & subcr_adr,
                                uint16_t timeout = 0) throw(CustomException);

  //Attempts to unbind TC User.
  //Returns true on succsess, false result means that this object has 
  //established references to handler.
  bool Unbind(void);
  //May be called only after successfull Unbind() call
  void endMapDlg(void);

protected:
  friend class smsc::inman::inap::Dialog;
  // TCDialogUserITF interface
  void onDialogInvoke(Invoke* op, bool lastComp) { }
  void onDialogContinue(bool compPresent);
  void onDialogPAbort(uint8_t abortCause);
  void onDialogREnd(bool compPresent);
  void onDialogUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                      uint16_t userInfo_len, uint8_t *pUserInfo);
  void onDialogNotice(uint8_t reportCause,
                      TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                      uint8_t invId = 0, uint8_t opCode = 0);

  // no OPs with returnResult defined
  void onInvokeResult(InvokeRFP pInv, TcapEntity* res);
  void onInvokeError(InvokeRFP pInv, TcapEntity* resE);
  void onInvokeResultNL(InvokeRFP pInv, TcapEntity* res) { }
  void onInvokeLCancel(InvokeRFP pInv);
  //
  void Awake(void) { _sync.notify(); }

private:
  typedef smsc::core::synchronization::MTRefWrapper_T<ATSIhandlerITF>
    MapUserRef;

  mutable EventMonitor  _sync;
  volatile  unsigned    _selfRef;
  volatile  bool        _dieing;

  TCDialogID      atsiId;
  //prefix for logging info
  const char *    _logPfx; //"MapATSI"
  char            _logId[sizeof("MapATSI[%u:%Xh]") + 2*sizeof(unsigned)*3 + 1];
  Dialog *        dialog;     //TCAP dialog
  TCSessionMA *   session;    //TCAP dialogs factory
  MapUserRef      atsiHdl;
  ATSIState       _atsiState;  //current state of mapATSI dialog
  Logger *        logger;

  void endTCap(void); //ends TC dialog, releases Dialog()
  bool doRefHdl(void);
  void unRefHdl(void);
  void resetMapDlg();

  void dieIfRequested(void);
  virtual ~MapATSIDlg();
};

} //atih
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_ATSI__ */

