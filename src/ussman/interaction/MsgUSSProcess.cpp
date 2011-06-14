#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "inman/interaction/serializer/SerializeFxdLenStringT.hpp"
#include "inman/interaction/serializer/SerializeOctArray16.hpp"
using smsc::inman::interaction::operator <<;
using smsc::inman::interaction::operator >>;

#include "ussman/interaction/MsgUSSProcess.hpp"

namespace smsc  {
namespace ussman {
namespace interaction {

/*
 * USS messages are transferred with length prefix and have the following
 * serialization formats:

Request (PROCESS_USS_REQUEST_TAG):
--------l
  2b        4b       1b   1b       1b        up to 160b   1b     up to 32    1b     1b    up to 32 1b     up to 32
------   ---------   --  -----  ----------   ----------   ---  -----------  ------   ---   -------- ----   -------
 cmdId : requestId : flg [DCS] : ussDataLen  :  ussData  : len : ms ISDN  : IN SSN : len : IN ISDN IMSI len  IMSI
                    |                                                                                           |
                     ------- processed by load() method -------------------------------------------------------

Result (PROCESS_USS_RESULT_TAG):
--------

  2b        4b          2b     1b   1b       1b        up to 160b   1b     up to 32
------   ---------   -------   --  -----   ----------   ----------   ---  -----------------------
 cmdId : requestId  : status : flg [DCS] : ussDataLen  :  ussData  : len : ms ISDN address string
                             |                                                             |
                              -------- present if status == 0 -----------------------------
                    |                                                                      |
                     ------- processed by save() method -----------------------------------
 
flg is type of USSDataString::DataKind_e, if flg == ussdPackedDCS then DCS presents and ussData 
contains binary data, otherwise flg indicates text string encoding and DCS is missed.
*/

/* ************************************************************************** *
 * class USSGatewayCmdAC implementation:
 * ************************************************************************** */
unsigned USSGatewayCmdAC::log2str(char * use_buf, unsigned buf_len/* = _ussOpData_strSZ*/) const
{
  unsigned res = 0, tSz = 0;
  *use_buf = 0;

  tSz = (unsigned)sizeof("msAdr=") - 1;
  if ((res + tSz) >= buf_len) {
    use_buf[res] = 0;
    return res;
  }
  strcpy(use_buf, "msAdr=");
  res += tSz;

  if ((res + TonNpiAddress::_strSZ) >= buf_len) {
    use_buf[res] = 0;
    return res;
  }
  res += _msIsdn.toString(use_buf + res, true, buf_len - res);

  if ((res + sizeof(", ussData={%ub: ")) >= buf_len) {
    use_buf[res] = 0;
    return res;
  }
  res += (unsigned)snprintf(use_buf + res, buf_len - res, ", ussData={%ub: ", _ussData.length());

  uint16_t dLen = _ussData.toString(use_buf + res, buf_len - res);
  if ((res + dLen) >= buf_len) { //buffer is too small
    if ((res + 3) >= buf_len) {
      strcpy(use_buf + res, "...");
      dLen = 3;
    } else
      dLen = 0;
  }
  res += dLen;

  if ((res + 1) < buf_len) {
    use_buf[res++] = '}';
  }
  use_buf[res] = 0;
  return res;
}

void USSGatewayCmdAC::save(PacketBufferAC & out_buf) const throw(SerializerException)
{
  out_buf << (uint8_t)_ussData.getDataKind();
  if (_ussData.isPackedDCS())
    out_buf << _ussData.getDCS();
  out_buf << static_cast<const USSDataString::DataArray_t::base_type &>(_ussData.getDataBuf());
  out_buf << _msIsdn.toString();
}

void USSGatewayCmdAC::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  USSDataString::DataKind_e dKind = USSDataString::ussdUNKNOWN;
  {
    uint8_t flg;
    in_buf >> flg;
    dKind = USSDataString::uint2Kind(flg);
  }
  if (dKind == USSDataString::ussdUNKNOWN)
    throw SerializerException("invalid data flag value" , SerializerException::invObjData, NULL);

  uint8_t dcs = 0xFF;
  if (dKind == USSDataString::ussdPackedDCS)
    in_buf >> dcs;

  in_buf >> static_cast<USSDataString::DataArray_t::base_type &>(_ussData.getDataBuf());
  _ussData.setDataKind(dKind, dcs);

  smsc::util::TonNpiAddressString sadr;
  in_buf >> sadr;
  if (!_msIsdn.fromText(sadr.c_str()))
    throw SerializerException("invalid msisdn" , SerializerException::invObjData, NULL);
}

/* -------------------------------------------------------------------------- *
 * Own methods:
 * -------------------------------------------------------------------------- */
void USSGatewayCmdAC::setMsIsdn(const char * adr_str)
  throw (std::exception)
{
  if (!_msIsdn.fromText(adr_str))
    throw smsc::util::Exception("USSGatewayCmdAC: invalid msISDN %s", adr_str);
}

/* ************************************************************************** *
 * class USSRequestMessage implementation:
 * ************************************************************************** */
void USSRequestMessage::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  USSGatewayCmdAC::load(in_buf);
  in_buf >> _inSSN;

  smsc::util::TonNpiAddressString inSaddr;
  in_buf >> inSaddr;
  if (!_inAddr.fromText(inSaddr.c_str()))
    throw SerializerException("USSRequestMessage: invalid IN ISDN" , SerializerException::invObjData, NULL);

  in_buf >> _imsi;
}

void USSRequestMessage::save(PacketBufferAC & out_buf) const throw(SerializerException)
{
  USSGatewayCmdAC::save(out_buf);
  out_buf << _inSSN;
  out_buf << _inAddr.toString();
  out_buf << _imsi;
}

//"IN={%s:%u}, IMSI=%s, %s"
unsigned USSRequestMessage::log2str(char * use_buf, unsigned buf_len/* = _ussReq_strSZ*/) const
{
  unsigned res = 0;
  *use_buf = 0;

  strcpy(use_buf, "IN={"); res += 4;
  res += _inAddr.toString(use_buf + res, true);
  res += snprintf(use_buf + res, buf_len - res - 1, ":%u}", (unsigned)_inSSN);

  if (!_imsi.empty()) {
    strcpy(use_buf + res, ", IMSI="); res += 7;
    strcpy(use_buf + res, _imsi.c_str()); res += (unsigned)_imsi.length();
  }
  strcpy(use_buf + res, ", "); res += 2;
  res += USSGatewayCmdAC::log2str(use_buf + res, buf_len - res);
  return res;
}

/* ************************************************************************** *
 * class USSResultMessage implementation:
 * ************************************************************************** */
const char * USSResultMessage::status2str(Status_e st_val)
{
  switch (st_val) {
  case reqOK:         return "reqOK";
  case reqDUPLICATE:  return "reqDUPLICATE";
  case reqFAILED:     return "reqFAILED";
  case reqDENIED:     return "reqDENIED";
  default:;
  }
  return "undefined";
}

void USSResultMessage::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  uint16_t inStatus;
  in_buf >> inStatus;

  if ((_status = uint2Status(inStatus)) == reqOK)
    USSGatewayCmdAC::load(in_buf);
}

void USSResultMessage::save(PacketBufferAC & out_buf) const throw(SerializerException)
{
  out_buf << (uint16_t)_status;
  if (_status == reqOK)
    USSGatewayCmdAC::save(out_buf);
}

unsigned USSResultMessage::log2str(char * use_buf, unsigned buf_len/* = _ussRes_strSZ*/) const
{
  unsigned res = snprintf(use_buf, buf_len - 1, "status = %u(%s)", (unsigned)_status, status2str(_status));
  if (!_status) {
    use_buf[res++] = ',';
    use_buf[res++] = ' ';
    res += USSGatewayCmdAC::log2str(use_buf + res, buf_len - res);
  }
  use_buf[res] = 0;
  return res;
}

} //interaction
} //ussman
} //smsc

