#ifndef _SCAG_PVSS_BASE_PROFILECOMMANDVISITOR_H
#define _SCAG_PVSS_BASE_PROFILECOMMANDVISITOR_H

namespace scag2 {
namespace pvss {

// class ProfileKey;
class DelCommand;
class SetCommand;
class GetCommand;
class IncCommand;
class IncModCommand;
class BatchCommand;
class GetProfileCommand;
class PvapException;

class ProfileCommandVisitor
{
public:
    virtual ~ProfileCommandVisitor() {}
    virtual bool visitDelCommand( DelCommand& cmd ) /* throw(PvapException) */  = 0;
    virtual bool visitSetCommand( SetCommand& cmd ) /* throw(PvapException) */  = 0;
    virtual bool visitGetCommand( GetCommand& cmd ) /* throw(PvapException) */  = 0;
    virtual bool visitIncCommand( IncCommand& cmd ) /* throw(PvapException) */  = 0;
    virtual bool visitIncModCommand( IncModCommand& cmd ) /* throw(PvapException) */  = 0;
    virtual bool visitBatchCommand( BatchCommand& cmd ) /* throw(PvapException) */  = 0;
    virtual bool visitGetProfileCommand( GetProfileCommand& cmd ) { return false; }
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILECOMMANDVISITOR_H */
