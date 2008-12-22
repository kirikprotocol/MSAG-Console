#ifndef __SCAG_PERS_MTPERS_PERSCOMMAND_H__
#define __SCAG_PERS_MTPERS_PERSCOMMAND_H__

#include <string>
#include <vector>
#include "core/buffers/Array.hpp"
#include "util/Exception.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pvss/base/Types.h"
#include "scag/pvss/base/Property.h"
#include "scag/pvss/base/PersServerResponse.h"
#include "scag/pers/util/Profile.h"
#include "scag/pers/util/AbntAddr.hpp"

#include "Connection.h"

namespace scag { namespace mtpers {

const uint32_t PACKET_LENGTH_SIZE = static_cast<uint32_t>(sizeof(uint32_t));

using std::string;
using namespace scag::pers::util;
using smsc::core::buffers::Array;
using smsc::util::Exception;

using smsc::logger::Logger;
using std::vector;

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

typedef scag::pvss::PersServerResponseType Response;

class PersCommandNotSupport: public Exception {};

static const string UPDATE_LOG = "U key=";

class PersCommand {
public:
  //PersCommand(vector<string>& logs):cmdId(scag::pers::util::PC_UNKNOWN), mod(0), dblogs(&logs), dblogMsg_(UPDATE_LOG) {};
  //PersCommand(PersCmd cmd, vector<string>& logs):cmdId(cmd), mod(0), dblogs(&logs), dblogMsg_(UPDATE_LOG) {};
  PersCommand():cmdId(scag::pers::util::PC_UNKNOWN), mod(0), dblogMsg_(UPDATE_LOG) {};
  PersCommand(PersCmd cmd):cmdId(cmd), mod(0), dblogMsg_(UPDATE_LOG) {};
  virtual ~PersCommand() {};
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
  bool notAbonentsProfile() const { return profileType != scag::pers::util::PT_ABONENT; };
  virtual void execCommand(Profile *pf) = 0;  
  virtual void flushLogs(Logger* log) const = 0;
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
  void execCommand(Profile *pf);
  void flushLogs(Logger* log) const;
private:
  PersCommand command_;
};

struct BatchPacket: public PersPacket {
  BatchPacket(Connection* connect, bool async, uint32_t sequenseNumber, time_t requestTime)
              :PersPacket(connect, async, sequenseNumber, requestTime), count_(0), transact_(false) {};
  ~BatchPacket(){};
  void deserialize(SerialBuffer &sb);
  void execCommand(Profile *pf);
  void flushLogs(Logger* log) const;
private:
  uint16_t count_;
  bool transact_;
  vector<PersCommand> batch_;
};

}//mtpers
}//scag

#endif

