/* ************************************************************************** *
 * TCProvider: TCProvider service implementation.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_SERVICE_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_SERVICE_HPP

#include "eyeline/corex/timers/TimeoutsMonitor.hpp"

#include "eyeline/ss7na/libsccp/SccpApi.hpp"

#include "eyeline/tcap/TCProviderIface.hpp"

//#include "eyeline/tcap/provd/TDlgFSMRegistry.hpp"
//#include "eyeline/tcap/provd/TDlgIndGenerator.hpp"
#include "eyeline/tcap/provd/TDlgCoordinator.hpp"
//#include "eyeline/tcap/provd/TDlgIndProcessor.hpp"
#include "eyeline/tcap/provd/MsgReadersPool.hpp"
#include "eyeline/tcap/provd/AppCtxSMRegistry.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::corex::timers::TimeoutsMonitor;

using eyeline::ss7na::libsccp::SccpApi;

using eyeline::tcap::TDlgProperties;
using eyeline::tcap::TDlgTimeout;
using eyeline::tcap::TCProviderCfg;
using eyeline::tcap::TDlgHandlerFactoryIface;


class TCService : public eyeline::tcap::TCProviderAC
                , protected TDlgIndProcessorIface {
public:
  TCService();
  ~TCService();

  // -----------------------------------------------------
  // -- TCProviderIface interface methods implementation
  // -----------------------------------------------------
  virtual bool init(const TCProviderCfg & use_cfg) /*throw(std::exception)*/;
  //
  virtual void shutdown(void);
  //Registers factory of handlers of TCAP dialogue associated with given
  //application context.
  //Returns false if ApplicationContext with given OID is already registered
  //NOTE.1: 'Dialogue expiration timeout' shouldn't be less than
  //        Remote Operations timeout implied by application context used.
  // 
  //NOTE.2: If 'Dialogue expiration timeout' isn't set, the default value
  //        specified in TCProvider configuration is used.
  virtual bool registerAppCTX(const asn1::EncodedOID & ctx_oid,
                             TDlgHandlerFactoryIface * dlg_hndls_factory,
                             TDlgTimeout dlg_exp_tmo = 0) /*throw()*/;
  //
  virtual void  unregisterAppCTX(const asn1::EncodedOID & ctx_oid) /*throw()*/;
  //
  virtual TDlgProperties createDialogue(uint8_t ssn_id, const asn1::EncodedOID & ac_oid,
                                   TDlgTimeout dlg_exp_tmo = 0) /*throw(std::exception)*/;
  virtual TDlgProperties createDialogue(uint8_t ssn_id, ros::LocalOpCode init_opcode,
                                   TDlgTimeout dlg_exp_tmo = 0) /*throw(std::exception)*/;

protected:
  //Creates locally initiated dialogue
  TDlgProperties createLclDialogue(uint8_t ssn_id,  const asn1::EncodedOID & ac_oid,
                                   const TDlgHandlerInfo & hdl_info,
                                   TDlgTimeout dlg_exp_tmo/* = 0*/) /*throw(std::exception)*/;

  // ----------------------------------------------------------
  // -- TDlgIndProcessorIface interface methods implementation
  // ----------------------------------------------------------
  virtual void updateDlgByIndication(TBeginIndComposer & tc_begin_ind) /*throw(std::exception)*/;
  virtual void updateDlgByIndication(TContIndComposer & tc_cont_ind) /*throw(std::exception)*/;
  virtual void updateDlgByIndication(TEndIndComposer & tc_end_ind) /*throw(std::exception)*/;
  virtual void updateDlgByIndication(TUAbortIndComposer & tc_uAbort_ind) /*throw(std::exception)*/;
  virtual void updateDlgByIndication(TPAbortIndComposer & tc_pAbort_ind) /*throw(std::exception)*/;
  virtual void updateDlgByIndication(TNoticeIndComposer & tc_notice_ind) /*throw(std::exception)*/;

private:
  TCProviderCfg           _cfg;
  const char *            _logId;
  AppCtxSMRegistry        _appCtxReg;
  smsc::logger::Logger *  _logger;
  SccpApi *               _sccpApi;
  MsgReadersPool          _msgReaders;
  TDlgFSMRegistry         _dlgFsmReg;
  TCLocalIndGenerator     _indGenerator;
  TDlgCoordinator         _dlgMgr;
  TimeoutsMonitor         _tmoReg;
};

} //provd
} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_PROVD_SERVICE_HPP */

