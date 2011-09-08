#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include <memory>

#include "eyeline/tcap/provd/TCService.hpp"
#include "eyeline/ss7na/libsccp/SccpApiFactory.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::sccp::SCCPAddress;
using eyeline::ss7na::libsccp::SccpApiFactory;


TCService::TCService() : _logId("TCSrv")
  , _logger(smsc::logger::Logger::getInstance("tcap.provd"))
  , _sccpApi(NULL), _msgReaders(*this), _indGenerator(_dlgFsmReg, _logger)
  , _dlgMgr(_dlgFsmReg, _indGenerator, _logger), _tmoReg("0", _logger)
{
  SccpApiFactory::init();
  _indGenerator.initThreads(2);
  _tmoReg.initThreads(2);
}

TCService::~TCService()
{
  shutdown();
  if (_sccpApi)
    delete _sccpApi;
//TODO:  SccpApiFactory::deinit();
}

//Creates locally initiated dialogue
TDlgProperties
  TCService::createLclDialogue(uint8_t ssn_id, const asn1::EncodedOID & ac_oid,
                               const TDlgHandlerInfo & hdl_info,
                               TDlgTimeout dlg_exp_tmo/* = 0*/) /*throw(std::exception)*/
{
  TDlgProperties  dlgProp;
 
  if (!hdl_info.empty()) {
    SCCPAddress     ownAdr(_cfg._tcapCfg._ownGT, ssn_id, SCCPAddress::routeGT);
    TDlgLocalConfig dlgCfg(ac_oid, hdl_info.dlgHdl, &_dlgMgr, _sccpApi->getConnNumByPolicy(),
                            &_tmoReg, hdl_info.dlgTimeout, _cfg._tcapCfg._dfltROSTmo, ownAdr);

    dlgProp.dlgId = _dlgFsmReg.createTDlgFSM(dlgCfg, &_dlgMgr); //throws
    dlgProp.indHdl = hdl_info.dlgHdl;
    dlgProp.reqHdl = dlgCfg._reqHdl;
  }
  return dlgProp;
}

// ---------------------------------------------------------------------------
// -- TCProviderIface interface methods implementation
// ---------------------------------------------------------------------------
bool TCService::init(const TCProviderCfg & use_cfg) /*throw(std::exception)*/
{
  if (_sccpApi)
    throw smsc::util::Exception("%s::init(): duplicate initialization", _logId);

  _sccpApi = &SccpApiFactory::getSccpApiIface();
  SccpApi::ErrorCode_e errSccp = _sccpApi->init(use_cfg._sccpCfg); //throws
  if (errSccp != SccpApi::OK) {
    smsc_log_fatal(_logger, "%s: SccpApi::init() failed: %d", _logId, (unsigned)errSccp);
    //delete _sccpApi; //TODO: check this!!!
    _sccpApi = 0;
    return false;
  }

  _cfg = use_cfg;
  _msgReaders.init(_sccpApi, _cfg._tcapCfg._iniMsgReaders, _cfg._tcapCfg._maxMsgReaders);
  _dlgMgr.initSCSP(_sccpApi);
  _tmoReg.initTimers(_cfg._tcapCfg._maxDlgNum);

  //connect configured SCCP Service Providers
  bool isOk = false;
  for (unsigned i = 0; i < _cfg._sccpCfg._links.size(); ++i) {
    errSccp = _sccpApi->connect(i);
    if (errSccp != SccpApi::OK)
      smsc_log_error(_logger, "%s: SccpApi::connect(%s) failed: %d", _logId,
                     _cfg._sccpCfg._links[i]._name.c_str(), (unsigned)errSccp);
    else
      isOk |= true;
  }
  if (!isOk) //all SCCP Providers are unreachable
    throw smsc::util::Exception("%s::init(): no SCCP Service Provider available", _logId);
  
  //bind subsystems to connected SCCP providers
  isOk = false;
  for (unsigned i = 0; i < _cfg._sccpCfg._links.size(); ++i) {
    errSccp = _sccpApi->bind(i, &_cfg._tcapCfg._subSNs[0], (uint8_t)(_cfg._tcapCfg._subSNs.size()));
    _sccpApi->getConnectState(_cfg._sccpCfg._links[i]._state, i);
    if (errSccp != SccpApi::OK)
      smsc_log_error(_logger, "%s: SccpApi::bind(%s) failed: %d", _logId,
                     _cfg._sccpCfg._links[i]._name.c_str(), (unsigned)errSccp);
    else {
      smsc_log_info(_logger, "%s: SccpApi::bind(%s) Ok", _logId,
                     _cfg._sccpCfg._links[i]._name.c_str());
      isOk |= true;
    }
  }
  if (!isOk) //all links to SCCP Providers failed to bind subsytems
    throw smsc::util::Exception("%s::init(): no connect binded", _logId);

  _msgReaders.Start();
  _indGenerator.Start();
  return true;
}

void TCService::shutdown()
{
  //unbind subsystems
  for (unsigned i = 0; i < _cfg._sccpCfg._links.size(); ++i) {
    SccpApi::ErrorCode_e errSccp = _sccpApi->unbind(i);
    if (errSccp != SccpApi::OK) {
      smsc_log_error(_logger, "%s: SccpApi::unbind(%s) failed: %d", _logId,
                     _cfg._sccpCfg._links[i]._name.c_str(), (unsigned)errSccp);
    }
  }
  //disconnect links
  for (unsigned i = 0; i < _cfg._sccpCfg._links.size(); ++i) {
    SccpApi::ErrorCode_e errSccp = _sccpApi->disconnect(i);
    if (errSccp != SccpApi::OK) {
      smsc_log_error(_logger, "%s: SccpApi::disconnect(%s) failed: %d", _logId,
                     _cfg._sccpCfg._links[i]._name.c_str(), (unsigned)errSccp);
    }
  }
  //
  _msgReaders.Stop(true);
  _indGenerator.Stop(true);
}
//
bool TCService::registerAppCTX(const asn1::EncodedOID & ctx_oid,
                               TDlgHandlerFactoryIface * dlg_hndls_factory,
                               TDlgTimeout dlg_exp_tmo/* = 0*/)
{
  return _appCtxReg.registerDlgHandlerFactory(ctx_oid, dlg_hndls_factory, dlg_exp_tmo);
}
//
void TCService::unregisterAppCTX(const asn1::EncodedOID & ctx_oid)
{
  _appCtxReg.unregisterDlgHandlerFactory(ctx_oid);
}

//
TDlgProperties
  TCService::createDialogue(uint8_t ssn_id, const asn1::EncodedOID & ac_oid,
                          TDlgTimeout dlg_exp_tmo/* = 0*/) /*throw(std::exception)*/
{
  
  TDlgHandlerInfo hdlInfo = _appCtxReg.getDlgHandler(ac_oid);
  return createLclDialogue(ssn_id, ac_oid, hdlInfo, dlg_exp_tmo);
}
//
TDlgProperties
  TCService::createDialogue(uint8_t ssn_id, ros::LocalOpCode init_opcode,
                          TDlgTimeout dlg_exp_tmo/* = 0*/)
{
  TDlgHandlerInfo hdlInfo = _appCtxReg.getDlgHandler(_ac_contextless_ops, init_opcode);
  return createLclDialogue(ssn_id, _ac_contextless_ops, hdlInfo, dlg_exp_tmo);
}

// ----------------------------------------------------------
// -- TDlgIndProcessorIface interface methods implementation
// ----------------------------------------------------------
void TCService::updateDlgByIndication(TBeginIndComposer & tc_begin_ind)
  /*throw(std::exception)*/
{
//TODO; possibly it's helpfull to maintain std::set<{rmt_adr, rmt_TrId}> of
//      active dialogs and check TR_BeginInd{rmt_adr, rmt_TrId} aginst it.

  const asn1::EncodedOID * acOid = tc_begin_ind.getAppCtx(); //NOTE: always non-zero
  ros::LocalOpCode opCode = 0;

  if (tc_begin_ind.getCompList()) {
    ros::ROSPdu & firstComp = tc_begin_ind.getCompList()->front();
    if (firstComp.get()->isInvoke())
      opCode = firstComp.get()->getOpCode();
  }

  TDlgHandlerInfo hdlInfo = _appCtxReg.getDlgHandler(*acOid, opCode);
  if (hdlInfo.empty()) { //unregistered AppCtx
    TransactionId trId(TransactionId::orgRemote, TDlgFSMRegistry::_TDLG_ID_RESERVED,
                       tc_begin_ind.getOrigTrId());

    _dlgMgr.notifyRmtTCUser(trId, TDialogueAssociate::dsu_ac_not_supported,
                            tc_begin_ind.getSCSPLink(), tc_begin_ind.getDestAddress(),
                            tc_begin_ind.getOrigAddress());
    return;
  }

  TDlgRemoteConfig  dlgCfg(*acOid, hdlInfo.dlgHdl, &_dlgMgr, tc_begin_ind.getSCSPLink(),
                           &_tmoReg, hdlInfo.dlgTimeout, _cfg._tcapCfg._dfltROSTmo,
                           tc_begin_ind.getDestAddress(), tc_begin_ind.getOrigAddress());

  TDialogueId dlgId = _dlgFsmReg.createTDlgFSM(dlgCfg, &_dlgMgr); //throws on failure
  tc_begin_ind.setDialogueId(dlgId);

  TDlgRefPtr_t  pDlgFSM = _dlgFsmReg.getTDlgFSM(dlgId);
  pDlgFSM->updateDlgByIndication(tc_begin_ind);
}

//
void TCService::updateDlgByIndication(TContIndComposer & tc_cont_ind)
  /*throw(std::exception)*/
{
  TDlgRefPtr_t  pDlgFSM = _dlgFsmReg.getTDlgFSM(tc_cont_ind.getDialogueId());
  if (!pDlgFSM.get()) {
    smsc_log_warn(_logger, "%s: %s for unknown TDlg[%u]", _logId,
                  tc_cont_ind.getIdent(), tc_cont_ind.getDialogueId());
    return;
  }
  pDlgFSM->updateDlgByIndication(tc_cont_ind);
}

//
void TCService::updateDlgByIndication(TEndIndComposer & tc_end_ind)
  /*throw(std::exception)*/
{
  TDlgRefPtr_t  pDlgFSM = _dlgFsmReg.getTDlgFSM(tc_end_ind.getDialogueId());
  if (!pDlgFSM.get()) {
    smsc_log_warn(_logger, "%s: %s for unknown TDlg[%u]", _logId,
                  tc_end_ind.getIdent(), tc_end_ind.getDialogueId());
    return;
  }
  pDlgFSM->updateDlgByIndication(tc_end_ind);
}

//
void TCService::updateDlgByIndication(TUAbortIndComposer & tc_uAbort_ind)
  /*throw(std::exception)*/
{
  TDlgRefPtr_t  pDlgFSM = _dlgFsmReg.getTDlgFSM(tc_uAbort_ind.getDialogueId());
  if (!pDlgFSM.get()) {
    smsc_log_warn(_logger, "%s: %s for unknown TDlg[%u]", _logId,
                  tc_uAbort_ind.getIdent(), tc_uAbort_ind.getDialogueId());
    return;
  }
  pDlgFSM->updateDlgByIndication(tc_uAbort_ind);

}

//
void TCService::updateDlgByIndication(TPAbortIndComposer & tc_pAbort_ind)
  /*throw(std::exception)*/
{
  TDlgRefPtr_t  pDlgFSM = _dlgFsmReg.getTDlgFSM(tc_pAbort_ind.getDialogueId());
  if (!pDlgFSM.get()) {
    smsc_log_warn(_logger, "%s: %s for unknown TDlg[%u]", _logId,
                  tc_pAbort_ind.getIdent(), tc_pAbort_ind.getDialogueId());
    return;
  }
  pDlgFSM->updateDlgByIndication(tc_pAbort_ind);
}

//
void TCService::updateDlgByIndication(TNoticeIndComposer & tc_notice_ind)
  /*throw(std::exception)*/
{
  TDlgRefPtr_t  pDlgFSM = _dlgFsmReg.getTDlgFSM(tc_notice_ind.getDialogueId());
  if (!pDlgFSM.get()) {
    smsc_log_warn(_logger, "%s: %s for unknown TDlg[%u]", _logId,
                  tc_notice_ind.getIdent(), tc_notice_ind.getDialogueId());
    return;
  }
  pDlgFSM->updateDlgByIndication(tc_notice_ind);
}

}}}

