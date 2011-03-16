/* ************************************************************************* *
 * mapATIH service: ATSI dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_ATSI__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_MAP_ATSI__

#include "inman/inap/HDSSnSession.hpp"
#include "inman/inap/map/DlgMapAC.hpp"
#include "inman/comp/map_atih/MapATSIComps.hpp"

namespace smsc {
namespace inman {
namespace inap {
namespace atih {

using smsc::inman::inap::TCSessionMA;

using smsc::inman::comp::atih::RequestedSubscription;
using smsc::inman::comp::atih::ATSIArg;
using smsc::inman::comp::atih::ATSIRes;


class ATSIhandlerITF : public MapDlgUserIface {
protected:
  virtual ~ATSIhandlerITF() //forbid interface destruction
  { }

public: 
  virtual void onATSIResult(ATSIRes & arg) = 0;
};

//MAP-ATSI dialog: 
//initiates a ANY-TIME-SUBSCRIPTION-INTERROFATION request to HLR,
//awiats result, in case of success reports it to MAP User.
class MapATSIDlg : public MapDialogAC { // VLR -> HLR
private:
  TCSessionMA * _tcSess;

  ATSIhandlerITF * atsiHdl(void)
  {
    return static_cast<ATSIhandlerITF *>(_resHdl.get());
  }

public:
  explicit MapATSIDlg(Logger * use_log = NULL)
    : MapDialogAC("MapATSI", use_log), _tcSess(NULL)
  { }
  //
  virtual ~MapATSIDlg()
  { }

  //Sets TC Dialog registry and MAP user for this dialog
  void init(TCSessionMA & tc_sess, ATSIhandlerITF & res_hdl, Logger * use_log = NULL);

  //requests specified subscription information
  void subsciptionInterrogation(const RequestedSubscription & req_cfg,
                                const TonNpiAddress & subcr_adr,
                                uint16_t timeout_sec = 0) throw(CustomException);
protected:
  // -----------------------------------------
  // -- MapDialogAC interface methods:
  // -----------------------------------------
  //Releases TC dialog object
  virtual void  rlseTCDialog(void) { _tcSess->releaseDialog(_tcDlg); }
 
  // -----------------------------------------
  // -- TCDialogUserITF interface methods:
  // -----------------------------------------
  //InvokeListener methods
  virtual void onInvokeResultNL(InvokeRFP pInv, TcapEntity * res);
  virtual void onInvokeResult(InvokeRFP pInv, TcapEntity * res);
};

} //atih
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_ATSI__ */

