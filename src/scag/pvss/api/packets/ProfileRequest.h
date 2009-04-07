#ifndef _SCAG_PVSS_BASE_PROFILEREQUEST_H
#define _SCAG_PVSS_BASE_PROFILEREQUEST_H

#include "Request.h"
#include "ProfileCommand.h"
#include "RequestVisitor.h"
#include "scag/pvss/data/ProfileKey.h"

namespace scag2 {
namespace pvss {

///
/// Abstract profile request class
///
class ProfileRequest : public Request
{
public:
    ProfileRequest( ProfileCommand* cmd = 0 ) : command_(cmd), context_(0) { initLog(); }
    ProfileRequest( uint32_t seqNum, ProfileCommand* cmd = 0 ) : Request(seqNum), command_(cmd), context_(0) { initLog(); }
    ProfileRequest( const ProfileKey& profileKey, ProfileCommand* cmd = 0 ) : profileKey_(profileKey), command_(cmd), context_(0) { initLog(); }
    virtual ~ProfileRequest() {
        logDtor();
        if (context_) { smsc_log_warn( log_, "request %p has extra context %p at dtor", this, context_ ); }
        if (command_) { delete command_; }
    }

    const ProfileKey& getProfileKey() const { return profileKey_; }
    ProfileKey& getProfileKey() { return profileKey_; }
    void setProfileKey( const ProfileKey& key ) { profileKey_ = key; }

    inline const ProfileCommand* getCommand() const { return command_; }
    inline ProfileCommand* getCommand() { return command_; }
    void setCommand( ProfileCommand* cmd ) {
        if (command_==cmd) return;
        if (command_) delete command_;
        command_ = cmd; 
    }
    
    virtual ProfileRequest* clone() const { return new ProfileRequest(*this); }

    virtual bool isValid() const {
        return profileKey_.isValid() && getCommand() && getCommand()->isValid();
    }

    virtual std::string toString() const {
        std::string rv(Request::toString());
        rv.reserve(100);
        rv += " ";
        rv += profileKey_.toString();
        // if ( profileKey_.isValid() ) rv += profileKey_.toString();
        // else rv = "key=NULL";
        if ( getCommand() ) {
            rv += " ";
            rv += getCommand()->toString();
        }
        return rv;
    }

    virtual bool visit( RequestVisitor& visitor ) /* throw (PvapException) */ 
    {
        return visitor.visitProfileRequest(*this);
    }

    void clear() {
        if ( getCommand() ) getCommand()->clear();
    }

    inline void* getExtraContext() { return context_; }
    inline void setExtraContext( void* ctx ) { context_ = ctx; }

protected:
    virtual const char* typeToString() const { return "prof"; }

private:
    ProfileRequest( const ProfileRequest& other ) :
    Request(other), profileKey_(other.profileKey_), command_(other.command_->clone()) {}

private:
    ProfileKey       profileKey_;
    ProfileCommand*  command_;
    void*            context_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILEREQUEST_H */
