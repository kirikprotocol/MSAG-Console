#include "AppRequest_QueryMmsRecordsByAbonentAddressAndStatus.hpp"
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
    (AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::packet_id,
     new smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>::ProducerT<AppRequest_QueryMmsRecordsByAbonentAddressAndStatus>());
  smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>& instance = smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>::getInstance();
  return true;
}

static bool factory_was_inited = init_factory();

AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::AppRequest_QueryMmsRecordsByAbonentAddressAndStatus()
  : _isSetAbonentAddress(false), _isSetMmsStatus(false)
{}

void
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::deserialize(const std::vector<uint8_t>& objectBuffer)
{
  RequestApplicationPacket::deserialize(objectBuffer);
  _deserializerBuf.ReadString<uint8_t>(_abonentAddress); _isSetAbonentAddress = true;
  _mmsStatus = _deserializerBuf.ReadNetInt32(); _isSetMmsStatus = true;
}

const std::string&
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::getAbonentAddress() const
{
  if ( _isSetAbonentAddress ) return _abonentAddress;
  else throw smsc::util::CustomException("AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::getAbonentAddress::: abonentAddress doesn't set");
}

const uint32_t
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::getMmsStatus() const
{
  if ( _isSetMmsStatus ) return _mmsStatus;
  else throw smsc::util::CustomException("AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::getMmsStatus::: mmsStatus doesn't set");
}

const smsc::util::comm_comp::AbstractEvent*
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::createEvent() { return this; }

AppResponse_MmsRecordsResultSet*
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::createAppResponse(ResponseApplicationPacket::result_status_t status) const
{
  return new AppResponse_MmsRecordsResultSet(status);
}

std::string
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::toString() const
{
  try {
    std::ostringstream obuf;
    obuf << "abonentAddress=[" << getAbonentAddress()
         << "],mmsStatus=[" << getMmsStatus()
         << "]";
    return obuf.str();
  } catch (smsc::util::CustomException& ex) {
    return std::string("object's field haven't been initialized [") + ex.what() + std::string("]");
  }
}

RequestApplicationPacket::packet_code_t
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::getPacketCode() const
{
  return packet_id;
}

const RequestApplicationPacket::packet_code_t
AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::packet_id;

}}
