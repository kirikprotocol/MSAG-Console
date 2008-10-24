#include "PersCommand.h"

namespace scag { namespace mtpers {

const int32_t INC_PROPERTY_ERROR = -1;

using namespace scag::pers;

void PersPacket::deserialize(SerialBuffer& sb) {
  profileType = (ProfileType)sb.ReadInt8();
  if (profileType == PT_ABONENT) {
    sb.ReadString(strKey);
    address.setAddress(strKey.c_str());
  } else {
    intKey = sb.ReadInt32();
  }
}

void CommandPacket::deserialize(SerialBuffer& sb) {
  PersPacket::deserialize(sb);
  createProfile = command.deserialize(sb);
}

void BatchPacket::deserialize(SerialBuffer& sb) {
  PersPacket::deserialize(sb);
  count = sb.ReadInt16();
  transact = (bool)sb.ReadInt8();
  createProfile = false;
  for (int i = 0; i < count; ++i) {
    PersCmd cmdId = (PersCmd)sb.ReadInt8();
    PersCommand command(cmdId, dblogs);
    createProfile = command.deserialize(sb) || createProfile;
    batch.push_back(command);
  }
}

bool PersCommand::deserialize(SerialBuffer& sb) {
  switch(cmdId) {
  case PC_DEL:
    sb.ReadString(propertyName);
    return false;
  case PC_SET:
    property.Deserialize(sb);
    return true;
  case PC_GET:
    sb.ReadString(propertyName);
    return false;
  case PC_INC:
    property.Deserialize(sb);
    return true;
  case PC_INC_RESULT:
    property.Deserialize(sb);
    return true;
  case PC_INC_MOD:
    mod = sb.ReadInt32();
    property.Deserialize(sb);
    return true;
  default:
    throw PersCommandNotSupport();
  }
}

Response PersCommand::execute(Profile *pf, SerialBuffer& sb) {
  try {
    switch (cmdId) {
      case scag::pers::util::PC_SET        : return set(pf, sb);
      case scag::pers::util::PC_GET        : return get(pf, sb);
      case scag::pers::util::PC_DEL        : return del(pf, sb);
      case scag::pers::util::PC_INC        : return inc(pf, sb);
      case scag::pers::util::PC_INC_MOD    : return incMod(pf, sb);
      case scag::pers::util::PC_INC_RESULT : return incResult(pf, sb);
      default                              : return RESPONSE_NOTSUPPORT;
    }
  } catch (const SerialBufferOutOfBounds &e) {
    smsc_log_warn(logger, "SerialBufferOutOfBounds key=%s bad data in buffer len=%d, data=%s",
                   pf->getKey().c_str(), sb.length(), sb.toString().c_str());
    return scag::pers::util::RESPONSE_ERROR;
  }
}

Response PersCommand::set(Profile *pf, SerialBuffer& sb) {
  smsc_log_debug(logger, "execute SET command");
  if (property.isExpired()) {
    createExpireLogMsg(pf->getKey(), property.toString());
    sb.WriteInt8(scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND);
    return scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND;
  }
  Property* p = pf->GetProperty(property.getName());
  if (p != NULL) {
    p->setValue(property);
    p->WriteAccess();
    createUpdateLogMsg(pf->getKey(), property.toString());
  } else {
    if (!pf->AddProperty(property)) {
      sb.WriteInt8(scag::pers::util::RESPONSE_ERROR);
      return scag::pers::util::RESPONSE_ERROR;
    }
    createAddLogMsg(pf->getKey(), property.toString());
  }
  pf->setChanged(true);
  sb.WriteInt8(scag::pers::util::RESPONSE_OK);
  return scag::pers::util::RESPONSE_OK;
}

Response PersCommand::get(Profile *pf, SerialBuffer& sb) {
  smsc_log_debug(logger, "execute GET command");
  Property* p = pf->GetProperty(propertyName.c_str());
  if (!p) {
    smsc_log_debug(logger, "profile %s, property '%s' not found", pf->getKey().c_str(), propertyName.c_str());
    sb.WriteInt8(scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND);
    return scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND;
  }
  if(p->getTimePolicy() == R_ACCESS) {
    p->ReadAccess();
    pf->setChanged(true);
  }
  property = *p;
  smsc_log_debug(logger, "profile %s, getProperty=%s", pf->getKey().c_str(), property.toString().c_str());
  sb.WriteInt8(scag::pers::util::RESPONSE_OK);
  property.Serialize(sb);
  return scag::pers::util::RESPONSE_OK;
}

Response PersCommand::del(Profile *pf, SerialBuffer& sb) {
  smsc_log_debug(logger, "execute DEL command");
  if (!pf->DeleteProperty(propertyName.c_str())) {
    smsc_log_debug(logger, "profile %s, property '%s' not found", pf->getKey().c_str(), propertyName.c_str());
    sb.WriteInt8(scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND);
    return scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND;
  }
  createDelLogMsg(pf->getKey(), propertyName);
  sb.WriteInt8(scag::pers::util::RESPONSE_OK);
  pf->setChanged(true);
  return scag::pers::util::RESPONSE_OK;
}

Response PersCommand::incModProperty(Profile *pf, uint32_t& result) {
  Property* p = pf->GetProperty(property.getName());
  if (!p) {
    result = static_cast<uint32_t>(property.getIntValue());
    result = mod > 0 ? result % mod : result;
    property.setIntValue(result);
    if (!pf->AddProperty(property)) {
      return scag::pers::util::RESPONSE_ERROR;
    }
    createAddLogMsg(pf->getKey(), property.toString());
    pf->setChanged(true);
    return scag::pers::util::RESPONSE_OK;
  }

  if (p->getType() == INT && property.getType() == INT) {
    result = static_cast<uint32_t>(p->getIntValue() + property.getIntValue());
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    createUpdateLogMsg(pf->getKey(), p->toString());
    pf->setChanged(true);
    return scag::pers::util::RESPONSE_OK;
  }

  if (p->convertToInt() && property.convertToInt()) {
    result = static_cast<uint32_t>(p->getIntValue() + property.getIntValue());
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    createDelLogMsg(pf->getKey(), p->getName());
    createAddLogMsg(pf->getKey(), p->toString());
    pf->setChanged(true);
    return scag::pers::util::RESPONSE_OK;
  }
  return scag::pers::util::RESPONSE_TYPE_INCONSISTENCE;
}

Response PersCommand::inc(Profile *pf, SerialBuffer& sb) {
  smsc_log_debug(logger, "execute INC command"); 
  uint32_t result = 0;
  Response resp = incModProperty(pf, result);
  sb.WriteInt8(resp);
  return resp;
}

Response PersCommand::incMod(Profile *pf, SerialBuffer& sb) {
  smsc_log_debug(logger, "execute INC_MOD command");
  uint32_t result = 0;
  Response resp = incModProperty(pf, result);
  sb.WriteInt8(resp);
  if (resp == scag::pers::util::RESPONSE_OK) {
    sb.WriteInt32(result);
  }
  return resp;
}

Response PersCommand::incResult(Profile *pf, SerialBuffer& sb) {
  smsc_log_debug(logger, "execute INC_RESULT command");
  return incMod(pf, sb);
}

void PersCommand::createAddLogMsg(string const& key, string const& msg) {
  dblogs.push_back(string("A key=" + key + " property=" + msg));
}

void PersCommand::createUpdateLogMsg(string const& key, string const& msg) {
  dblogs.push_back(string("U key=" + key + " property=" + msg));
}

void PersCommand::createDelLogMsg(string const& key, string const& msg) {
  dblogs.push_back(string("D key=" + key + " name=" + msg));
}

void PersCommand::createExpireLogMsg(string const& key, string const& msg) {
  dblogs.push_back(string("E key=" + key + " property=" + msg));
}


void PersPacket::flushLogs(Logger* log) const {
  smsc_log_debug(log, "flush logs");
  vector<string>::const_iterator i = dblogs.begin();
  for (i; i != dblogs.end(); ++i) {
    smsc_log_info(log, "%s", (*i).c_str());
  }
}

void PersPacket::setPacketSize(SerialBuffer& sb) const {
  sb.SetPos(0);
  sb.WriteInt32(sb.GetSize());
  sb.SetPos(0);
}

void CommandPacket::execCommand(Profile *pf, SerialBuffer& sb) {
  dblogs.clear();
  sb.SetPos(PACKET_LENGTH_SIZE);
  pf->setChanged(false);
  command.execute(pf, sb);
  setPacketSize(sb);
}

void BatchPacket::execCommand(Profile *pf, SerialBuffer& sb) {
  dblogs.clear();
  pf->setChanged(false);
  if (!transact) {
    for (int i = 0; i < count; ++i) {
      batch[i].execute(pf, sb);
    }
    setPacketSize(sb);
    return;
  }
  for (int i = 0; i < count; ++i) {
    Response resp = batch[i].execute(pf, sb);
    if (resp != scag::pers::util::RESPONSE_OK) {
      pf->setChanged(false);
      rollback = true;
      break;
    }
  }
  setPacketSize(sb);
  return;
}

}//mtpers
}//scag

