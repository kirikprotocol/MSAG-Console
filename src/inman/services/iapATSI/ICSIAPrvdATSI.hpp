/* ************************************************************************* *
 * IAProvider service utilizing MAP ATSI service.
 * ************************************************************************* */
#ifndef __INMAN_ICS_IAPRVD_ATSI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPRVD_ATSI_HPP

#include "inman/abprov/facility2/IAPQueryFacility.hpp"
#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/iapATSI/IAPrvdATSIDefs.hpp"
#include "inman/services/iapATSI/IAPQueryATSI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

using smsc::inman::ICServiceAC_T;
using smsc::inman::iaprvd::IAProviderAC;

struct IAPrvdATSI_CFG {
  uint16_t      _maxThreads;
  TCAPUsr_CFG   _atsiCfg;
  TCSessionMA * _mapSess;

  IAPrvdATSI_CFG() : _maxThreads(0), _mapSess(0)
  { }
  explicit IAPrvdATSI_CFG(const IAProviderATSI_XCFG & use_cfg)
    : _maxThreads(use_cfg._maxThreads), _atsiCfg(use_cfg._atsiCfg), _mapSess(0)
  { }
};


class ICSIAPrvdATSI : public ICServiceAC_T<IAProviderATSI_XCFG>,
                      public IAProviderAC {
private:
  mutable Mutex       _sync;
  const char *        _logId; //logging prefix
  IAPrvdATSI_CFG      _cfg;
  IAPQueriesPoolATSI  _qrsPool;
  IAPQueryFacility    _qrsFclt;

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
  static const IAPProperty  _iapProperty;

  ICSIAPrvdATSI(std::auto_ptr<IAProviderATSI_XCFG> & use_cfg,
                         const ICServicesHostITF * svc_host, Logger * use_log = NULL)
    : ICServiceAC_T<IAProviderATSI_XCFG>(ICSIdent::icsIAPrvdATSI, svc_host, use_cfg, use_log)
    , IAProviderAC(_iapProperty), _logId("iapATSI"), _cfg(*use_cfg.get())
    , _qrsFclt("iapATSI", use_log)
  {
    _icsDeps = use_cfg->_deps;
    _icsState = ICServiceAC::icsStConfig;
  }
  ~ICSIAPrvdATSI()
  {
    ICSStop(true);
  }

  // ----------------------------------
  // -- ICServiceAC_T interface methods
  // ----------------------------------
  //Returns IAProviderAC
  void * Interface(void) const { return (IAProviderAC*)this; }

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
  virtual bool cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb);
  virtual bool cancelAllQueries(void);
};

} //sri
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_ICS_IAPRVD_ATSI_HPP */

