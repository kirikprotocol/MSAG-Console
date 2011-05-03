/* ************************************************************************* *
 * MAP_SEND_ROUTING_INFO service: dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_CHSRI__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_MAP_CHSRI__

#include "inman/inap/HDSSnSession.hpp"
#include "inman/inap/map/DlgMapAC.hpp"
#include "inman/comp/map_chsri/MapCHSRIComps.hpp"

namespace smsc {
namespace inman {
namespace inap {
namespace chsri {

using smsc::util::CustomException;
using smsc::inman::inap::TCSessionMA;
using smsc::inman::comp::chsri::CHSendRoutingInfoRes;

class CHSRIhandlerITF : public MapDlgUserIface { // GMSC/SCF <- HLR
protected:
  virtual ~CHSRIhandlerITF() //forbid interface destruction
  { }

public:
  virtual void onMapResult(CHSendRoutingInfoRes & res) = 0;
};

//MAP-CH-SRI dialog: 
//initiates a CallHandling SEND-ROUTING-INFO request to HLR,
//collects intermediate results, in case of success reports it to MAP User.
class MapCHSRIDlg : public MapDialogAC { // GMSC/SCF -> HLR
private:
  TCSessionMA *         _tcSess;
  CHSendRoutingInfoRes  _reqRes;

  CHSRIhandlerITF * sriHdl(void)
  {
    return static_cast<CHSRIhandlerITF *>(_resHdl.get());
  }

public:
  explicit MapCHSRIDlg(Logger * use_log = NULL)
    : MapDialogAC("MapSRI", use_log), _tcSess(NULL)
  { }
  //
  virtual ~MapCHSRIDlg()
  { }

  //Sets TC Dialog registry and MAP user for this dialog
  void init(TCSessionMA & tc_sess, CHSRIhandlerITF & res_hdl, Logger * use_log = NULL);
  //
  void reqRoutingInfo(const char * subcr_adr, uint16_t timeout_sec = 0)
    throw(CustomException);
  void reqRoutingInfo(const TonNpiAddress & tnpi_adr, uint16_t timeout_sec = 0)
    throw(CustomException);

protected:
  // -----------------------------------------
  // -- MapDialogAC interface methods:
  // -----------------------------------------
  //Releases TC dialog object
  virtual void  rlseTCDialog(void) { _tcSess->releaseDialog(_tcDlg); }

  // -----------------------------------------
  // -- TCDialogUserITF interface methods:
  // -----------------------------------------
  //InvokeListener iface methods
  virtual void onInvokeResultNL(InvokeRFP pInv, TcapEntity * res);
  virtual void onInvokeResult(InvokeRFP pInv, TcapEntity * res);
};

} //chsri
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_CHSRI__ */

