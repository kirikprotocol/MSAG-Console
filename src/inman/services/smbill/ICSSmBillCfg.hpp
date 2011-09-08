/* ************************************************************************* *
 * SMS/USSD messages billing service configuration parameters definition.
 * ************************************************************************* */
#ifndef __INMAN_ICS_SMBILLING_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_SMBILLING_DEFS_HPP

#include <map>

#include "util/TonNpiAddress.hpp"
#include "util/OptionalObjT.hpp"
#include "inman/inap/TCUsrDefs.hpp"
#include "inman/services/ICSrvIDs.hpp"
#include "inman/services/common/BillingModes.hpp"
#include "inman/services/smbill/SmsExtraSrvMask.hpp"

namespace smsc    {
namespace inman   {
namespace smbill  {

using smsc::util::TonNpiAddress;
using smsc::inman::inap::TCAPUsr_CFG;

struct XSmsService {
  std::string     name;
  uint32_t        mask;
  uint32_t        svcCode; //actually it's a serviceId
  TonNpiAddress   adr;
  bool            chargeBearer;

  explicit XSmsService(const char * use_name = NULL, uint32_t use_mask = 0)
    : name(use_name), mask(use_mask), svcCode(0), chargeBearer(false)
  { }
};
typedef std::map<uint32_t, XSmsService> SmsXServiceMap;


//core SM billing parameters
class SmBillParams /*: public ChargeParm */{
protected:
  static const char * const k_nmCDRmodes[];

public:
  enum CDRMode_e { cdrNONE = 0, cdrBILLMODE = 1, cdrALL = 2 };
  enum ContractReqMode_e { reqOnDemand = 0, reqAlways };

  static const char * cdrModeStr(CDRMode_e mode_id) { return k_nmCDRmodes[mode_id]; }


  TrafficBillModes  billMode;
  ContractReqMode_e cntrReq;
  CDRMode_e         cdrMode;
  uint32_t          maxBilling;     //maximum number of Billings per connect
  uint16_t          maxThreads;     //maximum number of threads per connect, 0 - means no limitation
  bool              useCache;       //use abonents contract data cache
  uint32_t          cacheTmo;       //abonent cache data expiration timeout in secs
  std::string       cdrDir;         //location to store CDR files
  long              cdrInterval;    //rolling interval for CDR files
  std::auto_ptr<SmsXServiceMap>
                    smsXMap;        //SMS Extra services configuration

  smsc::util::OptionalObj_T<TCAPUsr_CFG>
                    capSms;         //CAP3Sms (SM charging) configuration

  SmBillParams() : cntrReq(reqOnDemand), cdrMode(cdrBILLMODE), maxBilling(0)
    , maxThreads(0), useCache(false), cacheTmo(0), cdrInterval(0)
  { }

  bool needIAProvider(void) const
  {
    return ((cntrReq == SmBillParams::reqAlways) || billMode.useIN());
  }
};

typedef SmBillParams::CDRMode_e         CDR_MODE;
typedef SmBillParams::ContractReqMode_e ContractReqMode_t;

//Billing service configuration parameters (from config.xml)
struct SmBillingXCFG {
  std::auto_ptr<SmBillParams> prm; //core SM billing parameters
  std::string     policyNm;       //name of default AbonenPolicy
  uint16_t        maxTimeout;     //maximum timeout for TCP operations,
                                  //billing aborts on its expiration
  uint16_t        abtTimeout;     //maximum timeout on abonent type requets,
                                  //(HLR & DB interaction), on expiration billing
                                  //continues in CDR mode 
  ICSIdsSet       deps;           //ICServices Billing depends on

  SmBillingXCFG() : prm(new SmBillParams()), maxTimeout(0), abtTimeout(0)
  { }
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_ICS_SMBILLING_DEFS_HPP */

