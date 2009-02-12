#ifndef _SCAG_PVSS_BASE_INCCOMMAND_H
#define _SCAG_PVSS_BASE_INCCOMMAND_H

#include "AbstractPropertyCommand.h"

namespace scag2 {
namespace pvss {

class IncCommand : public AbstractPropertyCommand
{
public:
    IncCommand() : AbstractPropertyCommand() {}
    IncCommand( uint32_t seqNum ) : AbstractPropertyCommand(seqNum) {}
    
    virtual bool visit( const ProfileKey& key, ProfileCommandVisitor& visitor ) throw ( PvapException )
    {
        return visitor.visitIncCommand( key, *this );
    }

protected:
    virtual const char* typeToString() const { return "inc"; }
    virtual ResponseTypeMatch& getResponseTypeMatch() const;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_INCCOMMAND_H */
