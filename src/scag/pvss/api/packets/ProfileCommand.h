#ifndef _SCAG_PVSS_BASE_PROFILECOMMAND_H
#define _SCAG_PVSS_BASE_PROFILECOMMAND_H

#include "scag/pvss/common/PvapException.h"
#include "logger/Logger.h"
#include "util/TypeInfo.h"

namespace scag2 {
namespace pvss {

class ProfileCommandVisitor;

/// The command which can be executed on the profile.
class ProfileCommand
{
protected:
    static smsc::logger::Logger* log_;

public:
    ProfileCommand() {}
    virtual ~ProfileCommand() {}

    // ProfileCommand( uint32_t seqNum ) : seqNum_(seqNum) {}
    
    // virtual uint32_t getSeqNum() const { return seqNum_; }
    // virtual void setSeqNum( uint32_t seqNum ) { seqNum_ = seqNum; }
    virtual std::string toString() const {
        return typeToString();
        // char buf[32];
        // sprintf( buf, "seqNum=%u %s", seqNum_, typeToString() );
        // return buf;
    }

    /// always return true (used in serialization)
    inline bool isRequest() const { return true; }

    /// check if the command is valid (except seqnum field)
    virtual bool isValid( PvssException* exc = 0 ) const = 0;

    /// visitor pattern
    virtual bool visit( ProfileCommandVisitor& visitor ) /* throw (PvapException) */  = 0;

    // virtual ResponseTypeMatch& getResponseTypeMatch() const = 0;

    /// clone the command
    virtual ProfileCommand* clone() const = 0;

    /// clear all fields except seqnum
    virtual void clear() = 0;

    virtual const char* typeToString() const = 0;

protected:
    ProfileCommand( const ProfileCommand& cmd ) {}

    void initLog();
    void logDtor();

private:
    ProfileCommand& operator = ( const ProfileCommand& );

protected:
    DECLMAGTC(ProfileCommand);
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILECOMMAND_H */
