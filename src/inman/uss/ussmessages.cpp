#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <assert.h>
#include <string>
#include <stdexcept>

#include "util/vformat.hpp"
using smsc::util::format;
#include "util/BinDump.hpp"

#include "inman/uss/ussmessages.hpp"
#include "inman/common/cvtutil.hpp"

using std::runtime_error;
using smsc::cbs::CBS_DCS;
using smsc::cbs::parseCBS_DCS;
using smsc::cvtutil::unpack7BitPadded2Text;

namespace smsc  {
namespace inman {
namespace interaction {

/* ************************************************************************** *
 * class SerializerUSS implementation:
 * ************************************************************************** */

SerializerUSS::SerializerUSS()
{
  registerProduct(USS2CMD::PROCESS_USS_REQUEST_TAG, new ProducerT<USSRequestMessage>());
  registerProduct(USS2CMD::PROCESS_USS_RESULT_TAG, new ProducerT<USSResultMessage>());
  _logger = smsc::logger::Logger::getInstance("smsc.ussman");
}

SerializerUSS* SerializerUSS::getInstance()
{
  static SerializerUSS instance;
  return &instance;
}

/*
 * USS messages are transferred with length prefix and have the following
 * serialization formats:

Request (PROCESS_USS_REQUEST_TAG):
--------l
  2b        4b       1b  1b       1b        up to 160b   1b     up to 32    1b     1b    up to 32 1b     up to 32
------   ---------   -- ----  ----------   ----------   ---  -----------  ------   ---   -------- ----   -------
 cmdId : requestId : flg DCS : ussDataLen  :  ussData  : len : ms ISDN  : IN SSN : len : IN ISDN IMSI len  IMSI
                    |                                                                                           |
                     ------- processed by load() method -------------------------------------------------------

Result (PROCESS_USS_RESULT_TAG):
--------

  2b        4b          2b     1b   1b       1b        up to 160b   1b     up to 32
------   ---------   -------   --  ---   ----------   ----------   ---  -----------------------
 cmdId : requestId  : status : flg DCS : ussDataLen  :  ussData  : len : ms ISDN address string
                             |                                                             |
                              -------- present if status == 0 -----------------------------
                    |                                                                      |
                     ------- processed by save() method -----------------------------------

if flg == 0 then DCS presents and ussData contains binary data, otherwise flg indicates text string encoding and 
DCS is missed.
There are next flg values for text string encoding:
0x01 - ASCII LATIN
0x02 - UCS2
*/
//NOTE: SerializerUSS doesn't provide partial deserialization of packets
SerializablePacketAC * SerializerUSS::deserialize(std::auto_ptr<ObjectBuffer>& p_in) const
        throw(SerializerException)
{
  return deserialize(*(p_in.get()));
}

SerializablePacketAC* SerializerUSS::deserialize(ObjectBuffer& in) const
        throw(SerializerException)
{
//  smsc_log_debug(_logger, "SerializerUSS::deserialize::: enter it");
  unsigned short objectId;
  uint32_t       reqId;
  try {
    in >> objectId;
//    smsc_log_debug(_logger, "SerializerUSS::deserialize::: read objId=%d", objectId);
    in >> reqId;
//    smsc_log_debug(_logger, "SerializerUSS::deserialize::: read reqId=%d", reqId);
  } catch (SerializerException & exc) {
    throw SerializerException("USSrlzr: invalid packet structure",
                              SerializerException::invPacket, exc.what());
  }

  std::auto_ptr<SerializableObjectAC> obj(create(objectId));
  if (!obj.get())
    throw SerializerException("USSrlzr: illegal command", SerializerException::invObject);
//  smsc_log_debug(_logger, "SerializerUSS::deserialize::: load SerializableObjectAC");
  obj->load(in);  //throws

  std::auto_ptr<USSPacketAC> pck(new USSPacketAC(reqId));
  pck->assignObj(0, *obj.release());
//  smsc_log_debug(_logger, "SerializerUSS::deserialize::: return new packet");
  return pck.release();
}

/* ************************************************************************** *
 * class USSMessageAC implementation:
 * ************************************************************************** */
void USSMessageAC::save(ObjectBuffer& out) const
{
  out << _flg;
  if ( _flg == PREPARED_USS_REQ )
    out << _dCS;
  out << _ussData;
  out << _msAdr.toString();
}

void USSMessageAC::load(ObjectBuffer& in) throw(SerializerException)
{
  in >> _flg;
  if ( _flg != PREPARED_USS_REQ &&
       _flg != LATIN1_USS_TEXT &&
       _flg != UCS2_USS_TEXT ) 
    throw SerializerException("invalid flg value" , SerializerException::invObjData, NULL);

  if ( _flg == PREPARED_USS_REQ ) {
    in >> _dCS;
    _dCS_wasRead = true;
  }
  in >> _ussData;
  if ( _flg == LATIN1_USS_TEXT )
    _latin1Text.assign((char*)&_ussData[0], _ussData.size());

  std::string sadr;
  in >> sadr;
  if (!_msAdr.fromText(sadr.c_str()))
    throw SerializerException("invalid msisdn" , SerializerException::invObjData
, NULL);
}

/* ************************************************************************** *
 * Own methods:
 * ************************************************************************** */

void USSMessageAC::setRAWUSSData(unsigned char dcs, const USSDATA_T& ussdata)
{
  _ussData.clear();
  _ussData = ussdata;
  _dCS = dcs;
  _flg = PREPARED_USS_REQ;
}

void USSMessageAC::setUSSData(const char * data, unsigned size)
{
  _ussData.clear();
  _ussData.insert(_ussData.begin(), (unsigned char*)data, (unsigned char*)data + size);
  _flg = LATIN1_USS_TEXT;
}

void USSMessageAC::setUCS2USSData(const std::vector<uint8_t>& ucs2)
{
  _ussData.clear();
  _ussData = ucs2;
  _flg = UCS2_USS_TEXT;
}

void USSMessageAC::setMSISDNadr(const char * adrStr) throw (CustomException)
{
  if (!_msAdr.fromText(adrStr))
    throw CustomException("invalid msisdn: %s", adrStr);
}

std::string USSMessageAC::toString() const
{
  return format("adr=%s,flg=%u%s,ussData=%s",
                _msAdr.toString().c_str(), (unsigned)_flg,
                _dCS_wasRead ? format(",dCS=%u", (unsigned)_dCS).c_str(): "",
                DumpHex(_ussData.size(), &_ussData[0]).c_str()
                );
}


/* ************************************************************************** *
 * class USSRequestMessage implementation:
 * ************************************************************************** */
void USSRequestMessage::load(ObjectBuffer& in) throw(SerializerException)
{
  USSMessageAC::load(in);
  in >> _inSSN;
  std::string inSaddr;
  in >> inSaddr;
  if (!_inAddr.fromText(inSaddr.c_str()))
    throw SerializerException("invalid IN isdn" , SerializerException::invObjData, NULL);

  in >> _imsi;
  if ( _imsi.length() > 32 )
    throw SerializerException(format("invalid IMSI len=%d", _imsi.length()).c_str(), SerializerException::invObjData, NULL);
}

void USSRequestMessage::save(ObjectBuffer& out) const
{
  USSMessageAC::save(out);
  out << _inSSN;
  out << _inAddr.toString();
}

std::string USSRequestMessage::toString() const
{
  std::string obuf = USSMessageAC::toString();
  format(obuf, ",inSSN=%u,inAddr=%s", (unsigned)_inSSN, _inAddr.toString().c_str());
  return obuf;
}


/* ************************************************************************** *
 * class USSResultMessage implementation:
 * ************************************************************************** */
bool  USSResultMessage::getUSSDataAsLatin1Text(std::string & str)
{
  CBS_DCS    parsedDCS;

  if (parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcGSM7Bit) {
    //unsigned ussdLen = unpack7BitPadded2Text(&_ussData[0], (unsigned)_ussData.size(), str);
    //skip language prefix
    if (parsedDCS.lngPrefix == CBS_DCS::lng4GSM7Bit)
      str.erase(0, 3);
    return true;
  }
  return false;
}

void USSResultMessage::load(ObjectBuffer& in) throw(SerializerException)
{
  in >> _status;
  if (_status)
    return;
  USSMessageAC::load(in);
}

void USSResultMessage::save(ObjectBuffer& out) const
{
  out << _status;
  if (_status)
    return;
  USSMessageAC::save(out);
}

std::string USSResultMessage::toString() const
{
  std::string obuf = format("status=%u", _status);
  if (!_status) {
    obuf += ",";
    obuf += USSMessageAC::toString();
  }
  return obuf;
}


} //interaction
} //inman
} //smsc

