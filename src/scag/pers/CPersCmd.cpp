#include "CPersCmd.h"
#include "sms/sms.h"

namespace scag { namespace cpers {

using smsc::sms::Address;

namespace commands_name {
  const char* UNKNOWN = "UNKNOWN";
  const char* GET_PROFILE = "GET_PROFILE";
  const char* PROFILE_RESP = "PROFILE_RESP";
  const char* DONE = "DONE";
  const char* DONE_RESP = "DONE_RESP";
  const char* CHECK_OWN = "CHECK_OWN";
  const char* CHECK_OWN_RESP = "CHECK_OWN_RESP";
};


bool CPersCmd::serialize(SerialBuffer& sb) const {
  //sb.setPos(4);
  if (sb.GetSize() == 0) {
    sb.SetPos(4);
  } else {
    sb.SetPos(sb.GetSize());
  }
  sb.WriteInt8(id);
  sb.WriteString(key.c_str());
  writeData(sb);
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  sb.SetPos(sb.length());
  return true;
}

bool CPersCmd::deserialize(SerialBuffer& sb) {
  //sb.setPos(4);
  //cmd_id = sb.ReadInt8();
  string str_key;
  sb.ReadString(str_key);
  Address addr(str_key.c_str());
  if (str_key[0] != '.') {
    addr.setNumberingPlan(1);
    addr.setNumberingPlan(1);
  }
  key = addr.toString();
  return true;
}

bool GetProfileCmd::deserialize(SerialBuffer &sb) {
  return CPersCmd::deserialize(sb);
}

void ProfileRespCmd::writeData(SerialBuffer &sb) const {
  sb.WriteInt8(is_ok);
  if (!is_ok) {
    return;
  }
  sb.WriteInt8(has_profile);  
  if (profile) {
    profile->Serialize(sb);
  }
}

bool ProfileRespCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  if (must_del_profile && profile) {
    delete profile;
    profile = 0;
  }
  is_ok = sb.ReadInt8();
  if (!is_ok) {
    return true;
  }
  has_profile = sb.ReadInt8();
  if (has_profile) {
    profile = new Profile(key);
    profile->Deserialize(sb);
    must_del_profile = true;
  }
  return true;
}

void ProfileRespCmd::setProfile(Profile* pf) {
  if (!pf) {
    return;
  }
  if (must_del_profile && profile) {
    delete profile;
  }
  profile = pf;
  has_profile = 1;
  must_del_profile = false;
}

Profile* ProfileRespCmd::getProfile() {
  return profile;
}

ProfileRespCmd::~ProfileRespCmd() {
  if (must_del_profile && profile) {
    delete profile;
  }
}

void DoneCmd::writeData(SerialBuffer &sb) const {
  sb.WriteInt8(is_ok);
}

bool DoneCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  is_ok = sb.ReadInt8();
  return true;
}

void DoneRespCmd::writeData(SerialBuffer &sb) const {
  sb.WriteInt8(is_ok);
}

bool DoneRespCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  is_ok = sb.ReadInt8();
  return true;
}

void CheckOwnRespCmd::writeData(SerialBuffer &sb) const {
  sb.WriteInt8(result);
}

bool CheckOwnRespCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  result = sb.ReadInt8();
  return true;
}

bool LoginCmd::serialize(SerialBuffer& sb) const {
  sb.setPos(4);
  sb.WriteInt8(cmd_id);
  sb.WriteInt32(rp_id);
  sb.WriteString(rp_psw.c_str());
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  sb.setPos(0);
  return true;
}

bool LoginCmd::deserialize(SerialBuffer& sb) {
  sb.setPos(4);
  cmd_id = sb.ReadInt8();
  rp_id = sb.ReadInt32();
  sb.ReadString(rp_psw);
  return true;
}


}//pers
}//scag
