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

bool ProfileCommandProcessor::ReadonlyFilter
    ::applyCommonLogic( const std::string& profkey,
                        ProfileRequest&    profileRequest,
                        Profile*           pf,
                        bool               createProfile )
{
    if (pf) {
        if (profileRequest.hasTiming()) { profileRequest.timingMark("pfgot"); }
        // smsc_log_debug(log_,"FIXME(pre): prof=%s",pf->toString().c_str());
        if ( pf->getKey() != profkey ) {
            smsc_log_warn(proc_.log_,"key mismatch: pf=%p pf.key=%s req.key=%s",
                          pf,pf->getKey().c_str(),profkey.c_str());
            pf->setKey(profkey);
        }
    }

    // full cleanup
    proc_.resetProfile(pf);
    profileRequest.getCommand()->visit(*this);
    if (!pf) {
        if (createProfile) {
            smsc_log_warn(proc_.log_, "%p: req=%p can't create profile %s", this, &profileRequest, profkey.c_str());
        }
        return false;
    }

    bool ret;
    if (proc_.rollback_) {
        smsc_log_debug(proc_.log_, "%p: %p rollback profile %s changes", this, &profileRequest, profkey.c_str());
        proc_.backup_->rollback(*proc_.profile_);
        proc_.profile_->setChanged(false);
        ret = false;
        if (profileRequest.hasTiming()) { profileRequest.timingMark("rollback"); }
    } else {
        // NOTE: flush logs has been moved outside this method to be able to rollback.
        // backup_->flushLogs(*profile_);
        if (pf->isChanged()) {
            smsc_log_debug(proc_.log_,"profile %s needs flush",profkey.c_str());
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


bool ProfileCommandProcessor::ReadonlyFilter::visitBatchCommand(BatchCommand &cmd) /* throw(PvapException) */  
{
    ProfileCommandProcessor proc(*proc_.backup_,proc_.profile_);
    BatchResponse* resp = new BatchResponse(StatusType::OK);
    proc_.response_.reset(resp);
    std::vector<BatchRequestComponent*> content = cmd.getBatchContent();
    typedef std::vector<BatchRequestComponent*>::iterator BatchIterator;
    ReadonlyFilter rf(proc,readonly_);
    if (cmd.isTransactional()) {
        for (BatchIterator i = content.begin(); i != content.end(); ++i) {
            bool result = (*i)->visit(rf);
            resp->addComponent(static_cast<BatchResponseComponent*>(proc.getResponse()));
            if (!result) {
                proc_.rollback_ = true;
                return false;
            }
        }
    } else {
        for (BatchIterator i = content.begin(); i != content.end(); ++i) {
            (*i)->visit(rf);
            resp->addComponent(static_cast<BatchResponseComponent*>(proc.getResponse()));
        }
    }
    return true;
}


bool ProfileCommandProcessor::ReadonlyFilter::visitGetProfileCommand(GetProfileCommand& cmd)
{
    GetProfileResponse* resp = new GetProfileResponse(StatusType::OK);
    proc_.response_.reset(resp);
    if ( !proc_.profile_ ) {
        proc_.response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    const PropertyHash& ph = proc_.profile_->getProperties();
    char* pk;
    Property* prop;
    for ( PropertyHash::Iterator i(&ph); i.Next(pk,prop); ) {
        if (prop) {
            resp->addComponent( new GetProfileResponseComponent(pk) );
        }
    }
    return true;
}


bool ProfileCommandProcessor::ReadonlyFilter::visitDelCommand(DelCommand &cmd) /* throw(PvapException) */  {
    if (readonly_) {
        throw smsc::util::Exception("in readonly mode");
    }
    proc_.response_.reset(new DelResponse());    
    std::auto_ptr<Property> holder;
    if (!proc_.profile_ || !proc_.profile_->DeleteProperty(cmd.getVarName().c_str(),&holder)) {
        proc_.response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    proc_.backup_->delProperty(holder.release());
    proc_.response_->setStatus(StatusType::OK);
    proc_.profile_->setChanged(true);
    return true;
}


bool ProfileCommandProcessor::ReadonlyFilter::visitGetCommand(GetCommand &cmd) /* throw(PvapException) */  {
    proc_.response_.reset(new GetResponse());
    if (!proc_.profile_) {
        proc_.response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    Property* p = proc_.profile_->GetProperty(cmd.getVarName().c_str());
    if (!p) {
        proc_.response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    proc_.backup_->getProperty(*p);
    if(p->getTimePolicy() == R_ACCESS) {
        if (!readonly_) {
            // throw smsc::util::Exception("in readonly mode");
            proc_.backup_->fixProperty(*p);
            p->ReadAccess();
            proc_.profile_->setChanged(true);
            proc_.backup_->propertyUpdated(*p);
        }
    }

    proc_.response_->setStatus(StatusType::OK);
    static_cast<GetResponse*>(proc_.response_.get())->setProperty(*p);
    return true;
}

bool ProfileCommandProcessor::ReadonlyFilter::visitIncCommand(IncCommand &cmd) /* throw(PvapException) */  {
    if (readonly_) {
        throw smsc::util::Exception("in readonly mode");
    }
    uint32_t result = 0;
    uint8_t status = proc_.incModProperty(cmd.getProperty(), 0, result);
    IncResponse *incResp = new IncResponse(status);
    incResp->setIncResult(cmd.isIncResult());
    proc_.response_.reset( incResp );
    if (status != StatusType::OK) {
        return false;
    }
    static_cast<IncResponse*>(proc_.response_.get())->setResult(result);
    return true;
}

bool ProfileCommandProcessor::ReadonlyFilter::visitIncModCommand(IncModCommand &cmd) /* throw(PvapException) */  {
    if (readonly_) {
        throw smsc::util::Exception("in readonly mode");
    }
    uint32_t result = 0;
    proc_.response_.reset( new IncResponse());
    if (!proc_.profile_) {
        proc_.response_->setStatus(StatusType::IO_ERROR);
        return false;
    }
    uint8_t status = proc_.incModProperty(cmd.getProperty(), cmd.getModulus(), result);
    proc_.response_->setStatus(status);
    if (status != StatusType::OK) {
        return false;
    }
    static_cast<IncResponse*>(proc_.response_.get())->setResult(result);
    return true;
}

bool ProfileCommandProcessor::ReadonlyFilter::visitSetCommand(SetCommand &cmd) /* throw(PvapException) */  {
    if (readonly_) {
        throw smsc::util::Exception("in readonly mode");
    }
    const Property& prop = cmd.getProperty();
    proc_.response_.reset( new SetResponse() );
    if (!proc_.profile_) {
        proc_.response_->setStatus(StatusType::IO_ERROR);
        return false;
    }
    if (prop.isExpired()) {
        proc_.response_->setStatus(StatusType::PROPERTY_NOT_FOUND);
        return false;
    }
    Property* p = proc_.profile_->GetProperty(prop.getName());
    if (p != NULL) {
        proc_.backup_->fixProperty(*p);
        p->setValue(prop);
        // FIXED by bukind on 20100209, now timepolicy is also updated!
        p->setTimePolicy(prop.getTimePolicy(),prop.getFinalDate(),prop.getLifeTime());
        p->WriteAccess();
        proc_.backup_->propertyUpdated(*p);
    } else {
        if (!proc_.profile_->AddProperty(prop)) {
            proc_.response_->setStatus(StatusType::BAD_REQUEST);
            return false;
        }
        proc_.backup_->addProperty(prop);
    }
    proc_.profile_->setChanged(true);
    proc_.response_->setStatus(StatusType::OK);
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
