#ifndef __BDB_AGENT_APPREQUEST_QUERYMMSRECORDSBYABONENTADDRESS_HPP__
# define __BDB_AGENT_APPREQUEST_QUERYMMSRECORDSBYABONENTADDRESS_HPP__ 1

#include <mmbox/app_protocol/ApplicationPackets.hpp>
#include <mmbox/app_protocol/messages/AppResponse_MmsRecordsResultSet.hpp>
#include <util/comm_comp/TEvent.hpp>

namespace mmbox {
namespace app_protocol {

class AppRequest_QueryMmsRecordsByAbonentAddress : public RequestApplicationPacket,
                                                   public smsc::util::comm_comp::TEvent<AppRequest_QueryMmsRecordsByAbonentAddress> {
public:
  AppRequest_QueryMmsRecordsByAbonentAddress();
  virtual void deserialize(const std::vector<uint8_t>& objectBuffer);
  virtual std::string toString() const;
  virtual packet_code_t getPacketCode() const;

  const std::string& getAbonentAddress() const;

  virtual const smsc::util::comm_comp::AbstractEvent* createEvent();
  virtual AppResponse_MmsRecordsResultSet* createAppResponse(ResponseApplicationPacket::result_status_t status) const;

  static const packet_code_t packet_id = 2;
private:
  std::string _abonentAddress;
  bool _isSetAbonentAddress;
};

}}

#endif
