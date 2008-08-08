#ifndef __SCAG_PERS_CPERS_CMD_H__
#define __SCAG_PERS_CPERS_CMD_H__

#include <string>

#include <logger/Logger.h>

#include "Types.h"
#include "scag/util/storage/SerialBuffer.h"
#include "Profile.h"

namespace scag { namespace cpers {

const int ADDR_PREFIX_SIZE = 5;

using std::string;
using namespace scag::pers::CentralPersCmd;
using scag::pers::Profile;
using smsc::logger::Logger;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

namespace ownership {
  enum {
    UNKNOWN,
    OWNER,
    NOT_OWNER
  };
};

enum ProfileRespStatus {
  STATUS_OK,
  STATUS_ERROR,
  STATUS_LOCKED
};

namespace commands_name {
  extern const char* UNKNOWN;
  extern const char* GET_PROFILE;
  extern const char* PROFILE_RESP;
  extern const char* DONE;
  extern const char* DONE_RESP;
  extern const char* CHECK_OWN;
  extern const char* CHECK_OWN_RESP;
};

struct CPersCmd {
public:
  CPersCmd(uint8_t _cmd_id, const char* _cmd_name):id(_cmd_id), name(_cmd_name) {};
  CPersCmd(uint8_t _cmd_id, const string& _key, const char* _cmd_name)
          :id(_cmd_id), key(_key), name(_cmd_name) {};
  virtual ~CPersCmd() {};
  bool serialize(SerialBuffer& sb) const;
  virtual bool deserialize(SerialBuffer& sb);

public:
  uint8_t id;
  string key;
  string name;

protected:
  virtual void writeData(SerialBuffer& sb) const = 0;

};

struct GetProfileCmd : public CPersCmd {
public:
  GetProfileCmd():CPersCmd(GET_PROFILE, commands_name::GET_PROFILE) {};
  GetProfileCmd(const string& _key):CPersCmd(GET_PROFILE, _key, commands_name::GET_PROFILE) {};
  GetProfileCmd(SerialBuffer &sb):CPersCmd(GET_PROFILE, commands_name::GET_PROFILE) {
    deserialize(sb);
  };
  virtual bool deserialize(SerialBuffer &sb);

protected:
  virtual void writeData(SerialBuffer& sb) const {}
};

struct ProfileRespCmd : public CPersCmd {
public:
  ProfileRespCmd():CPersCmd(PROFILE_RESP, commands_name::PROFILE_RESP), profile(0),
                   must_del_profile(false), has_profile(0), status(STATUS_OK) {}
  ProfileRespCmd(const string& _key):CPersCmd(PROFILE_RESP, _key, commands_name::PROFILE_RESP),
                                     profile(0), has_profile(0), status(STATUS_OK) ,must_del_profile(false) {}
  ProfileRespCmd(SerialBuffer &sb):CPersCmd(PROFILE_RESP, commands_name::PROFILE_RESP), profile(0),
                                   has_profile(0), status(STATUS_OK), must_del_profile(false) {
    deserialize(sb);
  }
  ~ProfileRespCmd();
  virtual bool deserialize(SerialBuffer &sb);
  void setProfile(Profile* pf);
  Profile* getProfile();
  void setStatus(ProfileRespStatus _status) { status = _status;};
  ProfileRespStatus getStatus() const { return static_cast<ProfileRespStatus>(status); };
  bool isOk() const { return status == STATUS_OK ? true : false; };

public:
  uint8_t status;

protected:
  virtual void writeData(SerialBuffer &sb) const;

private:  
  uint8_t has_profile;
  Profile *profile;
  bool must_del_profile;

};

struct DoneCmd : public CPersCmd {
public:
  DoneCmd():CPersCmd(DONE, commands_name::DONE), is_ok(0) {};
  DoneCmd(uint8_t _is_ok, const string& _key):CPersCmd(DONE, _key, commands_name::DONE), is_ok(_is_ok) {};
  DoneCmd(SerialBuffer &sb):CPersCmd(DONE, commands_name::DONE) {
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
  DoneRespCmd():CPersCmd(DONE_RESP, commands_name::DONE_RESP), is_ok(0) {};
  DoneRespCmd(uint8_t _is_ok, const string& _key):CPersCmd(DONE_RESP, _key, commands_name::DONE_RESP),
                                                  is_ok(_is_ok) {};
  DoneRespCmd(SerialBuffer &sb):CPersCmd(DONE_RESP, commands_name::DONE_RESP) {
    deserialize(sb);
  };
  virtual bool deserialize(SerialBuffer &sb);

public:
  uint8_t is_ok;

protected:
  virtual void writeData(SerialBuffer &sb) const;
};

struct CheckOwnCmd : public CPersCmd {
public:
  CheckOwnCmd():CPersCmd(CHECK_OWN, commands_name::CHECK_OWN) {};
  CheckOwnCmd(const string& _key):CPersCmd(CHECK_OWN, _key, commands_name::CHECK_OWN) {};
  CheckOwnCmd(SerialBuffer &sb):CPersCmd(CHECK_OWN, commands_name::CHECK_OWN) {
    deserialize(sb);
  };
  virtual bool deserialize(SerialBuffer &sb) {
    return CPersCmd::deserialize(sb);
  }

protected:
  virtual void writeData(SerialBuffer& sb) const {}
};

struct CheckOwnRespCmd : public CPersCmd {
public:
  CheckOwnRespCmd():CPersCmd(CHECK_OWN_RESP, commands_name::CHECK_OWN_RESP),
                    result(ownership::UNKNOWN) {};
  CheckOwnRespCmd(uint8_t _result, const string& _key)
                 :CPersCmd(CHECK_OWN_RESP, _key, commands_name::CHECK_OWN_RESP), result(_result) {};
  CheckOwnRespCmd(SerialBuffer &sb):CPersCmd(CHECK_OWN_RESP, commands_name::CHECK_OWN_RESP) {
    deserialize(sb);
  };
  virtual bool deserialize(SerialBuffer &sb);

public:
  uint8_t result;

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

