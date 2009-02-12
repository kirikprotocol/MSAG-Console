#ifndef _SCAG_PVSS_BASE_ABSTRACTCOMMAND_H
#define _SCAG_PVSS_BASE_ABSTRACTCOMMAND_H

#include "Command.h"
#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {

class ProfileKey;
class ProfileCommandVisitor;
class ResponseTypeMatch;

/// The command which can be executed on the profile.
/// NOTE: the class is misnamed (it should be named ProfileCommand)
class AbstractCommand : public Command
{
public:
    AbstractCommand() : seqNum_(uint32_t(-1)) {}
    AbstractCommand( uint32_t seqNum ) : seqNum_(seqNum) {}
    
    virtual uint32_t getSeqNum() const { return seqNum_; }
    virtual void setSeqNum( uint32_t seqNum ) { seqNum_ = seqNum; }
    virtual std::string toString() const {
        char buf[32];
        sprintf( buf, "seqNum=%u %s", seqNum_, typeToString() );
        return buf;
    }
    virtual const char* typeToString() const = 0;

    bool isRequest() const { return true; }

    virtual bool visit( const ProfileKey& key, ProfileCommandVisitor& visitor ) throw (PvapException) = 0;

    virtual ResponseTypeMatch& getResponseTypeMatch() const = 0;

private:
    uint32_t seqNum_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ABSTRACTCOMMAND_H */
