#include "AppRequest_InsertMmsRecord.hpp"
#include <util/comm_comp/PacketFactory.hpp>
#include <util/Exception.hpp>
#include <sstream>
#include <sys/types.h>

#include <stdio.h>

namespace mmbox {
namespace app_protocol {

static bool init_factory()
{
  smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>::getInstance().registerProduct
    (AppRequest_InsertMmsRecord::packet_id,
     new smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>::ProducerT<AppRequest_InsertMmsRecord>());

  smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>& instance = smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>::getInstance();

  return true;
}

static bool factory_was_inited = init_factory();

AppRequest_InsertMmsRecord::AppRequest_InsertMmsRecord() 
  : _mmsStatus(0), _sentTime(0),
    _isSetMmsId(false), _isSetAbonentAddress(false), _isSetMmsStatus(false), _isSetSentTime(false)
{}

void
AppRequest_InsertMmsRecord::deserialize(const std::vector<uint8_t>& objectBuffer)
{
  RequestApplicationPacket::deserialize(objectBuffer);
  _deserializerBuf.ReadString<uint16_t>(_mmsId); _isSetMmsId = true;
  _deserializerBuf.ReadString<uint8_t>(_abonentAddress); _isSetAbonentAddress = true;
  _mmsStatus = _deserializerBuf.ReadNetInt32(); _isSetMmsStatus = true;
  _sentTime = _deserializerBuf.ReadNetInt32(); _isSetSentTime = true;
}

const std::string&
AppRequest_InsertMmsRecord::getMmsId() const
{
  if ( _isSetMmsId ) return _mmsId;
  else throw smsc::util::CustomException("AppRequest_InsertMmsRecord::getMmsId::: mmsId doesn't set");
}

const std::string&
AppRequest_InsertMmsRecord::getAbonentAddress() const
{
  if ( _isSetAbonentAddress ) return _abonentAddress;
  else throw smsc::util::CustomException("AppRequest_InsertMmsRecord::getAbonentAddress::: abonentAddress doesn't set");
}

uint32_t
AppRequest_InsertMmsRecord::getMmsStatus() const
{
  if ( _isSetMmsStatus ) return _mmsStatus;
  else throw smsc::util::CustomException("AppRequest_InsertMmsRecord::getMmsStatus::: mmsStatus doesn't set");
}

uint32_t
AppRequest_InsertMmsRecord::getSentTime() const
{
  if ( _isSetSentTime ) return _sentTime;
  else throw smsc::util::CustomException("AppRequest_InsertMmsRecord::getSentTime::: sentTime doesn't set");
}

const smsc::util::comm_comp::AbstractEvent*
AppRequest_InsertMmsRecord::createEvent() { return this; }

ResponseApplicationPacket*
AppRequest_InsertMmsRecord::createAppResponse(ResponseApplicationPacket::result_status_t status) const
{
  return new ResponseApplicationPacket(getPacketCode(), status);
}

std::string
AppRequest_InsertMmsRecord::toString() const
{
  try {
    std::ostringstream obuf;
    obuf << "mmsId=[" << getMmsId()
         << "],abonentAddress=[" << getAbonentAddress()
         << "],mmsStatus=[" << getMmsStatus()
         << "],sendTime=[" << getSentTime()
         << "]";
    return obuf.str();
  } catch (smsc::util::CustomException& ex) {
    return std::string("object's field haven't been initialized [") + ex.what() + std::string("]");
  }
}

RequestApplicationPacket::packet_code_t
AppRequest_InsertMmsRecord::getPacketCode() const
{
  return packet_id;
}

const RequestApplicationPacket::packet_code_t
AppRequest_InsertMmsRecord::packet_id;

}}

