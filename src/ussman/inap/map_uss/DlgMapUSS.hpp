/* ************************************************************************* *
 * MAP-PROCESS-UNSTRUCTURED-SS-REQUEST service:
 * dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_USSMAN_INAP_MAP_USS2__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_INAP_MAP_USS2__

#include "inman/inap/map/DlgMapAC.hpp"
#include "ussman/comp/map_uss/MapUSSComps.hpp"

namespace smsc {
namespace ussman {
namespace inap {
namespace uss {

using smsc::util::CustomException;
using smsc::util::TonNpiAddress;
using smsc::util::IMSIString;

using smsc::inman::inap::TCSessionSR;
using smsc::inman::inap::InvokeRFP;
using smsc::inman::inap::TcapEntity;

using smsc::ussman::comp::USSDataString;

//USS Dialog Handler
class USSDhandlerITF : public smsc::inman::inap::MapDlgUserIface {
protected:
  virtual ~USSDhandlerITF() //forbid interface destruction
  { }

public:
  virtual void onMapResult(const USSDataString & res_arg) = 0;
};


//MAP-USSD dialog: 
//initiates a MAP-PROCESS-UNSTRUCTURED-SS-REQUEST request to SCF,
//awiats result, in case of success reports it to MAP User.
class MapUSSDlg : public smsc::inman::inap::MapDialogAC {
public:
  explicit MapUSSDlg(Logger * use_log = NULL)
    : smsc::inman::inap::MapDialogAC("MapUSS", use_log)
  { }
  //
  virtual ~MapUSSDlg()
  { }

  //Sets TC Dialog registry and MAP user for this dialog
  void init(TCSessionSR & tc_sess, USSDhandlerITF & res_hdl, Logger * use_log = NULL);

  //composes SS request data from plain text(ASCIIZ BY default).
  //Returns initialized dialog logId
  //Throws in case of failure.
  const char * requestSS(const USSDataString & re_data, 
                 const TonNpiAddress * subsc_adr = NULL, const IMSIString * subscr_imsi = NULL)
    /*throw (CustomException)*/;

protected:
  // -----------------------------------------
  // -- MapDialogAC interface methods:
  // -----------------------------------------
  // -----------------------------------------
  // -- TCDialogUserITF interface methods:
  // -----------------------------------------
  //InvokeListener methods
  virtual void onInvokeResultNL(InvokeRFP pInv, TcapEntity * res);
  virtual void onInvokeResult(InvokeRFP pInv, TcapEntity * res);

private:
  TCSessionSR * tcSessSR(void)
  {
    return static_cast<TCSessionSR *>(_tcSess);
  }
  USSDhandlerITF * ussdHdl(void)
  {
    return static_cast<USSDhandlerITF *>(_resHdl.get());
  }
};

} //uss
} //inap
} //ussman
} //smsc

#endif /* __SMSC_USSMAN_INAP_MAP_USS2__ */

