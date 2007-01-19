static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "util/vformat.hpp"
using smsc::util::format;

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
  _logger = smsc::logger::Logger::getInstance("smsc.ussbalance");
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
  2b        4b        1b       1b        up to 160b   1b     up to 32                  1b     1b    up to 32
------   ---------   -----  ----------   ----------   ---  -----------------------   ------   ---   --------
 cmdId : requestId  : DCS : ussDataLen  :  ussData  : len : ms ISDN address string : IN SSN : len : IN ISDN
                    |                                                                                       |
                    -------- processed by load() method ----------------------------------------------------


Result (PROCESS_USS_RESULT_TAG):
--------

  2b        4b          2b      1b       1b        up to 160b   1b     up to 32
------   ---------   -------   ---   ----------   ----------   ---  -----------------------
 cmdId : requestId  : status : DCS : ussDataLen  :  ussData  : len : ms ISDN address string
                             |                                                             |
                              -------- present if status == 0 -----------------------------
                    |                                                                      |
                    -------- processed by save() method ------------------------------------
*/
//NOTE: SerializerUSS doesn't provide partial deserialization of packets
SerializablePacketAC * SerializerUSS::deserialize(std::auto_ptr<ObjectBuffer>& p_in)
        throw(SerializerException)
{
  return deserialize(*(p_in.get()));
}

SerializablePacketAC* SerializerUSS::deserialize(ObjectBuffer& in)
        throw(SerializerException)
{
  smsc_log_debug(_logger, "SerializerUSS::deserialize::: enter it");
  unsigned short objectId;
  uint32_t       reqId;
  try {
    in >> objectId;
    smsc_log_debug(_logger, "SerializerUSS::deserialize::: read objId=%d", objectId);
    in >> reqId;
    smsc_log_debug(_logger, "SerializerUSS::deserialize::: read reqId=%d", reqId);
  } catch (SerializerException & exc) {
    throw SerializerException("USSrlzr: invalid packet structure",
                              SerializerException::invPacket, exc.what());
  }

  std::auto_ptr<SerializableObjectAC> obj(create(objectId));
  if (!obj.get())
    throw SerializerException("USSrlzr: illegal command", SerializerException::invObject);
  smsc_log_debug(_logger, "SerializerUSS::deserialize::: load SerializableObjectAC");
  obj->load(in);  //throws

  std::auto_ptr<USSPacketAC> pck(new USSPacketAC(reqId));
  pck->assignObj(0, obj.release());
  smsc_log_debug(_logger, "SerializerUSS::deserialize::: return new packet");
  return pck.release();
}

/* ************************************************************************** *
 * class USSMessageAC implementation:
 * ************************************************************************** */
void USSMessageAC::save(ObjectBuffer& out) const
{
  out << _dCS;
  out << _ussData;
  out << _msAdr.toString();
}

void USSMessageAC::load(ObjectBuffer& in) throw(SerializerException)
{
  smsc_log_debug(_logger, "USSMessageAC::load::: enter it");
  in >> _dCS;
  smsc_log_debug(_logger, "USSMessageAC::load::: read dcs=%d", _dCS);
  in >> _ussData;
  smsc_log_debug(_logger, "USSMessageAC::load::: read ussdata");
  std::string sadr;
  in >> sadr;
  smsc_log_debug(_logger, "USSMessageAC::load::: read sadr=%s", sadr.c_str());
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
}

void USSMessageAC::setUSSData(const unsigned char * data, unsigned size)
{
  _ussData.clear();
  _ussData.insert(_ussData.begin(), data, data + size);
  _dCS = USSMAN_LATIN1_DCS;
}

void USSMessageAC::setUSSData(const USSDATA_T& data)
{
  setUSSData(&data[0], data.size());
}

void USSMessageAC::setMSISDNadr(const char * adrStr) throw (CustomException)
{
  if (!_msAdr.fromText(adrStr))
    throw CustomException("invalid msisdn: %s", adrStr);
}

/* ************************************************************************** *
 * class USSResultMessage implementation:
 * ************************************************************************** */
bool  USSResultMessage::getUSSDataAsLatin1Text(std::string & str)
{
  CBS_DCS    parsedDCS;

  if (parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcGSM7Bit) {
    unsigned ussdLen = unpack7BitPadded2Text(&_ussData[0], _ussData.size(), str);
    //skip language prefix
    if (parsedDCS.lngPrefix == CBS_DCS::lng4GSM7Bit)
      str.erase(0, 3);
    return true;
  }
  return false;
}

void USSRequestMessage::load(ObjectBuffer& in) throw(SerializerException)
{
  smsc_log_debug(_logger, "USSRequestMessage::load::: Enter it");
  USSMessageAC::load(in);
  in >> _inSSN;
  smsc_log_debug(_logger, "USSRequestMessage::load::: read inSSN=%d", _inSSN);
  std::string inSaddr;
  in >> inSaddr;
  smsc_log_debug(_logger, "USSRequestMessage::load::: read inSaddr=%s", inSaddr.c_str());
  if (!_inAddr.fromText(inSaddr.c_str()))
    throw SerializerException("invalid IN isdn" , SerializerException::invObjData, NULL);
}

void USSRequestMessage::save(ObjectBuffer& out) const
{
  USSMessageAC::save(out);
  out << _inSSN;
  out << _inAddr.toString();
}

void USSResultMessage::load(ObjectBuffer& in) throw(SerializerException)
{
  smsc_log_debug(_logger, "USSResultMessage::load::: enter it");
  in >> _status;
  smsc_log_debug(_logger, "USSResultMessage::load::: read status=%d", _status);
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

} //interaction
} //inman
} //smsc

