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
  CPersCmd(uint8_t _cmd_id):cmd_id(_cmd_id) {};
  CPersCmd(uint8_t _cmd_id, const string& _key):cmd_id(_cmd_id), key(_key) {};
  virtual ~CPersCmd() {};
  bool serialize(SerialBuffer& sb) const;
  virtual bool deserialize(SerialBuffer& sb);

public:
  uint8_t cmd_id;
  string key;

protected:
  virtual void writeData(SerialBuffer& sb) const = 0;

};

struct GetProfileCmd : public CPersCmd {
public:
  GetProfileCmd():CPersCmd(GET_PROFILE) {};
  GetProfileCmd(const string& _key):CPersCmd(GET_PROFILE, _key) {};
  GetProfileCmd(SerialBuffer &sb):CPersCmd(GET_PROFILE) {
    deserialize(sb);
  };
  virtual bool deserialize(SerialBuffer &sb);

protected:
  virtual void writeData(SerialBuffer& sb) const {}
};

struct ProfileRespCmd : public CPersCmd {
public:
  ProfileRespCmd():CPersCmd(PROFILE_RESP), profile(0), is_ok(1) {}
  ProfileRespCmd(const string& _key):CPersCmd(PROFILE_RESP, _key),
                                     profile(0), is_ok(1) {}
  ProfileRespCmd(SerialBuffer &sb):CPersCmd(PROFILE_RESP), profile(0), is_ok(0) {
    deserialize(sb);
  }
  ~ProfileRespCmd();
  virtual bool deserialize(SerialBuffer &sb);

public:
  uint8_t is_ok;
  Profile *profile;

protected:
  virtual void writeData(SerialBuffer &sb) const;

};

struct DoneCmd : public CPersCmd {
public:
  DoneCmd():CPersCmd(DONE), is_ok(0) {};
  DoneCmd(uint8_t _is_ok, const string& _key):CPersCmd(DONE, _key), is_ok(_is_ok) {};
  DoneCmd(SerialBuffer &sb):CPersCmd(DONE) {
    deserialize(sb);
  };
  virtual bool deserialize(SerialBuffer &sb);

public:
  uint8_t is_ok;

protected:
  virtual void writeData(SerialBuffer &sb) const;
};

struct DoneRespCmd : public CPersCmd {
public:
  DoneRespCmd():CPersCmd(DONE_RESP), is_ok(0) {};
  DoneRespCmd(uint8_t _is_ok, const string& _key):CPersCmd(DONE_RESP, _key), is_ok(_is_ok) {};
  DoneRespCmd(SerialBuffer &sb):CPersCmd(DONE_RESP) {
    deserialize(sb);
  };
  virtual bool deserialize(SerialBuffer &sb);

public:
  uint8_t is_ok;

protected:
  virtual void writeData(SerialBuffer &sb) const;
};

struct LoginCmd {
public:
  LoginCmd():cmd_id(LOGIN), rp_id(0) {};
  LoginCmd(uint32_t _rp_id, const string& _rp_psw)
          :cmd_id(LOGIN), rp_id(_rp_id), rp_psw(_rp_psw) {};
  LoginCmd(SerialBuffer& sb) { 
    deserialize(sb); 
  }
  bool serialize(SerialBuffer& sb) const;
  bool deserialize(SerialBuffer& sb);

public:
  uint32_t rp_id;
  string rp_psw;
  uint8_t cmd_id;
};


}//pers
}//scag

#endif

