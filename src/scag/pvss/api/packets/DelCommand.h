#ifndef _SCAG_PVSS_BASE_DELCOMMAND_H
#define _SCAG_PVSS_BASE_DELCOMMAND_H

#include "BatchRequestComponent.h"
#include "ProfileCommandVisitor.h"

namespace scag2 {
namespace pvss {

class DelCommand : public BatchRequestComponent
{
public:
    DelCommand() : BatchRequestComponent() {}
    DelCommand( uint32_t seqNum ) : BatchRequestComponent(seqNum) {}
    
    virtual bool isValid() const { return !varName_.empty(); }
    virtual const char* typeToString() const { return "del"; }
    virtual std::string toString() const {
        return BatchRequestComponent::toString() + " var=\"" + varName_ + "\"";
    }
    virtual void clear() { varName_.clear(); }
    
    const std::string& getVarName() const { return varName_; }
    void setVarName( const std::string& varName ) { varName_ = varName; }

    virtual bool visit( ProfileCommandVisitor& visitor ) throw ( PvapException )
    {
        return visitor.visitDelCommand(*this);
    }

    virtual DelCommand* clone() const { return new DelCommand(*this); }

protected:
    virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    std::string varName_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_DELCOMMAND_H */
