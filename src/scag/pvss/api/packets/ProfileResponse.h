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
    ProfileResponse( CommandResponse* cmd = 0 ) : Response(), cmd_(cmd) { initLog(); }
    ProfileResponse( uint32_t seqNum, CommandResponse* cmd = 0 ) : Response(seqNum), cmd_(cmd) { initLog(); }
    virtual ~ProfileResponse() { logDtor(); clear(); }

    virtual uint8_t getStatus() const { return cmd_ ? cmd_->getStatus() : UNKNOWN; }

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitProfileResponse(*this);
    }
    virtual ProfileResponse* clone() const { return new ProfileResponse(*this); }
    virtual bool isPing() const { return false; }
    virtual void clear() { delete cmd_; cmd_ = 0; }

    inline CommandResponse* getResponse() { return cmd_; }
    inline const CommandResponse* getResponse() const { return cmd_; }
    inline void setResponse( CommandResponse* resp ) { clear(); cmd_ = resp; }

protected:
    virtual const char* typeToString() const { return "prof_resp"; }

private:
    ProfileResponse( const ProfileResponse& other ) : Response(other), cmd_(other.cmd_?other.cmd_->clone():0) {
        initLog();
    }
    ProfileResponse& operator = ( const ProfileResponse& other );

private:
    CommandResponse* cmd_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILERESPONSE_H */
