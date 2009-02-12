#ifndef _SCAG_PVSS_BASE_PROFILECOMMANDVISITOR_H
#define _SCAG_PVSS_BASE_PROFILECOMMANDVISITOR_H

namespace scag2 {
namespace pvss {

class ProfileKey;
class DelCommand;
class SetCommand;
class GetCommand;
class IncCommand;
class IncModCommand;
class BatchCommand;
class PvapException;

class ProfileCommandVisitor
{
public:
    virtual ~ProfileCommandVisitor() {}
    virtual bool visitDelCommand( const ProfileKey& key, DelCommand& cmd ) throw(PvapException) = 0;
    virtual bool visitSetCommand( const ProfileKey& key, SetCommand& cmd ) throw(PvapException) = 0;
    virtual bool visitGetCommand( const ProfileKey& key, GetCommand& cmd ) throw(PvapException) = 0;
    virtual bool visitIncCommand( const ProfileKey& key, IncCommand& cmd ) throw(PvapException) = 0;
    virtual bool visitIncModCommand( const ProfileKey& key, IncModCommand& cmd ) throw(PvapException) = 0;
    virtual bool visitBatchCommand( const ProfileKey& key, BatchCommand& cmd ) throw(PvapException) = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILECOMMANDVISITOR_H */
