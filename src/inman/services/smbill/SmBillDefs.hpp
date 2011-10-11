/* ************************************************************************* *
 * SMS/USSD messages billing request processing parameters. 
 * ************************************************************************* */
#ifndef __INMAN_BILLING_PROC_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_BILLING_PROC_DEFS

#include "inman/storage/CDRStorage.hpp"
#include "inman/incache/AbCacheDefs.hpp"
#include "inman/inap/TCDspIface.hpp"

#include "inman/services/iapmgr/IAPMgrDefs.hpp"
#include "inman/services/tmwatch/TMWatcherDefs.hpp"
#include "inman/services/scheduler/TaskSchedulerDefs.hpp"
#include "inman/services/smbill/ICSSmBillCfg.hpp"

namespace smsc   {
namespace inman  {
namespace smbill {

using smsc::inman::filestore::InBillingFileStorage;
using smsc::inman::cache::AbonentCacheITF;

using smsc::inman::TimerFact;
using smsc::inman::AbonentPolicyName_t;
using smsc::inman::iapmgr::IAPManagerITF;
using smsc::inman::TaskSchedulerFactoryITF;
using smsc::inman::inap::TCAPDispatcherITF;


struct SmBillingCFG {
  std::auto_ptr<SmBillParams> prm;    //core SM billing parameters
  TimerFact               maxTimeout; //maximum timeout for TCP operations,
                                      //billing aborts on its expiration
  TimerFact               abtTimeout; //maximum timeout on abonent type requets,
                                      //(HLR & DB interaction), on expiration billing
                                      //continues in CDR mode 
  AbonentPolicyName_t     policyNm;   //name of default AbonenPolicy
  const IAPManagerITF *   iapMgr;
  AbonentCacheITF *       abCache;
  TCAPDispatcherITF *     tcDisp;
  TaskSchedulerFactoryITF * schedMgr;
  std::auto_ptr<InBillingFileStorage> bfs; //CSV files rolling storage

  SmBillingCFG() : prm(new SmBillParams())
    , iapMgr(NULL), abCache(NULL), tcDisp(NULL), schedMgr(NULL)
  { }

  explicit SmBillingCFG(SmBillingXCFG & use_xcfg)
    : prm(use_xcfg.prm.release())
    , maxTimeout((long)use_xcfg.maxTimeout), abtTimeout((long)use_xcfg.abtTimeout)
    , policyNm(use_xcfg.policyNm.c_str())
    , iapMgr(NULL), abCache(NULL), tcDisp(NULL), schedMgr(NULL)
  { }
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_BILLING_PROC_DEFS */

