#include "CPersCmd.h"

namespace scag { namespace cpers {

bool CPersCmd::serialize(SerialBuffer& sb) const {
  sb.setPos(4);
  sb.WriteInt8(cmd_id);
  sb.WriteString(key.c_str());
  writeData(sb);
  sb.setPos(0);
  sb.WriteInt32(sb.length());
  return true;
}

bool CPersCmd::deserialize(SerialBuffer& sb) {
  sb.setPos(4);
  cmd_id = sb.ReadInt8();
  sb.ReadString(key);
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
  if (profile) {
    profile->Serialize(sb);
  }
}

bool ProfileRespCmd::deserialize(SerialBuffer &sb) {
  CPersCmd::deserialize(sb);
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
