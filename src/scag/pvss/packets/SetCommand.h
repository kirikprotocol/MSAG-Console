#ifndef _SCAG_PVSS_BASE_SETCOMMAND_H
#define _SCAG_PVSS_BASE_SETCOMMAND_H

#include "AbstractPropertyCommand.h"

namespace scag2 {
namespace pvss {

class SetCommand : public AbstractPropertyCommand
{
public:
    SetCommand() : AbstractPropertyCommand() {}
    SetCommand( uint32_t seqNum ) : AbstractPropertyCommand(seqNum) {}
    
    virtual bool visit( ProfileCommandVisitor& visitor ) throw ( PvapException )
    {
        return visitor.visitSetCommand(*this);
    }

    virtual SetCommand* clone() const { return new SetCommand(*this); }

protected:
    virtual const char* typeToString() const { return "set"; }
    virtual ResponseTypeMatch& getResponseTypeMatch() const;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SETCOMMAND_H */
