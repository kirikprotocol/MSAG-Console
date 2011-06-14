#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/map/USSD-Arg.h"
#include "inman/codec_inc/map/USSD-Res.h"

#include "inman/common/adrutil.hpp"
using smsc::cvtutil::packMAPAddress2OCTS;

#include "inman/comp/compsutl.hpp"
using smsc::inman::comp::smsc_log_component;
using smsc::inman::comp::OCTET_STRING_2_Address;
using smsc::inman::comp::ASN1DecodeError;
using smsc::inman::comp::ASN1EncodeError;

#include "ussman/comp/map_uss/MapUSSComps.hpp"
//using smsc::ussman::comp::USSDataString;

using smsc::util::MAPConst;

namespace smsc {
namespace ussman {
namespace comp {
namespace uss {
/* ************************************************************************** *
 * class USSReqArg implementation:
 * ************************************************************************** */
bool ProcessUSSRequestArg::msISDNadr_present(void) const
{
  return (_msAdr.signals[0] && (_msAdr.signals[1] || _msAdr.signals[0] != '0')) ? true : false;
}

bool ProcessUSSRequestArg::msAlerting_present(void) const
{
  return (_alrt == alertingNotSet) ? false : true;
}

void ProcessUSSRequestArg::setMSISDNadr(const char* adrStr) throw(CustomException)
{
  if (!_msAdr.fromText(adrStr))
    throw CustomException(-1, "USSReqArg: invalid msisdn", adrStr);
}

void ProcessUSSRequestArg::decode(const std::vector<uint8_t>& buf)
    throw(CustomException)
{
  USSD_Arg_t *  dcmd = NULL;  /* ASN decoded structure */
  asn_dec_rval_t  drc;        /* Decoder return value  */

  drc = ber_decode(0, &asn_DEF_USSD_Arg, (void **)&dcmd, &buf[0], buf.size());
  ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_USSD_Arg, "USSReqArg"); //throws
  smsc_log_component(compLogger, &asn_DEF_USSD_Arg, dcmd);

  //deserialize content of decoded component
  try {
    if (dcmd->ussd_DataCodingScheme.size != 1)
      throw CustomException("USSReqArg: unsupported DCS length: %u",
                            dcmd->ussd_DataCodingScheme.size);
    //check for constraints not specified in ASN.1 notation
    if (dcmd->ussd_String.size > USSDataString::_max_USSDStringLength) {
      smsc_log_warn(compLogger, "USSReqArg: ussdStrSz is too large: %u", dcmd->ussd_String.size);
    }
    setData(dcmd->ussd_DataCodingScheme.buf[0], dcmd->ussd_String.buf, dcmd->ussd_String.size);

    if (!dcmd->msisdn)
      _msAdr.clear();
    else if (!OCTET_STRING_2_Address(dcmd->msisdn, _msAdr))
      throw CustomException("USSReqArg: invalid msISDN adr");

    if (dcmd->alertingPattern) {
      if (dcmd->alertingPattern->size != 1)
        throw CustomException("USSReqArg: invalid alertingPattern length: %u",
                              dcmd->alertingPattern->size);
      _alrt = (AlertingPattern_e)(dcmd->alertingPattern->buf[0]);
    } else
      _alrt = alertingNotSet;

  } catch (const CustomException & exc) {
    asn_DEF_USSD_Arg.free_struct(&asn_DEF_USSD_Arg, dcmd, 0);
    throw;
  }
  asn_DEF_USSD_Arg.free_struct(&asn_DEF_USSD_Arg, dcmd, 0);
}

void ProcessUSSRequestArg::encode(std::vector<uint8_t>& buf) const throw(CustomException)
{
  asn_enc_rval_t  er;
  /* construct USSD_Arg */
  USSD_Arg_t      cmd;
  /* optionals: */
  uint8_t         alrt_buf[1];
  OCTET_STRING_t  alrt;
  uint8_t         fdcsbuf;
  uint8_t         fussdsbuf[USSDataString::_max_USSDStringLength];
  uint8_t         isdn_buf[1 + MAPConst::MAX_ISDN_AddressLength];
  OCTET_STRING_t  isdn;

  uint16_t dLen = 0;
  /* prepare ussd string */
  if (!isDataAsPackedDCS(&dLen))
    throw CustomException("USSReqArg: ussdata is not set");
  if (dLen > USSDataString::_max_USSDStringLength)
    throw CustomException("USSReqArg: ussdata size is too large: %u", dLen);

  memset(&cmd, 0x00, sizeof(cmd)); //clear optionals and asn_ctx
  cmd.ussd_DataCodingScheme.size = 1;
  cmd.ussd_DataCodingScheme.buf = &fdcsbuf;
  cmd.ussd_String.buf = &fussdsbuf[0];
  cmd.ussd_String.size = getDataAsPackedDCS(cmd.ussd_DataCodingScheme.buf[0], cmd.ussd_String.buf);

  /* prepare optionals */
  if (_alrt != alertingNotSet) {
    memset(&alrt, 0, sizeof(alrt));
    alrt_buf[0] = (uint8_t)_alrt;
    alrt.buf = alrt_buf;
    alrt.size = 1;
    cmd.alertingPattern = &alrt;
  }

  if (msISDNadr_present()) {
    memset(&isdn, 0, sizeof(isdn));
    isdn.size = packMAPAddress2OCTS(_msAdr, isdn_buf);
    isdn.buf = isdn_buf;
    cmd.msisdn = &isdn;
  }

  smsc_log_component(compLogger, &asn_DEF_USSD_Arg, &cmd);

  er = der_encode(&asn_DEF_USSD_Arg, &cmd, print2vec, &buf);
  ASNCODEC_LOG_ENC(er, asn_DEF_USSD_Arg, "USSReqArg");
}

/* ************************************************************************** *
 * class USSReqArg implementation:
 * ************************************************************************** */
void ProcessUSSRequestRes::decode(const std::vector<uint8_t>& buf) throw(CustomException)
{
  USSD_Res_t *        dcmd = NULL;    /* decoded structure */
  asn_dec_rval_t      drc;            /* Decoder return value  */

  drc = ber_decode(0, &asn_DEF_USSD_Res, (void **)&dcmd, &buf[0], buf.size());
  ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_USSD_Res, "USSReqRes");

  /* decode ussd string */
  if (dcmd->ussd_DataCodingScheme.size != 1)
    throw CustomException("USSReqRes: unsupported DCS length: %u", dcmd->ussd_DataCodingScheme.size);
  if (dcmd->ussd_String.size > USSDataString::_max_USSDStringLength) {
    smsc_log_warn(compLogger, "USSReqRes: ussdata size is too large: %u", dcmd->ussd_String.size);
  }
  setData(dcmd->ussd_DataCodingScheme.buf[0], dcmd->ussd_String.buf, dcmd->ussd_String.size);

  smsc_log_component(compLogger, &asn_DEF_USSD_Res, dcmd);
  asn_DEF_USSD_Res.free_struct(&asn_DEF_USSD_Res, dcmd, 0);
}

void ProcessUSSRequestRes::encode(std::vector<uint8_t>& buf) const throw(CustomException)
{
  asn_enc_rval_t      er;
  /* construct USSD_Arg */
  USSD_Res_t          cmd;
  uint8_t             fdcsbuf;
  uint8_t             fussdsbuf[USSDataString::_max_USSDStringLength];

  uint16_t dLen = 0;
  /* prepare ussd string */
  if (!isDataAsPackedDCS(&dLen))
    throw CustomException("USSReqRes: ussdata is not set");
  if (dLen > USSDataString::_max_USSDStringLength)
    throw CustomException("USSReqRes: ussdata size is too large: %u", dLen);

  memset(&cmd, 0x00, sizeof(cmd)); //clear optionals and asn_ctx
  cmd.ussd_DataCodingScheme.size = 1;
  cmd.ussd_DataCodingScheme.buf = &fdcsbuf;
  cmd.ussd_String.buf = &fussdsbuf[0];
  cmd.ussd_String.size = getDataAsPackedDCS(cmd.ussd_DataCodingScheme.buf[0], cmd.ussd_String.buf);

  smsc_log_component(compLogger, &asn_DEF_USSD_Res, &cmd); 

  er = der_encode(&asn_DEF_USSD_Res, &cmd, print2vec, &buf);
  ASNCODEC_LOG_ENC(er, asn_DEF_USSD_Res, "USSReqRes");
}

} //namespace uss
} //namespace comp
} //namespace inman
} //namespace smsc

