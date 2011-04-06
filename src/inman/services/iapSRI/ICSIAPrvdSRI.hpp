/* ************************************************************************* *
 * IAProvider service utilizing MAP CH-SRI service.
 * ************************************************************************* */
#ifndef __INMAN_ICS_IAPRVD_SRI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPRVD_SRI_HPP

#include "inman/abprov/facility2/IAPQueryFacility.hpp"
#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/iapSRI/IAPrvdSRIDefs.hpp"
#include "inman/services/iapSRI/IAPQuerySRI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::ICServiceAC_T;
using smsc::inman::iaprvd::IAProviderAC;

struct IAPrvdSRI_CFG {
  uint16_t      _maxThreads;
  TCAPUsr_CFG   _sriCfg;
  TCSessionMA * _mapSess;

  IAPrvdSRI_CFG() : _maxThreads(0), _mapSess(0)
  { }
  explicit IAPrvdSRI_CFG(const IAProviderSRI_XCFG & use_cfg)
    : _maxThreads(use_cfg._maxThreads), _sriCfg(use_cfg._sriCfg), _mapSess(0)
  { }
};


class ICSIAPrvdSRI : public ICServiceAC_T<IAProviderSRI_XCFG>,
                      public IAProviderAC {
private:
  mutable Mutex     _sync;
  const char *      _logId; //logging prefix
  IAPrvdSRI_CFG     _cfg;
  IAPQueriesPoolSRI _qrsPool;
  IAPQueryFacility  _qrsFclt;

protected:
  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  virtual Mutex & _icsSync(void) const { return _sync; }
  //Initializes service verifying that all dependent services are inited
  virtual RCode _icsInit(void);
  //Starts service verifying that all dependent services are started
  virtual RCode _icsStart(void);
  //Stops service
  virtual void  _icsStop(bool do_wait = false);

public:
  static const IAPProperty  _iapProperty;

  ICSIAPrvdSRI(std::auto_ptr<IAProviderSRI_XCFG> & use_cfg,
               const ICServicesHostITF * svc_host, Logger * use_log = NULL)
    : ICServiceAC_T<IAProviderSRI_XCFG>(ICSIdent::icsIAPrvdSRI, svc_host, use_cfg, use_log)
    , IAProviderAC(_iapProperty), _logId("iapSRI"), _cfg(*use_cfg.get())
    , _qrsFclt("iapSRI", use_log)
  {
    _icsDeps = use_cfg->_deps;
    _icsState = ICServiceAC::icsStConfig;
  }
  ~ICSIAPrvdSRI()
  {
    ICSStop(true);
  }

  // ----------------------------------
  // -- ICServiceAC_T interface methods
  // ----------------------------------
  //Returns IAProviderAC
  virtual void * Interface(void) const { return (IAProviderAC *)this; }

  // ----------------------------------
  // -- IAProviderAC interface methods
  // ----------------------------------
  virtual void  logConfig(Logger * use_log = NULL) const;
  // -------------------------------------------------------
  // -- IAPQueryProcessorITF interface methods
  // -------------------------------------------------------
  //Starts query and binds listener to it.
  //Returns true if query succesfully started, false otherwise
  virtual bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb);
  //Unbinds query listener, cancels query if no listeners remain.
  //Returns false if listener is already targeted and query waits for its mutex.
  virtual bool cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb);
  //Attempts to cancel all queries.
  //Returns false if at least one listener is already targeted and query waits for its mutex.
  virtual bool cancelAllQueries(void);
};

} //sri
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_ICS_IAPRVD_SRI_HPP */

