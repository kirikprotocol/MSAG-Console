#include "AppRequest_QueryMmsRecordsByAbonentAddress.hpp"
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
    (AppRequest_QueryMmsRecordsByAbonentAddress::packet_id,
     new smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>::ProducerT<AppRequest_QueryMmsRecordsByAbonentAddress>());
  smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>& instance = smsc::util::comm_comp::PacketFactory<RequestApplicationPacket>::getInstance();
  return true;
}

static bool factory_was_inited = init_factory();

AppRequest_QueryMmsRecordsByAbonentAddress::AppRequest_QueryMmsRecordsByAbonentAddress()
  : _isSetAbonentAddress(false)
{}

void
AppRequest_QueryMmsRecordsByAbonentAddress::deserialize(const std::vector<uint8_t>& objectBuffer)
{
  RequestApplicationPacket::deserialize(objectBuffer);
  _deserializerBuf.ReadString<uint8_t>(_abonentAddress); _isSetAbonentAddress = true;
}

const std::string&
AppRequest_QueryMmsRecordsByAbonentAddress::getAbonentAddress() const
{
  if ( _isSetAbonentAddress ) return _abonentAddress;
  else throw smsc::util::CustomException("AppRequest_QueryMmsRecordsByAbonentAddress::getAbonentAddress::: abonentAddress doesn't set");
}

const smsc::util::comm_comp::AbstractEvent*
AppRequest_QueryMmsRecordsByAbonentAddress::createEvent() { return this; }

AppResponse_MmsRecordsResultSet*
AppRequest_QueryMmsRecordsByAbonentAddress::createAppResponse(ResponseApplicationPacket::result_status_t status) const
{
  return new AppResponse_MmsRecordsResultSet(status);
}

std::string
AppRequest_QueryMmsRecordsByAbonentAddress::toString() const
{
  try {
    std::ostringstream obuf;
    obuf << "abonentAddress=[" << getAbonentAddress()
         << "]";
    return obuf.str();
  } catch (smsc::util::CustomException& ex) {
    return std::string("object's field haven't been initialized [") + ex.what() + std::string("]");
  }
}

RequestApplicationPacket::packet_code_t
AppRequest_QueryMmsRecordsByAbonentAddress::getPacketCode() const
{
  return packet_id;
}

const RequestApplicationPacket::packet_code_t
AppRequest_QueryMmsRecordsByAbonentAddress::packet_id;

}}
