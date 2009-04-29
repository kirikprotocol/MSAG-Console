#ifndef _SCAG_PVSS_BASE_PROFILEREQUEST_H
#define _SCAG_PVSS_BASE_PROFILEREQUEST_H

#include "Request.h"
#include "ProfileCommand.h"
#include "RequestVisitor.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/util/HRTimer.h"

namespace scag2 {
namespace pvss {

///
/// Abstract profile request class
///
class ProfileRequest : public Request
{
private:
    struct Timing {
        std::string    result;
        util::HRTiming timing;
        unsigned       total;
        Timing() : total(0) {
            timing.reset(result);
            result.reserve(400);
        }
    };

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
    

    /// --- these extra methods may be used to juggle with timing
    inline const std::string* getTimingResult() const {
        return timing_ ? &(timing_->result) : 0;
    }
    inline unsigned getTimingTotal() const {
        return timing_ ? timing_->total : 0;
    }
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
    ProfileKey       profileKey_;
    ProfileCommand*  command_;
    mutable Timing*  timing_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILEREQUEST_H */
