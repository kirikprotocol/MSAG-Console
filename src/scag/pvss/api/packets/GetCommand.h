#ifndef _SCAG_PVSS_BASE_GETCOMMAND_H
#define _SCAG_PVSS_BASE_GETCOMMAND_H

#include "BatchRequestComponent.h"
#include "ProfileCommandVisitor.h"

namespace scag2 {
namespace pvss {

class GetCommand : public BatchRequestComponent
{
public:
    GetCommand() : BatchRequestComponent() { initLog(); }
    // GetCommand( uint32_t seqNum ) : BatchRequestComponent(seqNum) {}
    virtual ~GetCommand() { logDtor(); }

    virtual bool isValid( PvssException* exc = 0 ) const {
        CHECKMAGTC;
        if ( varName_.empty() ) {
            if (exc) { *exc = PvssException("del has no name",PvssException::BAD_REQUEST); }
            return false;
        }
        return true;
    }
    virtual std::string toString() const {
        CHECKMAGTC;
        return BatchRequestComponent::toString() + " var=\"" + varName_ + "\"";
    }
    virtual void clear() { varName_.clear(); }
    
    const std::string& getVarName() const { return varName_; }
    void setVarName( const std::string& varName ) { varName_ = varName; }

    virtual bool visit( ProfileCommandVisitor& visitor ) // throw ( PvapException )
    {
        CHECKMAGTC;
        return visitor.visitGetCommand(*this);
    }

    virtual GetCommand* clone() const {
        CHECKMAGTC;
        return new GetCommand(*this); 
    }

protected:
    virtual const char* typeToString() const { return "get"; }
    // virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    std::string varName_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_GETCOMMAND_H */
