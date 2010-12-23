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
    ProfileRequest( ProfileCommand* cmd = 0 ) : command_(cmd), timing_(0) {
        initLog(); 
    }
    ProfileRequest( uint32_t seqNum, ProfileCommand* cmd = 0 ) : Request(seqNum), command_(cmd), timing_(0) {
        initLog(); 
    }
    ProfileRequest( const ProfileKey& profileKey, ProfileCommand* cmd = 0 ) : profileKey_(profileKey), command_(cmd), timing_(0) {
        initLog(); 
    }
    virtual ~ProfileRequest();

    const ProfileKey& getProfileKey() const {
        CHECKMAGTC;
        return profileKey_;
    }
    ProfileKey& getProfileKey() { return profileKey_; }
    void setProfileKey( const ProfileKey& key ) {
        CHECKMAGTC;
        profileKey_ = key; 
    }

    inline const ProfileCommand* getCommand() const {
        CHECKMAGTC;
        return command_;
    }
    inline ProfileCommand* getCommand() {
        CHECKMAGTC;
        return command_; 
    }
    void setCommand( ProfileCommand* cmd ) {
        CHECKMAGTC;
        if (command_==cmd) return;
        if (command_) delete command_;
        command_ = cmd; 
    }
    
    virtual ProfileRequest* clone() const { return new ProfileRequest(*this); }

    virtual bool isValid( PvssException* exc = 0 ) const {
        CHECKMAGTC;
        do {
            if ( !profileKey_.isValid() ) {
                if (exc) {*exc = PvssException("prof key is invalid",PvssException::BAD_REQUEST);}
                break;
            }
            if ( ! getCommand() ) {
                if (exc) {*exc = PvssException("prof has no command",PvssException::BAD_REQUEST);}
                break;
            }
            if ( ! getCommand()->isValid() ) {
                if (exc) {*exc = PvssException(PvssException::BAD_REQUEST,
                                               "prof cmd %s is invalid",
                                               getCommand()->toString().c_str());}
                break;
            }
            return true;
        } while (false);
        return false;
    }

    virtual std::string toString() const {
        CHECKMAGTC;
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
        CHECKMAGTC;
        return visitor.visitProfileRequest(*this);
    }

    void clear() {
        CHECKMAGTC;
        if ( getCommand() ) getCommand()->clear();
    }

    // --- timing
    virtual bool hasTiming() const {
        return ( timing_ ? timing_->timing.isValid() : false );
    }
    virtual void timingMark( const char* where ) const {
        if (!timing_) return;
        timing_->total += timing_->timing.mark(where);
    }
    virtual void timingComment( const char* comment ) const {
        if (!timing_) return;
        timing_->timing.comment(comment);
    }
    virtual const Timing* getTiming() const { return timing_; }
    virtual void mergeTiming( const Response& resp );

    /// --- these extra methods may be used to juggle with timing
    inline void startTiming() {
        if ( ! timing_ ) { timing_ = new Timing; }
    }
    inline void stopTiming() {
        if ( timing_ ) { timing_->timing.stop(); }
    }

protected:
    virtual const char* typeToString() const { return "prof"; }

private:
    ProfileRequest( const ProfileRequest& other ) :
    Request(other), profileKey_(other.profileKey_), command_(other.command_->clone()), timing_(0) {}

private:
    DECLMAGTC(ProfileRequest);
    ProfileKey       profileKey_;
    ProfileCommand*  command_;
    mutable Timing*  timing_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILEREQUEST_H */
