#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/tests/TstAbDtcr.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::util::CAPConst;
using smsc::util::format;

using smsc::inman::interaction::SerializablePacketIface;
using smsc::inman::interaction::INPAbntContract;
using smsc::inman::interaction::INPAbntContractHdr_dlg;
using smsc::inman::interaction::SPckContractRequest;
using smsc::inman::interaction::SPckContractResult;

using smsc::inman::iaprvd::CSIRecord;
using smsc::inman::comp::UnifiedCSI;

namespace smsc  {
namespace inman {
namespace test {
/* ************************************************************************** *
 * class DtcrDialog implementation
 * ************************************************************************** */
DtcrDialog::DtcrDialog(DtcrFacade * use_mgr, unsigned dlg_id, unsigned ab_id, bool use_cache/* = true*/)
  : _mgr(use_mgr), dId(dlg_id), abId(ab_id), state(dIdle), useCache(use_cache)
{ 
  abInfo = _mgr->getAbntDb().getAbnInfo(ab_id);
}

//Composes and sends request, num_bytes == 0 forces sending whole packet
bool DtcrDialog::sendRequest(unsigned num_bytes/* = 0*/)
{
  SPckContractRequest  pck;
  pck._Hdr.dlgId = dId;
  pck._Cmd.setSubscrNumber(abInfo->msIsdn.toString().c_str());
  pck._Cmd.allowCache(useCache);

  char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
  int n = 0;
  if (num_bytes)
    n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
  tbuf[n] = 0;

  std::string msg;
  format(msg, "--> %sRequest[%u](%s) : cache(%s) ..", tbuf, dId,
         pck._Cmd.subscrAddr().c_str(), useCache ? "true" : "false");
  _mgr->Prompt(Logger::LEVEL_INFO, msg);
  if (!_mgr->sendPckPart(&pck, num_bytes)) {
    state = dRequested;
    return true;
  }
  msg.clear(); format(msg, "--> Request[%u]: sending failed!", dId);
  _mgr->Prompt(Logger::LEVEL_ERROR, msg);
  return false;
}

void DtcrDialog::onResult(AbntContractResult * res)
{
  state = dReported;
  std::string msg;
  format(msg, "<-- Result[%u](%s): ", dId, abInfo->msIsdn.toString().c_str());
  if (res->errorCode()) {
    format(msg, "failed, error %u: %s", res->errorCode(), res->errorMsg());
  } else {
    AbonentSubscription abRec(res->contractType(), res->getSubscrImsi());
    if (res->getGsmSCF()) {
      CSIRecord  csiRec;
      csiRec.scfInfo = *res->getGsmSCF();
      csiRec.csiId =  csiRec.scfInfo.serviceKey ?
                        UnifiedCSI::csi_MO_SM : UnifiedCSI::csi_O_BC;
      abRec.csiSCF.insert(csiRec);
    }
    format(msg, " %s, source(%s)", abRec.toString().c_str(),
           res->cacheUsed() ? "cache" : res->policyUsed());
    abInfo->setSubscription(abRec);
  }
  _mgr->Prompt(Logger::LEVEL_INFO, msg);
}


/* ************************************************************************** *
 * class DtcrFacade implementation:
 * ************************************************************************** */
const INPAbntContract  DtcrFacade::_protoDef; //provided protocol definition

DtcrFacade::DtcrFacade(AbonentsDB & use_db, TcpServerIface & conn_srv, Logger * use_log/* = NULL*/)
  : TSTFacadeAC(conn_srv, use_log), _abDB(use_db)
  , _maxDlgId(0)
{
  strcpy(_logId, "TFDtcr");
}

DtcrFacade::~DtcrFacade()
{ 
  MutexGuard grd(_sync);
  do_disconnect();
  if (!_Dialogs.empty()) {
    DtcrDialogsMap::const_iterator it;
    for (it = _Dialogs.begin(); it != _Dialogs.end(); ++it) {
      DtcrDialog * dlg = it->second;
      delete dlg;
    }
    _Dialogs.clear();
  }
}

DtcrDialog * DtcrFacade::findDialog(unsigned did) const
{
  MutexGuard grd(_sync);
  DtcrDialogsMap::const_iterator it = _Dialogs.find(did);
  return (it != _Dialogs.end()) ? it->second : NULL;
}
void DtcrFacade::eraseDialog(unsigned did)
{
  MutexGuard grd(_sync);
  _Dialogs.erase(did);
}

DtcrDialog * DtcrFacade::initDialog(unsigned ab_id, bool use_cache/* = true*/)
{
  MutexGuard grd(_sync);
  unsigned did = nextDialogId();
  std::auto_ptr<DtcrDialog> dlg(new DtcrDialog(this, did, ab_id, use_cache));
  if (dlg->abnInfo()) {
    _Dialogs.insert(DtcrDialogsMap::value_type(did, dlg.get()));
    return dlg.release();
  }
  Prompt(Logger::LEVEL_ERROR, format("unregistered abonent[%u]!", ab_id));
  return NULL;
}

bool DtcrFacade::numericIncrement(char * buf, unsigned len)
{
  if (!len)
    return false;
  char nbuf[CAPConst::MAX_SMS_AddressValueLength + 1];
  unsigned i = CAPConst::MAX_SMS_AddressValueLength;
  nbuf[i] = 0;

  bool shift = true;
  do {
    nbuf[--i] = buf[--len];
    if ((nbuf[i] >= 0x30) && (nbuf[i] <= 0x39)) {
      if (shift) {
        if (++(nbuf[i]) > 0x39)
          nbuf[i] = 0x30;
        else
          shift = false;
      }
    } else
      return false;
  } while (len);
  strcpy(buf, nbuf + i);
  return true;
}


bool DtcrFacade::detectAbn(unsigned ab_id, bool use_cache/* = true*/)
{
  DtcrDialog * dlg = initDialog(ab_id, use_cache);
  return dlg ? dlg->sendRequest() : false;
}

bool DtcrFacade::detectAbn(const TonNpiAddress & sbscr, bool use_cache/* = true*/)
{
  unsigned ab_id = _abDB.searchAbn(sbscr);
  if (!ab_id) {
    AbonentInfo ab_info(sbscr);
    ab_id = _abDB.setAbnInfo(ab_info);
  }
  return ab_id ? detectAbn(ab_id, use_cache) : false;
}

//detects contract for number of abonents from DB starting from given abId
unsigned DtcrFacade::detectAbnMlt(unsigned ab_id, unsigned range,
                                  bool use_cache/* = true*/)
{
  unsigned rval = 0;
  for (; range > 0; --range, ++ab_id) {
    DtcrDialog * dlg = initDialog(ab_id, use_cache);
    if (!dlg)
      break;
    if (detectAbn(ab_id, use_cache))
      ++rval;
  }
  return rval;
}
//detects contract for number of abonents starting from given ISDN address
unsigned DtcrFacade::detectAbnMlt(const TonNpiAddress & sbscr, unsigned range,
                                  bool use_cache/* = true*/)
{
  unsigned rval = 0;
  TonNpiAddress msIsdn = sbscr;
  for (; range > 0; --range) {
    unsigned ab_id = _abDB.searchAbn(msIsdn);
    if (!ab_id) {
      AbonentInfo ab_info(msIsdn);
      ab_id = _abDB.setAbnInfo(ab_info);
    }
    if (detectAbn(ab_id, use_cache))
      ++rval;
    if (!numericIncrement(msIsdn.signals, msIsdn.length))
      break;
  }
  return rval;
}

// ------------------------------------------------------------
// -- PacketListenerIface interface methods:
// ------------------------------------------------------------
//Returns true if listener has utilized packet so no more listeners
//should be notified, false - otherwise (in that case packet will be
//reported to other listeners).
bool DtcrFacade::onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
  /*throw(std::exception) */
{
  INPAbntContract::PduId pduId = _protoDef.isKnownPacket(recv_pck);
  if (!pduId) {
    Prompt(Logger::LEVEL_ERROR, "unsupported Pdu received");
    return true;
  }
  if (pduId != SPckContractResult::getPduId()) {
    std::string msg;
    format(msg, "illegal Cmd[%u] received", (unsigned)_protoDef.getCmdId(pduId));
    Prompt(Logger::LEVEL_ERROR, msg);
    return true;
  }

  SPckContractResult iPck;
  try {
    iPck.deserialize(recv_pck, SerializablePacketIface::dsmPartial);
  } catch (const std::exception & exc) {
    std::string msg;
    format(msg, "corrupted Pdu[%u] received - %s", (unsigned)pduId, exc.what());
    Prompt(Logger::LEVEL_ERROR, msg);
    return true;
  }

  //complete deserialization
  try { iPck._Cmd.load(recv_pck); 
  } catch (const std::exception & exc) {
    std::string msg;
    format(msg, "corrupted cmd %u (dlgId: %u) received: %s",
            iPck._Cmd._cmdTAG, iPck._Hdr.dlgId, exc.what());
    Prompt(Logger::LEVEL_ERROR, msg.c_str());
    return true;
  }
  onContractResult(&iPck._Cmd, iPck._Hdr.dlgId);
  return true;
}

// ---------------------------------------------------
// -- AbntContractResHandlerITF interface implementation
// ---------------------------------------------------
void DtcrFacade::onContractResult(AbntContractResult* res, uint32_t req_id)
{
  DtcrDialog * pDlg = findDialog(req_id);
  if (!pDlg) {
    std::string msg;
    format(msg, "ERR: Dialog[%u] is inknown!", req_id);
    Prompt(Logger::LEVEL_ERROR, msg);
  } else {
    pDlg->onResult(res);
    eraseDialog(req_id);
  }
}

} // test
} // namespace inman
} // namespace smsc

