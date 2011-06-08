#ifndef _SCAG_PVSS_BASE_PROFILERESPONSE_H
#define _SCAG_PVSS_BASE_PROFILERESPONSE_H

#include "Response.h"
#include "ResponseVisitor.h"
#include "CommandResponse.h"

namespace scag2 {
namespace pvss {

class ProfileResponse : public Response
{
public:
    ProfileResponse( CommandResponse* cmd = 0 ) : Response(), cmd_(cmd), timing_(0) { initLog(); }
    ProfileResponse( uint32_t seqNum, CommandResponse* cmd = 0 ) : Response(seqNum), cmd_(cmd), timing_(0) { initLog(); }
    virtual ~ProfileResponse() {
        logDtor();
        clear();
    }

    virtual uint8_t getStatus() const {
        CHECKMAGTC;
        return cmd_ ? cmd_->getStatus() : UNKNOWN;
    }

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        CHECKMAGTC;
        return visitor.visitProfileResponse(*this);
    }
    virtual ProfileResponse* clone() const {
        CHECKMAGTC;
        return new ProfileResponse(*this);
    }
    virtual bool isPing() const { return false; }
    virtual void clear() {
        if (timing_) {
            // note: timing report is done in request
            delete timing_; timing_ = 0;
        }
        delete cmd_; cmd_ = 0; 
    }

    inline CommandResponse* getResponse() {
        CHECKMAGTC;
        return cmd_;
    }
    inline const CommandResponse* getResponse() const {
        CHECKMAGTC;
        return cmd_;
    }
    inline void setResponse( CommandResponse* resp ) {
        CHECKMAGTC;
        clear(); cmd_ = resp;
    }

    // --- timing
    virtual void startTiming( const Request& req ) {
        if (!timing_) {
            const Timing* t = req.getTiming();
            if (t) timing_ = new Timing(*t);
        }
    }
    virtual bool hasTiming() const {
        return (timing_ ? timing_->timing.isValid() : false );
    }
    virtual void timingMark( const char* where ) const {
        if (!timing_) return;
        timing_->total += timing_->timing.mark(where);
    }
    virtual void timingComment( const char* comment ) const {
        if (!timing_) return;
        timing_->timing.comment(comment);
    }

protected:
    virtual const char* typeToString() const { return "prof_resp"; }
    virtual std::string statusHolderToString() const {
        return getResponse() ? getResponse()->toString() : "";
    }

private:
    ProfileResponse( const ProfileResponse& other ) : Response(other), cmd_(other.cmd_?other.cmd_->clone():0) {
        initLog();
    }
    ProfileResponse& operator = ( const ProfileResponse& other );

private:
    DECLMAGTC(ProfileResponse);
    CommandResponse* cmd_;
    mutable Timing*  timing_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILERESPONSE_H */
