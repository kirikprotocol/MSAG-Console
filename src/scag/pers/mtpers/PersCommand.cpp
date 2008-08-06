#include "PersCommand.h"

namespace scag { namespace mtpers {

const int32_t INC_PROPERTY_ERROR = -1;

using namespace scag::pers;

void PersPacket::deserialize(SerialBuffer& sb) {
  profileType = (ProfileType)sb.ReadInt8();
  if (profileType == PT_ABONENT) {
    sb.ReadString(strKey);
    //__trace2__("Abonent profile key='%s'", strKey.c_str());
    address.setAddress(strKey.c_str());
  } else {
    intKey = sb.ReadInt32();
    //__trace2__("NotAbonent profile key=%d", intKey);
  }
}

void CommandPacket::deserialize(SerialBuffer& sb) {
  PersPacket::deserialize(sb);
  createProfile = command.deserialize(sb);
  //__trace2__("create profile=%d", createProfile);
}

void BatchPacket::deserialize(SerialBuffer& sb) {
  PersPacket::deserialize(sb);
  count = sb.ReadInt16();
  transact = (bool)sb.ReadInt8();
  //__trace2__("transact batch=%d batch cnt=%d", transact, count);
  createProfile = false;
  for (int i = 0; i < count; ++i) {
    PersCmd cmdId = (PersCmd)sb.ReadInt8();
    PersCommand command(cmdId);
    createProfile = command.deserialize(sb) || createProfile;
    batch.push_back(command);
  }
  //__trace2__("create profile=%d", createProfile);
}

bool PersCommand::deserialize(SerialBuffer& sb) {
  switch(cmdId) {
  case PC_DEL:
    sb.ReadString(propertyName);
    //__trace2__("DEL property name='%s'", propertyName.c_str());
    return false;
  case PC_SET:
    property.Deserialize(sb);
    //__trace2__("SET property='%s'", property.toString().c_str());
    return true;
  case PC_GET:
    sb.ReadString(propertyName);
    //__trace2__("GET property name='%s'", propertyName.c_str());
    return false;
  case PC_INC:
    property.Deserialize(sb);
    //__trace2__("INC property='%s'", property.toString().c_str());
    return true;
  case PC_INC_RESULT:
    property.Deserialize(sb);
    //__trace2__("INC_RESULT property='%s'", property.toString().c_str());
    return true;
  case PC_INC_MOD:
    mod = sb.ReadInt32();
    property.Deserialize(sb);
    //__trace2__("INC_MOD mod=%d property='%s'", mod, property.toString().c_str());
    return true;
  default:
    throw PersCommandNotSupport();
  }
}

Response PersCommand::execute(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  try {
    switch (cmdId) {
      case scag::pers::PC_SET        : return set(pf, sb, dblog, key);
      case scag::pers::PC_GET        : return get(pf, sb, dblog, key);
      case scag::pers::PC_DEL        : return del(pf, sb, dblog, key);
      case scag::pers::PC_INC        : return inc(pf, sb, dblog, key);
      case scag::pers::PC_INC_MOD    : return incMod(pf, sb, dblog, key);
      case scag::pers::PC_INC_RESULT : return incResult(pf, sb, dblog, key);
      default                        : return RESPONSE_NOTSUPPORT;
    }
  } catch (const SerialBufferOutOfBounds &e) {
    smsc_log_warn(dblog, "SerialBufferOutOfBounds key=%s bad data in buffer len=%d, data=%s",
                   key.c_str(), sb.length(), sb.toString().c_str());
    return scag::pers::RESPONSE_ERROR;
  }
}

Response PersCommand::set(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  smsc_log_debug(dblog, "execute SET command");
  if (property.isExpired()) {
    smsc_log_info(dblog, "E key=\"%s\" property=%s", key.c_str(), property.toString().c_str());
    sb.WriteInt8(scag::pers::RESPONSE_PROPERTY_NOT_FOUND);
    return scag::pers::RESPONSE_PROPERTY_NOT_FOUND;
  }
  Property* p = pf->GetProperty(property.getName());
  if (p != NULL) {
    p->setValue(property);
    p->WriteAccess();
    smsc_log_info(dblog, "U key=\"%s\" property=%s", key.c_str(), property.toString().c_str());
  } else {
    pf->AddProperty(property);
    smsc_log_info(dblog, "A key=\"%s\" property=%s", key.c_str(), property.toString().c_str());
  }
  pf->setChanged(true);
  sb.WriteInt8(scag::pers::RESPONSE_OK);
  return scag::pers::RESPONSE_OK;
}

Response PersCommand::get(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  smsc_log_debug(dblog, "execute GET command");
  Property* p = pf->GetProperty(propertyName.c_str());
  if (!p) {
    smsc_log_debug(dblog, "profile %s, property '%s' not found", key.c_str(), propertyName.c_str());
    sb.WriteInt8(scag::pers::RESPONSE_PROPERTY_NOT_FOUND);
    return scag::pers::RESPONSE_PROPERTY_NOT_FOUND;
  }
  if(p->getTimePolicy() == R_ACCESS) {
    p->ReadAccess();
    pf->setChanged(true);
  }
  property = *p;
  smsc_log_debug(dblog, "profile %s, getProperty=%s", key.c_str(), property.toString().c_str());
  sb.WriteInt8(scag::pers::RESPONSE_OK);
  property.Serialize(sb);
  return scag::pers::RESPONSE_OK;
}

Response PersCommand::del(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  smsc_log_debug(dblog, "execute DEL command");
  if (!pf->DeleteProperty(propertyName.c_str())) {
    smsc_log_debug(dblog, "profile %s, property '%s' not found", key.c_str(), propertyName.c_str());
    sb.WriteInt8(scag::pers::RESPONSE_PROPERTY_NOT_FOUND);
    return scag::pers::RESPONSE_PROPERTY_NOT_FOUND;
  }
  smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.c_str(), propertyName.c_str());
  sb.WriteInt8(scag::pers::RESPONSE_OK);
  pf->setChanged(true);
  return scag::pers::RESPONSE_OK;
}

int PersCommand::incModProperty(Profile *pf, Logger* dblog, const string& key) {
  int result = INC_PROPERTY_ERROR;
  Property* p = pf->GetProperty(property.getName());
  if (!p) {
    result = property.getIntValue();
    result = mod > 0 ? result % mod : result;
    property.setIntValue(result);
    pf->AddProperty(property);
    smsc_log_info(dblog, "A key=\"%s\" property=%s", key.c_str(), property.toString().c_str());            
    pf->setChanged(true);
    return result;
  }

  if (p->getType() == INT && property.getType() == INT) {
    result = p->getIntValue() + property.getIntValue();
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    smsc_log_info(dblog, "U key=\"%s\" property=%s", key.c_str(), p->toString().c_str());                
    pf->setChanged(true);
    return result;
  }

  if (p->convertToInt() && property.convertToInt()) {
    result = p->getIntValue() + property.getIntValue();
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.c_str(), p->getName());                
    smsc_log_info(dblog, "A key=\"%s\" property=%s", key.c_str(), p->toString().c_str());                
    pf->setChanged(true);
    return result;
  }
  return result;
}

Response PersCommand::inc(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  smsc_log_debug(dblog, "execute INC command"); 
  if (incModProperty(pf, dblog, key) == INC_PROPERTY_ERROR) {
    sb.WriteInt8(scag::pers::RESPONSE_TYPE_INCONSISTENCE);
    return scag::pers::RESPONSE_TYPE_INCONSISTENCE;
  }
  sb.WriteInt8(scag::pers::RESPONSE_OK);
  return scag::pers::RESPONSE_OK;
}

Response PersCommand::incMod(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  smsc_log_debug(dblog, "execute INC_MOD command");
  int result = incModProperty(pf, dblog, key);
  if (result == INC_PROPERTY_ERROR) {
    sb.WriteInt8(scag::pers::RESPONSE_TYPE_INCONSISTENCE);
    return scag::pers::RESPONSE_TYPE_INCONSISTENCE;
  }
  sb.WriteInt8(scag::pers::RESPONSE_OK);
  sb.WriteInt32(result);
  return scag::pers::RESPONSE_OK;
}

Response PersCommand::incResult(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  smsc_log_debug(dblog, "execute INC_RESULT command");
  return incMod(pf, sb, dblog, key);
}

void PersPacket::setPacketSize(SerialBuffer& sb) const {
  sb.SetPos(0);
  sb.WriteInt32(sb.GetSize());
  sb.SetPos(0);
}

void CommandPacket::execCommand(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  sb.SetPos(PACKET_LENGTH_SIZE);
  pf->setChanged(false);
  command.execute(pf, sb, dblog, key);
  setPacketSize(sb);
}

void BatchPacket::execCommand(Profile *pf, SerialBuffer& sb, Logger* dblog, const string& key) {
  pf->setChanged(false);
  if (transact) {
    for (int i = 0; i < count; ++i) {
      Response resp = batch[i].execute(pf, sb, dblog, key);
      if (resp != scag::pers::RESPONSE_OK) {
        smsc_log_warn(dblog, "break transact batch execution response = %d", resp);
        pf->setChanged(false);
        sb.Empty();
        sb.SetPos(4);
        sb.WriteInt8(resp);
        break;
      }
    }
  } else {
    for (int i = 0; i < count; ++i) {
      batch[i].execute(pf, sb, dblog, key);
    }
  }
  setPacketSize(sb);
}

}//mtpers
}//scag

