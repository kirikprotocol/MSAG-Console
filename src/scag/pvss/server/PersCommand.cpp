#include "PersCommand.h"
#include "scag/util/io/Print.h"

namespace scag2 {

using namespace util;

namespace pvss  {

const int32_t INC_PROPERTY_ERROR = -1;
static const string PROPERTY_LOG = " property=";
static const string NAME_LOG     = " name=";
static const string UPDATE_LOG   = "U key=";
static const string ADD_LOG      = "A key=";
static const string DEL_LOG      = "D key=";
static const string EXPIRE_LOG   = "E key=";

void PersPacket::deserialize(SerialBuffer& sb) {
  profileType = (ProfileType)sb.ReadInt8();
  if (profileType == PT_ABONENT) {
    sb.ReadString(strKey);
    address.fromString(strKey.c_str());
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
  batch_.reserve(count_);
  PvssCommand command;
  for (int i = 0; i < count_; ++i) {
    command.setCmdId((PersCmd)sb.ReadInt8());
    createProfile = command.deserialize(sb) || createProfile;
    batch_.push_back(command);
  }
}

void PvssCommand::setCmdId(PersCmd cmd) {
  cmdId = cmd;
}

bool PvssCommand::deserialize(SerialBuffer& sb) {
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

Response PvssCommand::execute(Profile *pf, SerialBuffer& sb) {
  switch (cmdId) {
    case PC_SET        : return set(pf, sb);
    case PC_GET        : return get(pf, sb);
    case PC_DEL        : return del(pf, sb);
    case PC_INC        : return inc(pf, sb);
    case PC_INC_MOD    : return incMod(pf, sb);
    case PC_INC_RESULT : return incResult(pf, sb);
    default            : return RESPONSE_NOTSUPPORT;
  }
}

Response PvssCommand::set(Profile *pf, SerialBuffer& sb) {
  if (property.isExpired()) {
    createExpireLogMsg(pf->getKey(), property.toString());
    sb.WriteInt8(RESPONSE_PROPERTY_NOT_FOUND);
    return RESPONSE_PROPERTY_NOT_FOUND;
  }
  Property* p = pf->GetProperty(property.getName());
  if (p != NULL) {
    p->setValue(property);
    p->WriteAccess();
    createUpdateLogMsg(pf->getKey(), property.toString());
  } else {
    if (!pf->AddProperty(property)) {
      sb.WriteInt8(RESPONSE_ERROR);
      return RESPONSE_ERROR;
    }
    createAddLogMsg(pf->getKey(), property.toString());
  }
  pf->setChanged(true);
  sb.WriteInt8(RESPONSE_OK);
  return RESPONSE_OK;
}

Response PvssCommand::get(Profile *pf, SerialBuffer& sb) {
  Property* p = pf->GetProperty(propertyName.c_str());
  if (!p) {
    sb.WriteInt8(RESPONSE_PROPERTY_NOT_FOUND);
    return RESPONSE_PROPERTY_NOT_FOUND;
  }
  if(p->getTimePolicy() == R_ACCESS) {
    p->ReadAccess();
    pf->setChanged(true);
  }
  property = *p;
  sb.WriteInt8(RESPONSE_OK);
  property.Serialize(sb);
  return RESPONSE_OK;
}

Response PvssCommand::del(Profile *pf, SerialBuffer& sb) {
  if (!pf->DeleteProperty(propertyName.c_str())) {
    sb.WriteInt8(RESPONSE_PROPERTY_NOT_FOUND);
    return RESPONSE_PROPERTY_NOT_FOUND;
  }
  createDelLogMsg(pf->getKey(), propertyName);
  sb.WriteInt8(RESPONSE_OK);
  pf->setChanged(true);
  return RESPONSE_OK;
}

Response PvssCommand::incModProperty(Profile *pf, uint32_t& result) {
  Property* p = pf->GetProperty(property.getName());
  if (!p) {
    result = static_cast<uint32_t>(property.getIntValue());
    result = mod > 0 ? result % mod : result;
    property.setIntValue(result);
    if (!pf->AddProperty(property)) {
      return RESPONSE_ERROR;
    }
    createAddLogMsg(pf->getKey(), property.toString());
    pf->setChanged(true);
    return RESPONSE_OK;
  }

  if (p->getType() == INT && property.getType() == INT) {
    result = static_cast<uint32_t>(p->getIntValue() + property.getIntValue());
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    createUpdateLogMsg(pf->getKey(), p->toString());
    pf->setChanged(true);
    return RESPONSE_OK;
  }

  if (p->convertToInt() && property.convertToInt()) {
    result = static_cast<uint32_t>(p->getIntValue() + property.getIntValue());
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    createDelLogMsg(pf->getKey(), p->getName());
    createAddLogMsg(pf->getKey(), p->toString());
    pf->setChanged(true);
    return RESPONSE_OK;
  }
  return RESPONSE_TYPE_INCONSISTENCE;
}

Response PvssCommand::inc(Profile *pf, SerialBuffer& sb) {
  uint32_t result = 0;
  Response resp = incModProperty(pf, result);
  sb.WriteInt8(resp);
  return resp;
}

Response PvssCommand::incMod(Profile *pf, SerialBuffer& sb) {
  uint32_t result = 0;
  Response resp = incModProperty(pf, result);
  sb.WriteInt8(resp);
  if (resp == RESPONSE_OK) {
    sb.WriteInt32(result);
  }
  return resp;
}

Response PvssCommand::incResult(Profile *pf, SerialBuffer& sb) {
  return incMod(pf, sb);
}

void PvssCommand::createAddLogMsg(string const& key, string const& msg) {
  dblogMsg_ = ADD_LOG;
  dblogMsg_.append(key);
  dblogMsg_.append(PROPERTY_LOG);
  dblogMsg_.append(msg);
}

void PvssCommand::createUpdateLogMsg(string const& key, string const& msg) {
  dblogMsg_ = UPDATE_LOG;
  dblogMsg_.append(key);
  dblogMsg_.append(PROPERTY_LOG);
  dblogMsg_.append(msg);
}

void PvssCommand::createDelLogMsg(string const& key, string const& msg) {
  dblogMsg_ = DEL_LOG;
  dblogMsg_.append(key);
  dblogMsg_.append(NAME_LOG);
  dblogMsg_.append(msg);
}

void PvssCommand::createExpireLogMsg(string const& key, string const& msg) {
  dblogMsg_ = EXPIRE_LOG;
  dblogMsg_.append(key);
  dblogMsg_.append(PROPERTY_LOG);
  dblogMsg_.append(msg);
}

const char* PvssCommand::dbLog() const {
  return dblogMsg_.c_str();
}

/*
//void CommandPacket::flushLogs(Logger* log) const {
void CommandPacket::flushLogs(Logger* log) {
  smsc_log_debug(log, "flush logs");
  //dbLogs_.push_back(command_.dbLog());
  smsc_log_info(log, "%s", command_.dbLog());
}

//void BatchPacket::flushLogs(Logger* log) const {
void BatchPacket::flushLogs(Logger* log) {
  smsc_log_debug(log, "flush logs");
  dbLogs_.reserve(batch_.size());
  for (vector<PvssCommand>::const_iterator i = batch_.begin(); i != batch_.end(); ++i) {
    //dbLogs_.push_back((*i).dbLog());
    smsc_log_info(log, "%s", (*i).dbLog());
  }
}
*/
PersPacket::PersPacket(Connection* connect, bool async, uint32_t sequenseNumber, time_t requestTime):createProfile(false), rollback(false),
            connection_(connect), asynch_(async), sequenseNumber_(sequenseNumber), intKey(0), requestTime_(requestTime)
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
  connection_->sendResponse(response_.c_ptr(), response_.GetSize(), &dbLogs_);
}

void CommandPacket::execCommand(Profile *pf, Logger* dblogger) {
  pf->setChanged(false);
  command_.execute(pf, response_);
  if (pf->isChanged()) {
    dbLogs_.push_back(DbLog(dblogger, command_.dbLog()));
  }
}

void BatchPacket::execCommand(Profile *pf, Logger* dblogger) {
  pf->setChanged(false);
  dbLogs_.reserve(batch_.size());
  DbLog dblog;
  if (!transact_) {
    for (vector<PvssCommand>::iterator i = batch_.begin(); i != batch_.end(); ++i) {
      if ((*i).execute(pf, response_) == RESPONSE_OK) {
        dblog.set(dblogger,(*i).dbLog());
        dbLogs_.push_back(dblog);
      }
    }
    return;
  }
  for (vector<PvssCommand>::iterator i = batch_.begin(); i != batch_.end(); ++i) {
    Response resp = (*i).execute(pf, response_);
    if (resp != RESPONSE_OK) {
      pf->setChanged(false);
      rollback = true;
      dbLogs_.clear();
      break;
    } else {
      dblog.set(dblogger,(*i).dbLog());
      dbLogs_.push_back(dblog);
    }
  }
  return;
}

}//pvss
}//scag2

