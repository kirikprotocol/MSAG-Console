#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/tests/TstSmDialog.hpp"

namespace smsc  {
namespace inman {
namespace test {
/* ************************************************************************** *
 * class CapSmDialogCfg implementation
 * ************************************************************************** */
void CapSmDialogCfg::printConfig(FILE * use_stream/* = stdout*/) const
{
  fprintf(use_stream, "CamSmDialog config:\n"
            "  bearerType : dp%s\n", _ussdOp ? "USSD" : "SMS");
  if (_orgAb) {
    const char * orgImsi = getOrgIMSI();
    fprintf(use_stream, 
            "  OrigAbnt[%u]: %s (%s), IMSI %s\n",
            _orgAbId, _orgAb->msIsdn.toString().c_str(), _orgAb->type2Str(),
            orgImsi ? orgImsi : "<none>");
  } else {
    fprintf(use_stream,
            "  OrigAbnt[%u]: unknown (unknown), IMSI <none>\n", _orgAbId);
  }
  const TonNpiAddress * adrMSC = getOrgMSC();
  fprintf(use_stream, 
          "  OrigMSC: %s\n", adrMSC ? adrMSC->toString().c_str() : "<none>");

  if (_dstAb) {
    const char * abImsi = getDstIMSI();
    fprintf(use_stream, "  DestAbnt[%u]: %s (%s), IMSI %s\n", _dstAbId,
            _dstAb->msIsdn.toString().c_str(), _dstAb->type2Str(),
            abImsi ? abImsi : "<none>");
  } else
    fprintf(use_stream, "  DestAbnt[%u]: unknown (unknown), IMSI <none>\n", _dstAbId);

  adrMSC = getDstMSC();
  fprintf(use_stream, 
          "  DestMSC: %s\n", adrMSC ? adrMSC->toString().c_str() : "<none>");

  fprintf(use_stream, 
          "  chargePol  : %s\n"
          "  chargeType : %s\n"
          "  forcedCDR  : %s\n"
          "  SMSExtra: %xh\n",
          CDRRecord::nmPolicy(_chgPolicy), _chgType ? "MT" : "MO",
          _forcedCDR ? "ON" : "OFF", _xsmsIds);
}


const char * CapSmDialogCfg::getOrgIMSI(void) const
{
  if (_isOrgIMSI)
    return (_orgAb && !_orgAb->abImsi.empty()) ? _orgAb->abImsi.c_str() : NULL;
  return _orgIMSI.empty() ? NULL : _orgIMSI.c_str();
}

const char * CapSmDialogCfg::getDstIMSI(void) const
{
  if (_isDstIMSI)
    return (_dstAb && !_dstAb->abImsi.empty()) ? _dstAb->abImsi.c_str() : NULL;
  return _dstIMSI.empty() ? NULL : _dstIMSI.c_str();
}
//
const TonNpiAddress * CapSmDialogCfg::getOrgMSC(void) const
{
  if (_isOrgMSC)
    return (_orgAb && !_orgAb->vlrNum.empty()) ? &_orgAb->vlrNum : NULL;
  return _orgMSC.empty() ? NULL : &_orgMSC;
}
const TonNpiAddress * CapSmDialogCfg::getDstMSC(void) const
{
  if (_isDstMSC)
    return (_dstAb && !_dstAb->vlrNum.empty()) ? &_dstAb->vlrNum : NULL;
  return _dstMSC.empty() ? NULL : &_dstMSC;
}

void  CapSmDialogCfg::setOrgIMSI(bool imsi_on, const IMSIString * use_imsi/* = NULL*/)
{
  if (!(_isOrgIMSI = imsi_on) && use_imsi)
    _orgIMSI = *use_imsi;
  else
    _orgIMSI.clear();
}
void  CapSmDialogCfg::setDstIMSI(bool imsi_on, const IMSIString * use_imsi/* = NULL*/)
{
  if (!(_isDstIMSI = imsi_on) && use_imsi)
    _dstIMSI = *use_imsi;
  else
    _dstIMSI.clear();
}
//
void  CapSmDialogCfg::setOrgMSC(bool msc_on, const TonNpiAddress * use_msc/* = NULL*/)
{
  if (!(_isOrgMSC = msc_on) && use_msc)
    _orgMSC = *use_msc;
  else
    _orgMSC.clear();
}
void  CapSmDialogCfg::setDstMSC(bool msc_on, const TonNpiAddress * use_msc/* = NULL*/)
{
  if (!(_isDstMSC = msc_on) && use_msc)
    _dstMSC = *use_msc;
  else
    _dstMSC.clear();
}

} // test
} // namespace inman
} // namespace smsc

