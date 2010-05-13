#ifndef _SCAG_PVSS_BASE_GETPROFILECOMMAND_H
#define _SCAG_PVSS_BASE_GETPROFILECOMMAND_H

#include "ProfileCommand.h"
#include "ProfileCommandVisitor.h"

namespace scag2 {
namespace pvss {

class GetProfileCommand : public ProfileCommand
{
public:
    GetProfileCommand() { initLog(); }
    // GetCommand( uint32_t seqNum ) : BatchRequestComponent(seqNum) {}
    virtual ~GetProfileCommand() { logDtor(); }

    virtual bool isValid( PvssException* exc = 0 ) const {
        /*
        if ( varName_.empty() ) {
            if (exc) { *exc = PvssException("del has no name",PvssException::BAD_REQUEST); }
            return false;
        }
         */
        return true;
    }
    /*
    virtual std::string toString() const {
        return BatchRequestComponent::toString() + " var=\"" + varName_ + "\"";
    }
     */
    virtual void clear() {}
    
    /*
    const std::string& getVarName() const { return varName_; }
    void setVarName( const std::string& varName ) { varName_ = varName; }
     */

    virtual bool visit( ProfileCommandVisitor& visitor ) throw ( PvapException )
    {
        return visitor.visitGetProfileCommand(*this);
    }

    virtual GetProfileCommand* clone() const { return new GetProfileCommand(*this); }

protected:
    virtual const char* typeToString() const { return "get_prof"; }
    // virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    // std::string varName_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_GETCOMMAND_H */
