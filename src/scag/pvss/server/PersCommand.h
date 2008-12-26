#ifndef _SCAG_PVSS_SERVER_PERSCOMMAND_H_
#define _SCAG_PVSS_SERVER_PERSCOMMAND_H_

#include <string>
#include <vector>
#include "core/buffers/Array.hpp"
#include "util/Exception.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pvss/base/Types.h"
#include "scag/pvss/base/Property.h"
#include "scag/pvss/base/PersServerResponse.h"
#include "scag/pvss/profile/Profile.h"
#include "scag/pvss/profile/AbntAddr.hpp"

#include "Connection.h"

namespace scag2 {
namespace pvss  {

const uint32_t PACKET_LENGTH_SIZE = static_cast<uint32_t>(sizeof(uint32_t));

using std::string;
using smsc::core::buffers::Array;
using smsc::util::Exception;

using smsc::logger::Logger;
using std::vector;

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

typedef scag::pvss::PersServerResponseType Response;

class PersCommandNotSupport: public Exception {};

class PvssCommand {
public:
  PvssCommand():cmdId(PC_UNKNOWN), mod(0) {};
  PvssCommand(PersCmd cmd):cmdId(cmd), mod(0) {};
  virtual ~PvssCommand() {};
  bool deserialize(SerialBuffer& sb);
  Response execute(Profile *pf, SerialBuffer&sb);
  const char* dbLog() const;
  void setCmdId(PersCmd cmd);

private:
  Response set(Profile *pf, SerialBuffer& sb);
  Response get(Profile *pf, SerialBuffer& sb);
  Response del(Profile *pf, SerialBuffer& sb);
  Response inc(Profile *pf, SerialBuffer& sb);
  Response incMod(Profile *pf, SerialBuffer& sb);
  Response incResult(Profile *pf, SerialBuffer& sb);
  Response incModProperty(Profile *pf, uint32_t& result);
  void createAddLogMsg(string const& key, string const& msg);
  void createUpdateLogMsg(string const& key, string const& msg);
  void createDelLogMsg(string const& key, string const& msg);
  void createExpireLogMsg(string const& key, string const& msg);

private:
  PersCmd cmdId;
  Property property;
  string propertyName;
  uint32_t mod;
  string dblogMsg_;
};

struct PersPacket {
  PersPacket(Connection* connect, bool async, uint32_t sequenseNumber, time_t requestTime);
  virtual ~PersPacket() {};
  virtual void deserialize(SerialBuffer& sb);
  bool notAbonentsProfile() const { return profileType != PT_ABONENT; };
  virtual void execCommand(Profile *pf, Logger* dblog) = 0;  
  //virtual void flushLogs(Logger* log) = 0;
  //virtual void flushLogs(Logger* log) const = 0;
  void createResponse(PersServerResponseType resp);
  void sendResponse();
  //uint32_t getResponseSize() const { return response_.GetSize(); };
  //const char* getResponseData() const { return response_.c_ptr(); };
  uint32_t getSequenceNumber() const { return sequenseNumber_; }

  ProfileType profileType;
  uint32_t intKey;
  string strKey;
  AbntAddr address;
  bool createProfile;
  bool rollback;
protected:
  Connection* connection_;
  SerialBuffer response_;
  vector<DbLog> dbLogs_;
private:
  uint32_t sequenseNumber_;
  bool asynch_;
  time_t requestTime_;
};

struct CommandPacket: public PersPacket {
  CommandPacket(Connection* connect, PersCmd cmdId, bool async, uint32_t sequenseNumber, time_t requestTime)
                :PersPacket(connect, async, sequenseNumber, requestTime), command_(cmdId)  {};
  ~CommandPacket(){};
  void deserialize(SerialBuffer &sb);
  void execCommand(Profile *pf, Logger* dblog);
  //void flushLogs(Logger* log);
private:
  PvssCommand command_;
};

struct BatchPacket: public PersPacket {
  BatchPacket(Connection* connect, bool async, uint32_t sequenseNumber, time_t requestTime)
              :PersPacket(connect, async, sequenseNumber, requestTime), count_(0), transact_(false) {};
  ~BatchPacket(){};
  void deserialize(SerialBuffer &sb);
  void execCommand(Profile *pf, Logger* dblog);
  //void flushLogs(Logger* log);
private:
  uint16_t count_;
  bool transact_;
  vector<PvssCommand> batch_;
};

}//pvss
}//scag2

#endif

