/* ************************************************************************** *
 * Abonent contract and gsmSCF parameters determination service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNT_DETECTOR__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_ABNT_DETECTOR__

#include "inman/common/ObjRegistryT.hpp"

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/tcpsrv/ICSTcpSrvDefs.hpp"
#include "inman/services/abdtcr/AbntDtcrManager.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::interaction::ConnectUId;
using smsc::inman::interaction::IProtocolAC;
using smsc::inman::interaction::INPAbntContract;

using smsc::inman::tcpsrv::ConnectGuard;

//
class ICSAbntDetector : public smsc::inman::ICServiceAC_T<AbntDetectorXCFG>
                      , public smsc::inman::tcpsrv::ICSConnServiceIface {
private:
  typedef smsc::inman::interaction::PckBuffersPool_T<256> PacketsPool;
  typedef smsc::util::POBJRegistry_T<ConnectUId, AbntDetectorManager> SessionsRegistry;

  mutable smsc::core::synchronization::EventMonitor  _sync;
  /* - */
  const char *          _logId;     //logging prefix
  AbonentDetectorCFG    _wCfg;      //configuration for workers
  ConnectManagerID      _lastSessId;
  SessionsRegistry      _sessReg;   //
  PacketsPool           _pckPool;   //incoming packets pool

protected:
  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  virtual Mutex & _icsSync(void) const { return _sync; }
  //Initializes service verifying that all dependent services are inited
  virtual RCode _icsInit(void);
  //Starts service verifying that all dependent services are started
  virtual RCode _icsStart(void) { return ICServiceAC::icsRcOk; }
  //Stops service
  virtual void  _icsStop(bool do_wait = false);

public:
  static const INPAbntContract  _iProtoDef; //provided protocol definition

  ICSAbntDetector(std::auto_ptr<AbntDetectorXCFG> & use_cfg,
                  ICServicesHostITF * svc_host, Logger * use_log)
      : smsc::inman::ICServiceAC_T<AbntDetectorXCFG>
          (ICSIdent::icsIdAbntDetector, svc_host, use_cfg, use_log)
      , _logId("AbDtcr"), _wCfg(*use_cfg.get()), _lastSessId(0)
  {
    _icsDeps = use_cfg->icsDeps;
    _icsState = ICServiceAC::icsStConfig;
#ifdef INTRUSIVE_POOL_DEBUG
    _pckPool.debugInit(_logId, logger);
#endif /* INTRUSIVE_POOL_DEBUG */
  }
  virtual ~ICSAbntDetector();

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

} //abdtcr
} //inman
} //smsc
#endif /* __INMAN_ICS_ABNT_DETECTOR__ */

