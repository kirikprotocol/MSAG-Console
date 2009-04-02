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

    virtual bool isValid() const { return !varName_.empty(); }
    virtual std::string toString() const {
        return BatchRequestComponent::toString() + " var=\"" + varName_ + "\"";
    }
    virtual void clear() { varName_.clear(); }
    
    const std::string& getVarName() const { return varName_; }
    void setVarName( const std::string& varName ) { varName_ = varName; }

    virtual bool visit( ProfileCommandVisitor& visitor ) throw ( PvapException )
    {
        return visitor.visitGetCommand(*this);
    }

    virtual GetCommand* clone() const { return new GetCommand(*this); }

protected:
    virtual const char* typeToString() const { return "get"; }
    // virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    std::string varName_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_GETCOMMAND_H */
