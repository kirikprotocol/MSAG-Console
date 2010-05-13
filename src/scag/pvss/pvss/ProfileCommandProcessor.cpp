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
#include "scag/pvss/api/packets/GetProfileCommand.h"
#include "scag/pvss/api/packets/GetProfileResponse.h"
#include "scag/pvss/profile/ProfileBackup.h"

namespace scag2 {
namespace pvss  {

bool ProfileCommandProcessor::applyCommonLogic( const std::string& profkey,
                                                ProfileRequest&    profileRequest,
                                                Profile*           pf,
                                                bool               createProfile )
{
    if (pf) {
        if (profileRequest.hasTiming()) { profileRequest.timingMark("pfgot"); }
        // smsc_log_debug(log_,"FIXME(pre): prof=%s",pf->toString().c_str());
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

    bool ret;
    if (rollback_) {
        smsc_log_debug(log_, "%p: %p rollback profile %s changes", this, &profileRequest, profkey.c_str());
        backup_->rollback(*profile_);
        profile_->setChanged(false);
        ret = false;
        if (profileRequest.hasTiming()) { profileRequest.timingMark("rollback"); }
    } else {
        // NOTE: flush logs has been moved outside this method to be able to rollback.
        // backup_->flushLogs(*profile_);
        if (pf->isChanged()) {
            smsc_log_debug(log_,"profile %s needs flush",profkey.c_str());
        }
        ret = true;
        if (profileRequest.hasTiming()) { profileRequest.timingMark("proc_ok"); }
    }
    if ( profileRequest.hasTiming() ) {
        char buf[100];
        snprintf(buf,sizeof(buf)," [%s props=%u]",
                 profileRequest.getProfileKey().toString().c_str(),
                 pf->GetCount());
        profileRequest.timingComment(buf);
    }
    // smsc_log_debug(log_,"FIXME(post): prof=%s",pf->toString().c_str());
    return ret;
    
}


void ProfileCommandProcessor::finishProcessing( bool commit )
{
    if (profile_) {
        if (commit) {
            backup_->flushLogs(*profile_);
        } else {
            backup_->rollback(*profile_);
            if (response_.get()) response_->setStatus(StatusType::SERVER_BUSY);
        }
    }
}


void ProfileCommandProcessor::resetProfile(Profile *pf) 
{
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
    typedef std::vector<BatchRequestComponent*>::iterator BatchIterator;
    if (cmd.isTransactional()) {
        for (BatchIterator i = content.begin(); i != content.end(); ++i) {
            bool result = (*i)->visit(proc);
            resp->addComponent(static_cast<BatchResponseComponent*>(proc.getResponse()));
            if (!result) {
                rollback_ = true;
                return false;
            }
        }
    } else {
        for (BatchIterator i = content.begin(); i != content.end(); ++i) {
            (*i)->visit(proc);
            resp->addComponent(static_cast<BatchResponseComponent*>(proc.getResponse()));
        }
    }
    return true;
}


bool ProfileCommandProcessor::visitGetProfileCommand(GetProfileCommand& cmd)
{
    GetProfileResponse* resp = new GetProfileResponse(StatusType::OK);
    response_.reset(resp);
    if ( !profile_ ) {
        response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    const PropertyHash& ph = profile_->getProperties();
    char* pk;
    Property* prop;
    for ( PropertyHash::Iterator i(&ph); i.Next(pk,prop); ) {
        if (prop) {
            resp->addComponent( new GetProfileResponseComponent(pk) );
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
    } else {
        if (!profile_->AddProperty(prop)) {
            response_->setStatus(StatusType::BAD_REQUEST);
            return false;
        }
        backup_->addProperty(prop);
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
        profile_->setChanged(true);
        return StatusType::OK;
    }

    if (p->getType() == INT && property.getType() == INT) {
        backup_->fixProperty(*p);
        result = static_cast<uint32_t>(p->getIntValue() + property.getIntValue());
        result = mod > 0 ? result % mod : result;
        p->setIntValue(result);
        p->WriteAccess();
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
        backup_->addProperty(*p,true);
        profile_->setChanged(true);
        return StatusType::OK;
    }
    return StatusType::TYPE_INCONSISTENCE;
}

}
}
