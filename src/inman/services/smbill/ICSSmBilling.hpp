/* ************************************************************************** *
 * SMS/USSD messages billing service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_SMBILLING_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_SMBILLING_HPP

#include "inman/common/ObjRegistryT.hpp"

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/tcpsrv/ICSTcpSrvDefs.hpp"
#include "inman/services/smbill/SmBillManager.hpp"

namespace smsc {
namespace inman {
namespace smbill {

using smsc::inman::interaction::ConnectUId;
using smsc::inman::interaction::IProtocolAC;
using smsc::inman::tcpsrv::ConnectGuard;
using smsc::inman::filestore::InFileStorageRoller;

//
class ICSSmBilling : public smsc::inman::ICServiceAC_T<SmBillingXCFG>
                   , public smsc::inman::tcpsrv::ICSConnServiceIface {
private:
  typedef smsc::inman::interaction::PckBuffersPool_T<1536> PacketsPool;
  typedef smsc::util::POBJRegistry_T <ConnectUId, SmBillManager> SessionsRegistry;

  mutable smsc::core::synchronization::EventMonitor _sync;
  /* - */
  const char *        _logId;     //logging prefix
  SmBillingCFG        _wCfg;      //configuration for workers
  ConnectManagerID    _lastSessId;
  SessionsRegistry    _sessReg;
  PacketsPool         _pckPool;   //incoming packets pool
  std::auto_ptr<InFileStorageRoller> _roller;

protected:
  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  Mutex & _icsSync(void) const { return _sync; }
  //Initializes service verifying that all dependent services are inited
  RCode _icsInit(void);
  //Starts service verifying that all dependent services are started
  RCode _icsStart(void);
  //Stops service
  void  _icsStop(bool do_wait = false);

public:
  static const INPBilling  _iProtoDef; //provided protocol definition

  ICSSmBilling(std::auto_ptr<SmBillingXCFG> & use_cfg,
                  ICServicesHostITF * svc_host, Logger * use_log)
      : smsc::inman::ICServiceAC_T<SmBillingXCFG>
          (ICSIdent::icsIdSmBilling, svc_host, use_cfg, use_log)
      , _logId("SmBill"), _wCfg(*(use_cfg.get())), _lastSessId(0)
  {
    _icsDeps = use_cfg->deps;
    delete use_cfg.release();
    _icsState = ICServiceAC::icsStConfig;
#ifdef __GRD_POOL_DEBUG__
    _pckPool.debugInit(_logId, logger);
#endif /* __GRD_POOL_DEBUG__ */
  }
  virtual ~ICSSmBilling();

  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  //Returns ICSConnServiceIface
  virtual void * Interface(void) const
  {
    return (smsc::inman::tcpsrv::ICSConnServiceIface*)this;
  }

  // -------------------------------------
  // ICSConnServiceIface interface methods:
  // -------------------------------------
  //Returns definition of IProtocol this service provides
  virtual const IProtocolAC & protoDef(void) const { return _iProtoDef; }
  //Creates a connect listener serving given connect.
  //Returns true on success, false -  otherwise.
  //Note: upon entry the referenced Connect is configured 
  //  to process in consequitive mode, so it's recommended
  //  to reconfigure Connect within this call.
  virtual bool setConnListener(const ConnectGuard & use_conn) /*throw()*/;
  //Notifies that given connection is to be closed, no more socket events will be reported.
  virtual void onDisconnect(const ConnectGuard & use_conn) /*throw()*/;
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_ICS_SMBILLING_HPP */

