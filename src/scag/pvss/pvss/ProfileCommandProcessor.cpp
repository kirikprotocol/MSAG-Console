#include <memory>

#include "ProfileCommandProcessor.h"

#include "scag/pvss/api/packets/ResponseVisitor.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/packets/BatchResponse.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/DelResponse.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/GetResponse.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncResponse.h"
#include "scag/pvss/api/packets/IncModCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/SetResponse.h"

namespace scag2 {
namespace pvss  {

bool ProfileCommandProcessor::visitBatchCommand(BatchCommand &cmd) throw(PvapException) {
  ProfileCommandProcessor proc;
  proc.setProfile(profile_);
  response_.reset( new BatchResponse(cmd.getSeqNum()) );
  std::vector<BatchRequestComponent*> content = cmd.getBatchContent();
  batchLogs_.reserve(content.size());
  typedef std::vector<BatchRequestComponent*>::iterator BatchIterator;
  BatchResponse* resp = static_cast<BatchResponse*>(response_.get());
  if (cmd.isTransactional()) {
    for (BatchIterator i = content.begin(); i != content.end(); ++i) {
      bool result = (*i)->visit(proc);
      resp->addComponent(proc.getBatchResponseComponent());
      addBatchComponentDBLog(proc.getDBLog());
      if (!result) {
        rollback_ = true;
        batchLogs_.clear();
        return false;
      }
    }
  } else {
    for (BatchIterator i = content.begin(); i != content.end(); ++i) {
      (*i)->visit(proc);
      resp->addComponent(proc.getBatchResponseComponent());
      addBatchComponentDBLog(proc.getDBLog());
    }
  }
  return true;
}

bool ProfileCommandProcessor::visitDelCommand(DelCommand &cmd) throw(PvapException) {
  response_.reset( new DelResponse(cmd.getSeqNum()) ); 
  if (!profile_->DeleteProperty(cmd.getVarName().c_str())) {
    response_->setStatus(Response::PROPERTY_NOT_FOUND);
    return false;
  }
  dblog_.createDelLogMsg(profile_->getKey(), cmd.getVarName());
  response_->setStatus(Response::OK);
  profile_->setChanged(true);
  return true;
}

bool ProfileCommandProcessor::visitGetCommand(GetCommand &cmd) throw(PvapException) {
  response_.reset( new GetResponse(cmd.getSeqNum()) ); 
  Property* p = profile_->GetProperty(cmd.getVarName().c_str());
  if (!p) {
    response_->setStatus(Response::PROPERTY_NOT_FOUND);
    return false;
  }
  if(p->getTimePolicy() == R_ACCESS) {
    p->ReadAccess();
    profile_->setChanged(true);
  }
  response_->setStatus(Response::OK);
  static_cast<GetResponse*>(response_.get())->setProperty(new Property(*p));
  return true;
}

bool ProfileCommandProcessor::visitIncCommand(IncCommand &cmd) throw(PvapException) {
  uint32_t result = 0;
  Response::StatusType status = incModProperty(cmd.getProperty(), 0, result);
  response_.reset( new IncResponse(cmd.getSeqNum()) );
  response_->setStatus(status);
  if (status != Response::OK) {
    return false;
  }
  static_cast<IncResponse*>(response_.get())->setResult(result);
  return true;
}

bool ProfileCommandProcessor::visitIncModCommand(IncModCommand &cmd) throw(PvapException) {
  uint32_t result = 0;
  Response::StatusType status = incModProperty(cmd.getProperty(), cmd.getModulus(), result);
  response_.reset( new IncResponse(cmd.getSeqNum()) );
  response_->setStatus(status);
  if (status != Response::OK) {
    return false;
  }
  static_cast<IncResponse*>(response_.get())->setResult(result);
  return true;
}

bool ProfileCommandProcessor::visitSetCommand(SetCommand &cmd) throw(PvapException) {
  Property *prop = cmd.getProperty();
  response_.reset( new SetResponse() );
  if (prop->isExpired()) {
    dblog_.createExpireLogMsg(profile_->getKey(), prop->toString());
    response_->setStatus(Response::PROPERTY_NOT_FOUND);
    return false;
  }
  Property* p = profile_->GetProperty(prop->getName());
  if (p != NULL) {
    p->setValue(*prop);
    p->WriteAccess();
    dblog_.createUpdateLogMsg(profile_->getKey(), prop->toString());
  } else {
    if (!profile_->AddProperty(*prop)) {
      response_->setStatus(Response::ERROR);
      return false;
    }
    dblog_.createAddLogMsg(profile_->getKey(), prop->toString());
  }
  profile_->setChanged(true);
  response_->setStatus(Response::OK);
  return true;
}

Response::StatusType ProfileCommandProcessor::incModProperty(Property* property, uint32_t mod, uint32_t& result) {
  Property* p = profile_->GetProperty(property->getName());
  if (!p) {
    result = static_cast<uint32_t>(property->getIntValue());
    result = mod > 0 ? result % mod : result;
    property->setIntValue(result);
    if (!profile_->AddProperty(*property)) {
      return Response::ERROR;
    }
    dblog_.createAddLogMsg(profile_->getKey(), property->toString());
    profile_->setChanged(true);
    return Response::OK;
  }

  if (p->getType() == INT && property->getType() == INT) {
    result = static_cast<uint32_t>(p->getIntValue() + property->getIntValue());
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    dblog_.createUpdateLogMsg(profile_->getKey(), p->toString());
    profile_->setChanged(true);
    return Response::OK;
  }

  if (p->convertToInt() && property->convertToInt()) {
    result = static_cast<uint32_t>(p->getIntValue() + property->getIntValue());
    result = mod > 0 ? result % mod : result;
    p->setIntValue(result);
    p->WriteAccess();
    dblog_.createDelLogMsg(profile_->getKey(), p->getName());
    dblog_.createAddLogMsg(profile_->getKey(), p->toString());
    profile_->setChanged(true);
    return Response::OK;
  }
  return Response::TYPE_INCONSISTENCE;
}

void ProfileCommandProcessor::setProfile(Profile *pf) {
  profile_ = pf;
}

BatchResponseComponent* ProfileCommandProcessor::getBatchResponseComponent() {
  return static_cast<BatchResponseComponent*>(response_.release());
}

Response* ProfileCommandProcessor::getResponse() {
  return response_.release();
}

const string& ProfileCommandProcessor::getDBLog() const {
  return dblog_.getLogMsg();
}

void ProfileCommandProcessor::addBatchComponentDBLog(const string& logmsg) {
  if (!logmsg.empty()) {
    batchLogs_.push_back(logmsg);
  }
}


void ProfileCommandProcessor::flushLogs(Logger* logger) {
  if (!batchLogs_.empty()) {
    for (std::vector<string>::iterator i = batchLogs_.begin(); i != batchLogs_.end(); ++i) {
      smsc_log_info(logger, (*i).c_str());
    }
    batchLogs_.clear();
    return;
  }
  if (!dblog_.getLogMsg().empty()) {
    smsc_log_info(logger, dblog_.getLogMsg().c_str());
    dblog_.clear();
  }
}

}
}
