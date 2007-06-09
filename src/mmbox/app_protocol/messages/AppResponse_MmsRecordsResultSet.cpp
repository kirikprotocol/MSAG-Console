#include "AppResponse_MmsRecordsResultSet.hpp"
#include "AppRequest_QueryMmsRecordsByAbonentAddress.hpp"
#include <sstream>
#include <util/BufferSerialization.hpp>

namespace mmbox {
namespace app_protocol {

AppResponse_MmsRecordsResultSet::AppResponse_MmsRecordsResultSet(result_status_t status)
  : ResponseApplicationPacket(AppRequest_QueryMmsRecordsByAbonentAddress().getPacketCode(), status) {}

void
AppResponse_MmsRecordsResultSet::serialize(std::vector<uint8_t>& objectBuffer) const
{
  ResponseApplicationPacket::serialize(objectBuffer);
  smsc::util::SerializationBuffer outputBuf;

  for (result_set_t::const_iterator iter=_resultRecords.begin(), end_iter=_resultRecords.end();
       iter != end_iter; ++iter) {
    outputBuf.WriteString<uint16_t>(iter->getMmsId());
    outputBuf.WriteString<uint8_t>(iter->getAbonentAddress());
    outputBuf.WriteNetInt32(iter->getMmsStatus());
    outputBuf.WriteNetInt32(iter->getSentTime());
  }
  objectBuffer.insert(objectBuffer.end(), (uint8_t*)outputBuf.getBuffer(), (uint8_t*)outputBuf.getBuffer() + outputBuf.getBufferSize());
}

std::string
AppResponse_MmsRecordsResultSet::toString() const
{
  std::ostringstream obuf;
  obuf << ResponseApplicationPacket::toString()
       << ", resultRecords=[";
  for (result_set_t::const_iterator iter=_resultRecords.begin(), begin_iter=iter, end_iter=_resultRecords.end(); 
       iter != end_iter; ++iter) {
    if ( iter == begin_iter )
      obuf << "[" << iter->toString() << "]";
    else
      obuf << ",[" << iter->toString() << "]";
  }
  return obuf.str();
}

void
AppResponse_MmsRecordsResultSet::addRecord(const mmbox::app_specific_db::MmsDbRecord& record)
{
  _resultRecords.push_back(record);
}

}}
