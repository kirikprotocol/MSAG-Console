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
//using scag::pers::Profile;
//using scag::pers::ProfileType;
//using scag::pers::PersCmd;
//using scag::pers::Property;
using smsc::core::buffers::Array;
using smsc::util::Exception;
//using scag::pers::AbntAddr;

using smsc::logger::Logger;
using std::vector;

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

typedef scag::pers::util::PersServerResponseType Response;

class PersCommandNotSupport: public Exception {};

class PersCommand {
public:
  PersCommand():cmdId(scag::pers::util::PC_UNKNOWN), mod(0) {};
  PersCommand(PersCmd cmd):cmdId(cmd), mod(0) {};
  ~PersCommand() {};
  bool deserialize(SerialBuffer& sb);
  Response execute(Profile *pf, SerialBuffer&sb, Logger* dblog, const string& key);

private:
  Response set(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);
  Response get(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);
  Response del(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);
  Response inc(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);
  Response incMod(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);
  Response incResult(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);
  uint32_t incModProperty(Profile *pf, Logger* dblog, const string& key);

private:
  PersCmd cmdId;
  Property property;
  string propertyName;
  uint32_t mod;
};

struct PersPacket {
  PersPacket():createProfile(false) {};
  virtual ~PersPacket() {};
  virtual void deserialize(SerialBuffer& sb);
  bool notAbonentsProfile() const { return profileType != scag::pers::util::PT_ABONENT; };
  void setPacketSize(SerialBuffer& sb) const;
  virtual void execCommand(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) = 0;

  ProfileType profileType;
  uint32_t intKey;
  string strKey;
  AbntAddr address;
  bool createProfile;
};

struct CommandPacket: public PersPacket {
  CommandPacket(PersCmd cmdId):command(cmdId) {};
  ~CommandPacket(){};
  void deserialize(SerialBuffer &sb);
  void execCommand(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);

  PersCommand command;
};

struct BatchPacket: public PersPacket {
  BatchPacket():count(0), transact(false) {};
  ~BatchPacket(){};
  void deserialize(SerialBuffer &sb);
  void execCommand(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key);

  uint16_t count;
  bool transact;
  vector<PersCommand> batch;
};

}//mtpers
}//scag

#endif

