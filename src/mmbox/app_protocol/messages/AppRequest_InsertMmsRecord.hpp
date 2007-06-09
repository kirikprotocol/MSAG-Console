#ifndef __BDB_AGENT_APPREQUEST_INSERTMMSRECORD_HPP__
# define __BDB_AGENT_APPREQUEST_INSERTMMSRECORD_HPP__ 1

#include <mmbox/app_protocol/ApplicationPackets.hpp>
#include <util/comm_comp/TEvent.hpp>

namespace mmbox {
namespace app_protocol {

class AppRequest_InsertMmsRecord : public RequestApplicationPacket,
                                   public smsc::util::comm_comp::TEvent<AppRequest_InsertMmsRecord> {
public:
  AppRequest_InsertMmsRecord();
  virtual void deserialize(const std::vector<uint8_t>& objectBuffer);
  virtual std::string toString() const;
  virtual packet_code_t getPacketCode() const;

  const std::string& getMmsId() const;
  const std::string& getAbonentAddress() const;
  uint32_t getMmsStatus() const;
  uint32_t getSentTime() const;

  virtual const smsc::util::comm_comp::AbstractEvent* createEvent();
  virtual ResponseApplicationPacket* createAppResponse(ResponseApplicationPacket::result_status_t status) const;

  static const packet_code_t packet_id = 1;
private:
  std::string _mmsId, _abonentAddress;
  uint32_t _mmsStatus, _sentTime;
  bool _isSetMmsId, _isSetAbonentAddress, _isSetMmsStatus, _isSetSentTime;
};

}}

#endif
