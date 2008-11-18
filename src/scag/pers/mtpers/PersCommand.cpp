#include "PersCommand.h"
#include "scag/util/Print.h"

namespace scag {

using namespace util;

namespace mtpers {

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
  createProfile = command_.deserialize(sb);
}

void BatchPacket::deserialize(SerialBuffer& sb) {
  PersPacket::deserialize(sb);
  count_ = sb.ReadInt16();
  transact_ = (bool)sb.ReadInt8();
  createProfile = false;
  for (int i = 0; i < count_; ++i) {
    PersCmd cmdId = (PersCmd)sb.ReadInt8();
    PersCommand command(cmdId, dblogs_);
    createProfile = command.deserialize(sb) || createProfile;
    batch_.push_back(command);
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
  switch (cmdId) {
    case scag::pers::util::PC_SET        : return set(pf, sb);
    case scag::pers::util::PC_GET        : return get(pf, sb);
    case scag::pers::util::PC_DEL        : return del(pf, sb);
    case scag::pers::util::PC_INC        : return inc(pf, sb);
    case scag::pers::util::PC_INC_MOD    : return incMod(pf, sb);
    case scag::pers::util::PC_INC_RESULT : return incResult(pf, sb);
    default                              : return RESPONSE_NOTSUPPORT;
  }
}

Response PersCommand::set(Profile *pf, SerialBuffer& sb) {
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
  Property* p = pf->GetProperty(propertyName.c_str());
  if (!p) {
    sb.WriteInt8(scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND);
    return scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND;
  }
  if(p->getTimePolicy() == R_ACCESS) {
    p->ReadAccess();
    pf->setChanged(true);
  }
  property = *p;
  sb.WriteInt8(scag::pers::util::RESPONSE_OK);
  property.Serialize(sb);
  return scag::pers::util::RESPONSE_OK;
}

Response PersCommand::del(Profile *pf, SerialBuffer& sb) {
  if (!pf->DeleteProperty(propertyName.c_str())) {
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
  uint32_t result = 0;
  Response resp = incModProperty(pf, result);
  sb.WriteInt8(resp);
  return resp;
}

Response PersCommand::incMod(Profile *pf, SerialBuffer& sb) {
  uint32_t result = 0;
  Response resp = incModProperty(pf, result);
  sb.WriteInt8(resp);
  if (resp == scag::pers::util::RESPONSE_OK) {
    sb.WriteInt32(result);
  }
  return resp;
}

Response PersCommand::incResult(Profile *pf, SerialBuffer& sb) {
  return incMod(pf, sb);
}

void PersCommand::createAddLogMsg(string const& key, string const& msg) {
  dblogs->push_back(string("A key=" + key + " property=" + msg));
}

void PersCommand::createUpdateLogMsg(string const& key, string const& msg) {
  dblogs->push_back(string("U key=" + key + " property=" + msg));
}

void PersCommand::createDelLogMsg(string const& key, string const& msg) {
  dblogs->push_back(string("D key=" + key + " name=" + msg));
}

void PersCommand::createExpireLogMsg(string const& key, string const& msg) {
  dblogs->push_back(string("E key=" + key + " property=" + msg));
}


void PersPacket::flushLogs(Logger* log) const {
  smsc_log_debug(log, "flush logs");
  for (vector<string>::const_iterator i = dblogs_.begin(); i != dblogs_.end(); ++i) {
    smsc_log_info(log, "%s", (*i).c_str());
  }
}

PersPacket::PersPacket(Connection* connect, bool async, uint32_t sequenseNumber):createProfile(false), rollback(false), connection_(connect),
                                                                                 asynch_(async), sequenseNumber_(sequenseNumber)
{
  if (asynch_) {
    response_.WriteInt32(sequenseNumber_);
  }
}


void PersPacket::createResponse(PersServerResponseType resp) {
  response_.Empty();
  if (asynch_) {
    response_.WriteInt32(sequenseNumber_);
  }
  response_.WriteInt8(resp);
}

void PersPacket::sendResponse() {
  connection_->sendResponse(response_.c_ptr(), response_.GetSize());
}

void CommandPacket::execCommand(Profile *pf) {
  dblogs_.clear();
  pf->setChanged(false);
  command_.execute(pf, response_);
}

void BatchPacket::execCommand(Profile *pf) {
  dblogs_.clear();
  pf->setChanged(false);
  if (!transact_) {
    for (int i = 0; i < count_; ++i) {
      batch_[i].execute(pf, response_);
    }
    return;
  }
  for (int i = 0; i < count_; ++i) {
    Response resp = batch_[i].execute(pf, response_);
    if (resp != scag::pers::util::RESPONSE_OK) {
      pf->setChanged(false);
      rollback = true;
      break;
    }
  }
  return;
}

}//mtpers
}//scag

