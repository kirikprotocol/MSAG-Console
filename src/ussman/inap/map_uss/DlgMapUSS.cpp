#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
/* ************************************************************************* *
 * MAP-PROCESS-UNSTRUCTURED-SS-REQUEST service:
 * dialog implementation (over TCAP dialog)
 * ************************************************************************* */

#include "util/BinDump.hpp"
using smsc::util::DumpHex;

#include "inman/common/adrutil.hpp"
using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::packMAPAddress2OCTS;

#include "ussman/inap/map_uss/DlgMapUSS.hpp"
using smsc::ussman::comp::uss::MAPUSS_OpCode;

#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::_RCS_TC_Dialog;
using smsc::inman::inap::TC_DlgError;

namespace smsc {
namespace ussman {
namespace inap {
namespace uss {

/* makeUI(): composes UserInfo for TC dialog portion, containing MAP-OpenInfo PDU
 * according to following notation:

UserInfo ::= [UNIVERSAL 8] IMPLICIT SEQUENCE {
  direct-reference    OBJECT IDENTIFIER (map-DialogueAS),
  single-ASN1-type    [0] CHOICE {
    map-accept	[0] MAP-OpenInfo
  }
}

map-DialogueAS  OBJECT IDENTIFIER ::=
  {gsm-NetworkId as-Id(1) map-DialoguePDU(1) version1(1)}

gsm-NetworkId OBJECT IDENTIFIER ::=
  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1) }

MAP-OpenInfo ::= SEQUENCE {
  destinationReference	[0] AddressString OPTIONAL,
        -- IMSI in case of id_ac_map_networkUnstructuredSs_v2 ops (cl. 7.3.1)
  originationReference	[1] AddressString OPTIONAL,
        -- ISDN Address in case of id_ac_map_networkUnstructuredSs_v2 ops (cl. 7.3.1)
  ...,
  extensionContainer	ExtensionContainer  OPTIONAL
  -- extensionContainer must not be used in version 2
}

 * NOTE.1: Some MAP protocol implementations require the ISDN address of 
 * destinationReference to be passed as vendor specific extension:
 *
 *   destinationISDN	[2] AddressString
 *  
 * References:
 * 1) Q.773: Transaction capabilities formats and encoding,
 *    clause 4.2.3 Dialogue Portion
 *
 * 2) X690: Specification of Basic Encoding Rules (BER)
 *    clause 8.18 Encoding of a value of the external type
 *
 * 3) 3GPP TS 29.02 (ETSI TS 129 002): Mobile Application Part (MAP) 
 *    clauses 7.3.1, 17.*
 *
 * NOTE.2: maximum encoded length of UserInfo containing MAP-OpenInfo PDU is
 *         10 + 3*2 + 3*(2 + sizeof(TONNPI_ADDRESS_OCTS)) < 80 bytes;
 */

struct UserInfoBuffer {
protected:
  static const uint16_t _maxUI_len = 80;  //see NOTE.2

  uint16_t _pos;
  uint16_t _size;
  uint8_t  _buf[_maxUI_len + 1];

public:
  UserInfoBuffer() : _pos(0), _size(0)
  { }

  bool empty(void) const { return _size == 0; }

  uint16_t size(void) const { return _size; }

  uint8_t * get(void) { return &_buf[_pos]; }

  void seekEnd(void) { _pos = _maxUI_len; _size = 0; }

  void prepend(uint8_t use_oct)
  {
    _buf[--_pos] = use_oct;
    ++_size;
  }
  //Note: 'num_oct' MUST BE != 0 
  void prepend(const uint8_t * use_octs, uint8_t num_octs)
  {
    do {
      prepend(use_octs[--num_octs]);
    } while (num_octs);
  }
};


//Returns position of 1st encoding octet in buffer
static void makeUI(UserInfoBuffer & ui, const TonNpiAddress & own_adr,
                       const TonNpiAddress * ms_adr, const TonNpiAddress * ms_imsi)
{
  //BER encoded map-DialogueAS OID + single-ASN1-type tag [0]
  static const uint8_t ui_oid[] = {06,0x07,0x04,0x00,0x00,0x01,0x01,0x01,0x01,0xA0};

  uint8_t   addrLen = 0;
  uint8_t   addrOcts[sizeof(TONNPI_ADDRESS_OCTS) + 1];

  ui.seekEnd();
  if (ms_adr) {
    addrLen = (uint8_t)packMAPAddress2OCTS(*ms_adr, addrOcts);
    ui.prepend(addrOcts, addrLen);
    ui.prepend(addrLen);
    ui.prepend(0x82); //destinationISDN [2]
  }

  addrLen = (uint8_t)packMAPAddress2OCTS(own_adr, addrOcts);
  ui.prepend(addrOcts, addrLen);
  ui.prepend(addrLen);
  ui.prepend(0x81); //originationReference	[1]

  if (ms_imsi) {
    addrLen = (uint8_t)packMAPAddress2OCTS(*ms_imsi, addrOcts);
    ui.prepend(addrOcts, addrLen);
    ui.prepend(addrLen);
    ui.prepend(0x80); //destinationReference [0]
  }

  //NOTE: total encoding size is less then 127, so encode it as small number
  addrLen = (uint8_t)ui.size();
  ui.prepend(addrLen);
  ui.prepend(0xA0); //[0] MAP-OpenInfo

  addrLen = (uint8_t)ui.size();
  ui.prepend(addrLen);
  ui.prepend(ui_oid, (uint8_t)sizeof(ui_oid)); //map-DialogueAS OID + single-ASN1-type

  addrLen = (uint8_t)ui.size();
  ui.prepend(addrLen);
  ui.prepend(0x28); //UserInfo ::= IMPLICIT [UNIVERSAL 8]
}

/* ************************************************************************** *
 * class MapUSSDlg implementation:
 * ************************************************************************** */
void MapUSSDlg::init(TCSessionSR & tc_sess, USSDhandlerITF & res_hdl,
                     Logger * use_log/* = NULL*/)
{
  MutexGuard  grd(_sync);
  _tcSess = &tc_sess;
  bindUser(res_hdl);
  if (use_log)
    _logger = use_log;
}

/* ------------------------------------------------------------------------ *
 * MAP-PROCESS-UNSTRUCTURED-SS-REQUEST interface
 * ------------------------------------------------------------------------ */

//composes SS request data from plain text(ASCIIZ BY default).
//Returns initialized dialog logId
//Throws in case of failure.
const char *
  MapUSSDlg::requestSS(const USSDataString & req_data, 
                       const TonNpiAddress * subsc_adr/* = NULL*/,
                       const IMSIString * subscr_imsi/* = NULL*/)
  /*throw (CustomException)*/
{
  MutexGuard  grd(_sync);
  if (!_resHdl.get() || !_tcSess)
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                          _logPfx, "invalid initialization");

  if (!(_tcDlg = tcSessSR()->openDialog(_logger)))
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                          _logPfx, "unable to create TC dialog");
  bindTCDialog(); //composes _logId

  //create Component for TCAP invoke
  smsc::ussman::comp::uss::ProcessUSSRequestArg arg(req_data, _logger);

  if (_logger->isDebugEnabled()) {
    if (req_data.isLatin1Text()) {
      _logger->log_(Logger::LEVEL_DEBUG, "%s: USS request: '%s'", _logId, req_data.getText());
    } else {
      char dump[USSDataString::_max_StrForm_sz];

      uint16_t sLen = req_data.toString(dump, (uint16_t)sizeof(dump));
      if (sLen < (uint16_t)sizeof(dump)) {
        _logger->log_(Logger::LEVEL_DEBUG, "%s: USS request: '%s'", _logId, dump);
      } else { //WARNING: too long data!
        char * pStr = new char[sLen + 1];
        req_data.toString(pStr, sLen + 1);
        _logger->log_(Logger::LEVEL_WARN, "%s: USS request: '%s'", _logId, pStr);
        delete [] pStr;
      }
    }
  }

  if (subsc_adr)
    arg.setMSISDNadr(*subsc_adr);

  UserInfoBuffer uiBuf;
  //GVR NOTE: though MAP specifies that msISDN address in USS request may
  //present in component portion of TCAP invoke, the most of HLRs expect
  //it only in user info section (see makeUI() comments).
  if (subsc_adr || subscr_imsi) {//prepare user info
    TonNpiAddress msImsi;
    if (subscr_imsi) { //verify IMSI
      if (!msImsi.fromText(subscr_imsi->c_str()))
        throw CustomException("%s: invalid IMSI: '%s'", _logId, subscr_imsi->c_str());
      //set TonNPI to International 'Land Mobile' (E.212) for transmission in UserInfo
      msImsi.numPlanInd = 0x06;
      msImsi.typeOfNumber = 0x01;
    }
    makeUI(uiBuf, _tcSess->getOwnAdr(), subsc_adr, msImsi.empty() ? NULL : &msImsi);
  }

  _tcDlg->sendInvoke(MAPUSS_OpCode::processUSS_Request, &arg);
  if (uiBuf.empty())
    _tcDlg->beginDialog(); //throws
  else
    _tcDlg->beginDialog(uiBuf.get(), uiBuf.size()); //throws
  _ctrState.s.ctrInited = MapDialogAC::operInited;
  return _logId;
}


/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapUSSDlg::onInvokeResultNL(InvokeRFP pInv, TcapEntity* res)
{
  MutexGuard  grd(_sync);
  smsc_log_warn(_logger, "%s: unexoected TC_RESULT_NL_IND for %s", _logId,
                pInv->idStr().c_str());
}

void MapUSSDlg::onInvokeResult(InvokeRFP pInv, TcapEntity* res)
{
  bool do_end = false;
  smsc::ussman::comp::uss::ProcessUSSRequestRes *
    resComp = static_cast<smsc::ussman::comp::uss::ProcessUSSRequestRes *>(res->getParam());
  {
    MutexGuard  grd(_sync);
    if (resComp)
      smsc_log_debug(_logger, "%s: %s got a returnResult(%u)",
                    _logId, pInv->idStr().c_str(), (unsigned)res->getOpcode());
    else
      smsc_log_error(_logger, "%s: %s got a returnResult(%u) with missing parameter",
                    _logId, pInv->idStr().c_str(), (unsigned)res->getOpcode());

    _ctrState.s.ctrResulted = 1;
    _ctrState.s.ctrInited = MapDialogAC::operDone;
    if ((do_end = (_ctrState.s.ctrFinished != 0)))
      unbindTCDialog();
    if (!doRefUser())
      return;
  }
  if (resComp)
    ussdHdl()->onMapResult(*resComp);
  unRefUserNotify(do_end, 0);
}

} //chsri
} //inap
} //ussman
} //smsc


