#ifndef _SCAG_PVSS_BASE_INCMODCOMMAND_H
#define _SCAG_PVSS_BASE_INCMODCOMMAND_H

#include "IncCommand.h"

namespace scag2 {
namespace pvss {

class IncModCommand : public IncCommand
{
public:
    IncModCommand() : IncCommand(inherited()), modulus_(0) { initLog(); }
    // IncModCommand( uint32_t seqNum ) : IncCommand(seqNum), modulus_(0) {}
    virtual ~IncModCommand() { logDtor(); }
    
    virtual bool isValid( PvssException* exc = 0 ) const {
        if ( !IncCommand::isValid(exc) ) {return false;}
        //if ( modulus_ == 0 ) {
          //  if (exc) { *exc = PvssException("incmod has mod=0",PvssException::BAD_REQUEST); }
            //return false;
        //}
        return true;
    }
    virtual std::string toString() const {
        char buf[32];
        sprintf(buf, " mod=%u", modulus_ );
        return IncCommand::toString() + buf;
    }

    uint32_t getModulus() const { return modulus_; }
    void setModulus( uint32_t mod ) { modulus_ = mod; }

    virtual bool visit( ProfileCommandVisitor& visitor ) // throw ( PvapException )
    {
        return visitor.visitIncModCommand( *this );
    }

    virtual IncModCommand* clone() const { return new IncModCommand(*this); }

protected:
    virtual const char* typeToString() const { return "incmod"; }

private:
    uint32_t modulus_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_INCMODCOMMAND_H */
