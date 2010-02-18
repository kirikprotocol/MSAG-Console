#include <memory>

#include "ProfileCommandProcessor.h"

#include "scag/pvss/api/packets/ProfileRequest.h"
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
#include "scag/pvss/profile/ProfileBackup.h"

namespace scag2 {
namespace pvss  {


bool ProfileCommandProcessor::applyCommonLogic( const std::string& profkey,
                                                ProfileRequest&    profileRequest,
                                                Profile*           pf,
                                                bool               createProfile )
{
    if (pf) {
        smsc_log_debug(log_,"FIXME(pre): prof=%s",pf->toString().c_str());
        if ( pf->getKey() != profkey ) {
            smsc_log_warn(log_,"key mismatch: pf=%p pf.key=%s req.key=%s",
                          pf,pf->getKey().c_str(),profkey.c_str());
            pf->setKey(profkey);
        }
    }

    // full cleanup
    resetProfile(pf);

    profileRequest.getCommand()->visit(*this);
    if (!pf) {
        if (createProfile) {
            smsc_log_warn(log_, "%p: req=%p can't create profile %s", this, &profileRequest, profkey.c_str());
        }
        return false;
    }

    if (rollback_) {
        smsc_log_debug(log_, "%p: %p rollback profile %s changes", this, &profileRequest, profkey.c_str());
        backup_->rollback(*profile_);
        return false;
    }

    backup_->flushLogs(*profile_);
    smsc_log_debug(log_,"FIXME(post): prof=%s",pf->toString().c_str());
    if (pf->isChanged()) {
        smsc_log_debug(log_,"profile %s needs flush",profkey.c_str());
    }
    return true;
    
}



void ProfileCommandProcessor::resetProfile(Profile *pf) 
{
    /*
    if (!dblog_.getLogMsg().empty()) {
        smsc_log_warn(log_, "db log not empty: %s", dblog_.getLogMsg().c_str());
        dblog_.clear();
    }
    if (!batchLogs_.empty()) {
        for (std::vector<string>::iterator i = batchLogs_.begin(); i != batchLogs_.end(); ++i) {
            smsc_log_warn(log_, "batch db log not empty: %s", i->c_str());
        }
        std::vector<string>().swap(batchLogs_);
    }
     */
    backup_->cleanup();
    rollback_ = false;
    profile_ = pf;
    if (pf) { pf->setChanged(false); }
    response_.reset(0);
}



bool ProfileCommandProcessor::visitBatchCommand(BatchCommand &cmd) /* throw(PvapException) */  {
    ProfileCommandProcessor proc(*backup_,profile_);
    BatchResponse* resp = new BatchResponse(StatusType::OK);
    response_.reset(resp);
    std::vector<BatchRequestComponent*> content = cmd.getBatchContent();
    // batchLogs_.reserve(content.size());
    typedef std::vector<BatchRequestComponent*>::iterator BatchIterator;
    if (cmd.isTransactional()) {
        for (BatchIterator i = content.begin(); i != content.end(); ++i) {
            bool result = (*i)->visit(proc);
            resp->addComponent(static_cast<BatchResponseComponent*>(proc.getResponse()));
            // addBatchComponentDBLog(proc.getDBLog());
            // proc.clearDBLog();
            if (!result) {
                rollback_ = true;
                // batchLogs_.clear();
                return false;
            }
        }
    } else {
        for (BatchIterator i = content.begin(); i != content.end(); ++i) {
            (*i)->visit(proc);
            resp->addComponent(static_cast<BatchResponseComponent*>(proc.getResponse()));
            // addBatchComponentDBLog(proc.getDBLog());
            // proc.clearDBLog();
        }
    }
    return true;
}


bool ProfileCommandProcessor::visitDelCommand(DelCommand &cmd) /* throw(PvapException) */  {
    response_.reset(new DelResponse());    
    std::auto_ptr<Property> holder;
    if (!profile_ || !profile_->DeleteProperty(cmd.getVarName().c_str(),&holder)) {
        response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    backup_->delProperty(holder.release());
    // dblog_.createDelLogMsg(profile_->getKey(), cmd.getVarName());
    response_->setStatus(StatusType::OK);
    profile_->setChanged(true);
    return true;
}


bool ProfileCommandProcessor::visitGetCommand(GetCommand &cmd) /* throw(PvapException) */  {
    response_.reset(new GetResponse());
    if (!profile_) {
        response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    Property* p = profile_->GetProperty(cmd.getVarName().c_str());
    if (!p) {
        response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    // temporary
    // Logger* theLog = profile_->getLog();
    // if (theLog) {
    // smsc_log_debug(theLog,"G key=%s name=%s", profile_->getKey().c_str(), p->toString().c_str());
    // }
    backup_->getProperty(*p);
    if(p->getTimePolicy() == R_ACCESS) {
        backup_->fixTimePolicy(*p);
        p->ReadAccess();
        profile_->setChanged(true);
    }

    response_->setStatus(StatusType::OK);
    static_cast<GetResponse*>(response_.get())->setProperty(*p);
    return true;
}

bool ProfileCommandProcessor::visitIncCommand(IncCommand &cmd) /* throw(PvapException) */  {
    uint32_t result = 0;
    uint8_t status = incModProperty(cmd.getProperty(), 0, result);
    IncResponse *incResp = new IncResponse(status);
    incResp->setIncResult(cmd.isIncResult());
    response_.reset( incResp );
    if (status != StatusType::OK) {
        return false;
    }
    static_cast<IncResponse*>(response_.get())->setResult(result);
    return true;
}

bool ProfileCommandProcessor::visitIncModCommand(IncModCommand &cmd) /* throw(PvapException) */  {
    uint32_t result = 0;
    response_.reset( new IncResponse());
    if (!profile_) {
        response_->setStatus(StatusType::IO_ERROR);
        return false;
    }
    uint8_t status = incModProperty(cmd.getProperty(), cmd.getModulus(), result);
    response_->setStatus(status);
    if (status != StatusType::OK) {
        return false;
    }
    static_cast<IncResponse*>(response_.get())->setResult(result);
    return true;
}

bool ProfileCommandProcessor::visitSetCommand(SetCommand &cmd) /* throw(PvapException) */  {
    const Property& prop = cmd.getProperty();
    response_.reset( new SetResponse() );
    if (!profile_) {
        response_->setStatus(StatusType::IO_ERROR);
        return false;
    }
    if (prop.isExpired()) {
        // FIXME: do we need this message? it is not related to real profile...
        // dblog_.createExpireLogMsg(profile_->getKey(), prop.toString());
        response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    Property* p = profile_->GetProperty(prop.getName());
    if (p != NULL) {
        backup_->fixProperty(*p);
        p->setValue(prop);
        // FIXED by bukind on 20100209, now timepolicy is also updated!
        p->setTimePolicy(prop.getTimePolicy(),prop.getFinalDate(),prop.getLifeTime());
        p->WriteAccess();
        backup_->propertyUpdated(*p);
        // dblog_.createUpdateLogMsg(profile_->getKey(), p->toString());
    } else {
        if (!profile_->AddProperty(prop)) {
            response_->setStatus(StatusType::BAD_REQUEST);
            return false;
        }
        backup_->addProperty(prop);
        // dblog_.createAddLogMsg(profile_->getKey(), prop.toString());
    }
    profile_->setChanged(true);
    response_->setStatus(StatusType::OK);
    return true;
}

uint8_t ProfileCommandProcessor::incModProperty(Property& property, uint32_t mod, uint32_t& result)
{
    Property* p = profile_->GetProperty(property.getName());
    if (!p) {
        result = static_cast<uint32_t>(property.getIntValue());
        result = mod > 0 ? result % mod : result;
        property.setIntValue(result);
        if (!profile_->AddProperty(property)) {
            return StatusType::IO_ERROR;
        }
        backup_->addProperty(property);
        // dblog_.createAddLogMsg(profile_->getKey(), property.toString());
        profile_->setChanged(true);
        return StatusType::OK;
    }

    if (p->getType() == INT && property.getType() == INT) {
        backup_->fixProperty(*p);
        result = static_cast<uint32_t>(p->getIntValue() + property.getIntValue());
        result = mod > 0 ? result % mod : result;
        p->setIntValue(result);
        p->WriteAccess();
        // dblog_.createUpdateLogMsg(profile_->getKey(), p->toString());
        backup_->propertyUpdated(*p);
        profile_->setChanged(true);
        return StatusType::OK;
    }

    if (p->convertToInt() && property.convertToInt()) {
        result = static_cast<uint32_t>(p->getIntValue() + property.getIntValue());
        result = mod > 0 ? result % mod : result;
        backup_->delProperty(p,true);
        p->setIntValue(result);
        p->WriteAccess();
        // dblog_.createDelLogMsg(profile_->getKey(), p->getName());
        // dblog_.createAddLogMsg(profile_->getKey(), p->toString());
        backup_->addProperty(*p,true);
        profile_->setChanged(true);
        return StatusType::OK;
    }
    return StatusType::TYPE_INCONSISTENCE;
}

/*
BatchResponseComponent* ProfileCommandProcessor::getBatchResponseComponent() {
    return static_cast<BatchResponseComponent*>(response_.release());
}
CommandResponse* ProfileCommandProcessor::getResponse() {
  return response_.release();
}
const string& ProfileCommandProcessor::getDBLog() const {
  return dblog_.getLogMsg();
}
void ProfileCommandProcessor::clearDBLog() {
  return dblog_.clear();
}
void ProfileCommandProcessor::addBatchComponentDBLog(const string& logmsg) {
  if (!logmsg.empty()) {
    batchLogs_.push_back(logmsg);
  }
}
void ProfileCommandProcessor::flushLogs(Logger* logger) {
  if (!batchLogs_.empty()) {
    for (std::vector<string>::iterator i = batchLogs_.begin(); i != batchLogs_.end(); ++i) {
      smsc_log_info(logger, "%s", (*i).c_str());
    }
    std::vector<string>().swap(batchLogs_);
    return;
  }
  if (!dblog_.getLogMsg().empty()) {
    smsc_log_info(logger, "%s", dblog_.getLogMsg().c_str());
    dblog_.clear();
  }
}
 */

}
}
