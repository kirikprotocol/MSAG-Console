#ifndef __SCAG_PERS_CPERS_CMD_H__
#define __SCAG_PERS_CPERS_CMD_H__

#include <string>

#include <logger/Logger.h>

#include "Types.h"
#include "SerialBuffer.h"
#include "Profile.h"

namespace scag { namespace cpers {

using std::string;
using namespace scag::pers::CentralPersCmd;
using scag::pers::Profile;
using smsc::logger::Logger;

struct CPersCmd {
public:
  CPersCmd():cmd_id(UNKNOWN) {
    logger = Logger::getInstance("cperscmd");
  };
  CPersCmd(uint8_t _cmd_id):cmd_id(_cmd_id) {};
  virtual ~CPersCmd() {};
  virtual bool serialize(SerialBuffer& sb) const;
  virtual bool deserialize(SerialBuffer& sb);

public:
  uint8_t cmd_id;

protected:
  Logger* logger;
};

struct LoginCmd : public CPersCmd {
public:
  LoginCmd():CPersCmd(LOGIN), rp_id(0) {};
  LoginCmd(uint32_t _rp_id, const string& _rp_psw)
          :CPersCmd(LOGIN), rp_id(_rp_id), rp_psw(_rp_psw) {};
  LoginCmd(SerialBuffer& sb) { 
    deserialize(sb); 
  }
  bool serialize(SerialBuffer& sb) const;
  bool deserialize(SerialBuffer& sb);

public:
  uint32_t rp_id;
  string rp_psw;
};

struct GetProfileCmd : public CPersCmd {
public:
  GetProfileCmd():CPersCmd(GET_PROFILE) {};
  GetProfileCmd(const string& _key):CPersCmd(GET_PROFILE), key(_key) {};
  virtual bool serialize(SerialBuffer &sb) const;
  virtual bool deserialize(SerialBuffer &sb);

public:
  string key;
};

struct DoneCmd : public CPersCmd {
public:
  DoneCmd():CPersCmd(DONE), is_ok(0) {};
  DoneCmd(uint8_t _is_ok):CPersCmd(DONE), is_ok(_is_ok) {};
  virtual bool serialize(SerialBuffer &sb) const;
  virtual bool deserialize(SerialBuffer &sb);

public:
  string key;
  uint8_t is_ok;
};

struct DoneRespCmd : public CPersCmd {
public:
  DoneRespCmd():CPersCmd(DONE_RESP), is_ok(0) {};
  DoneRespCmd(uint8_t _is_ok):CPersCmd(DONE_RESP), is_ok(_is_ok) {};
  virtual bool serialize(SerialBuffer &sb) const;
  virtual bool deserialize(SerialBuffer &sb);

public:
  string key;
  uint8_t is_ok;
};

struct ProfileRespCmd : public CPersCmd {
public:
  ProfileRespCmd():CPersCmd(PROFILE_RESP), profile(0), is_ok(1) {}
  ProfileRespCmd(const string& _key):CPersCmd(PROFILE_RESP), key(_key),
                                     profile(0), is_ok(1) {}
  ~ProfileRespCmd();
  virtual bool serialize(SerialBuffer &sb) const;
  virtual bool deserialize(SerialBuffer &sb);

public:
  uint8_t is_ok;
  string key;
  Profile *profile;
};

}//pers
}//scag

#endif

