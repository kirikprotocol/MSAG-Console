#ifndef __SCAG_PERS_MTPERS_PERSCOMMAND_H__
#define __SCAG_PERS_MTPERS_PERSCOMMAND_H__

#include <string>
#include <vector>
#include "core/buffers/Array.hpp"
#include "util/Exception.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pers/util/Types.h"
#include "scag/pers/util/Property.h"
#include "scag/pers/util/Profile.h"
#include "scag/pers/util/AbntAddr.hpp"

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

typedef scag::pers::util::PersServerResponseType Response;

class PersCommandNotSupport: public Exception {};

class PersCommand {
public:
  PersCommand(vector<string>& logs):cmdId(scag::pers::util::PC_UNKNOWN), mod(0), dblogs(logs), logger(Logger::getInstance("pers.cmd")) {};
  PersCommand(PersCmd cmd, vector<string>& logs):cmdId(cmd), mod(0), dblogs(logs), logger(Logger::getInstance("pers.cmd")) {};
  virtual ~PersCommand() {};
  bool deserialize(SerialBuffer& sb);
  Response execute(Profile *pf, SerialBuffer&sb);

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
  string logMsg;
  uint32_t mod;
  vector<string>& dblogs;
  Logger* logger;
};

struct PersPacket {
  PersPacket():createProfile(false), rollback(false) {};
  virtual ~PersPacket() {};
  virtual void deserialize(SerialBuffer& sb);
  bool notAbonentsProfile() const { return profileType != scag::pers::util::PT_ABONENT; };
  void setPacketSize(SerialBuffer& sb) const;
  virtual void execCommand(Profile *pf, SerialBuffer& sb) = 0;
  void flushLogs(Logger* log) const;

  ProfileType profileType;
  uint32_t intKey;
  string strKey;
  AbntAddr address;
  bool createProfile;
  bool rollback;
protected:
  vector<string> dblogs;
};

struct CommandPacket: public PersPacket {
  CommandPacket(PersCmd cmdId):command(cmdId, dblogs)  {};
  ~CommandPacket(){};
  void deserialize(SerialBuffer &sb);
  void execCommand(Profile *pf, SerialBuffer& sb);

  PersCommand command;
};

struct BatchPacket: public PersPacket {
  BatchPacket():count(0), transact(false) {};
  ~BatchPacket(){};
  void deserialize(SerialBuffer &sb);
  void execCommand(Profile *pf, SerialBuffer& sb);

  uint16_t count;
  bool transact;
  vector<PersCommand> batch;
};

}//mtpers
}//scag

#endif

