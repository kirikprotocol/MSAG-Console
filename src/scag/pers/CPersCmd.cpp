#include "CPersCmd.h"

namespace scag { namespace cpers {

bool CPersCmd::serialize(SerialBuffer& sb) const {
  sb.Empty();
  sb.setPos(4);
  sb.WriteInt8(cmd_id);
  return true;
}

bool CPersCmd::deserialize(SerialBuffer& sb) {
  sb.setPos(4);
  cmd_id = sb.ReadInt8();
  return true;
}

bool LoginCmd::serialize(SerialBuffer& sb) const {
  CPersCmd::serialize(sb);
  sb.WriteInt32(rp_id);
  sb.WriteString(rp_psw.c_str());
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  return true;
}

bool LoginCmd::deserialize(SerialBuffer& sb) {
  CPersCmd::deserialize(sb);
  rp_id = sb.ReadInt32();
  sb.ReadString(rp_psw);
  return true;
}

bool GetProfileCmd::serialize(SerialBuffer &sb) const {
  CPersCmd::serialize(sb);
  sb.WriteString(key.c_str());
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  return true;
}

bool GetProfileCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  sb.ReadString(key);
  return true;
}

bool ProfileRespCmd::serialize(SerialBuffer &sb) const {
  CPersCmd::serialize(sb);
  sb.WriteString(key.c_str());
  sb.WriteInt8(is_ok);
  if (!is_ok) {
    return true;
  }
  if (profile) {
    profile->Serialize(sb);
  }
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  return true;
}

bool ProfileRespCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  sb.ReadString(key);
  if (profile) {
    delete profile;
    profile = 0;
  }
  is_ok = sb.ReadInt8();
  if (!is_ok) {
    return true;
  }
  if (sb.length() > sb.getPos()) {
    profile = new Profile(key);
    profile->Deserialize(sb);
  }
  return true;
}

ProfileRespCmd::~ProfileRespCmd() {
  if (profile) {
    delete profile;
  }
}

bool DoneCmd::serialize(SerialBuffer &sb) const {
  CPersCmd::serialize(sb);
  sb.WriteInt8(is_ok);
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  return true;
}

bool DoneCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  is_ok = sb.ReadInt8();
  return true;
}

bool DoneRespCmd::serialize(SerialBuffer &sb) const {
  CPersCmd::serialize(sb);
  sb.WriteInt8(is_ok);
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  return true;
}

bool DoneRespCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
  is_ok = sb.ReadInt8();
  return true;
}


}//pers
}//scag
