#ifndef _SCAG_PVSS_BASE_INCCOMMAND_H
#define _SCAG_PVSS_BASE_INCCOMMAND_H

#include "AbstractPropertyCommand.h"

namespace scag2 {
namespace pvss {

class IncCommand : public AbstractPropertyCommand
{
protected:
    struct inherited {};
    IncCommand( const inherited& ) : inherited_(true) {}

public:
    IncCommand() : AbstractPropertyCommand(), inherited_(false) { initLog(); }
    // IncCommand( uint32_t seqNum ) : AbstractPropertyCommand(seqNum) {}
    virtual ~IncCommand() { if (!inherited_) logDtor(); }
    
    virtual bool isValid() const {
        return AbstractPropertyCommand::isValid() && hasIntValue();
    }

    virtual bool visit( ProfileCommandVisitor& visitor ) throw ( PvapException )
    {
        return visitor.visitIncCommand( *this );
    }

    virtual IncCommand* clone() const { return new IncCommand(*this); }

protected:
    virtual const char* typeToString() const { return "inc"; }
    // virtual ResponseTypeMatch& getResponseTypeMatch() const;
private:
    bool inherited_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_INCCOMMAND_H */
