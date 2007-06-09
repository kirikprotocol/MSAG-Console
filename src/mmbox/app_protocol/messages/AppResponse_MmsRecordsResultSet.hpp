#ifndef __SMSC_BDB_AGENT_APPRESPONSE_MMSRECORDSRESULTSET_HPP__
# define __SMSC_BDB_AGENT_APPRESPONSE_MMSRECORDSRESULTSET_HPP__ 1

# include <string>
# include <list>
# include <sys/types.h>
# include <mmbox/app_protocol/ApplicationPackets.hpp>
# include <mmbox/app_specific_db/MmsRecord.hpp>

namespace mmbox {
namespace app_protocol {

class AppResponse_MmsRecordsResultSet : public ResponseApplicationPacket {
public:
  AppResponse_MmsRecordsResultSet(result_status_t status);
  virtual void serialize(std::vector<uint8_t>& objectBuffer) const;
  virtual std::string toString() const;

  void addRecord(const mmbox::app_specific_db::MmsDbRecord& record);
private:
  typedef std::list<mmbox::app_specific_db::MmsDbRecord> result_set_t;
  std::list<mmbox::app_specific_db::MmsDbRecord> _resultRecords;
};

}}

#endif
