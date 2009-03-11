#ifndef _SCAG_PVSS_BASE_ABSTRACTPROFILEREQUEST_H
#define _SCAG_PVSS_BASE_ABSTRACTPROFILEREQUEST_H

#include "Request.h"
#include "scag/pvss/data/ProfileKey.h"
#include "AbstractCommand.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace pvss {

///
/// Abstract profile request class
///
class AbstractProfileRequest : public Request
{
protected:
    AbstractProfileRequest() : Request() {}
    AbstractProfileRequest( const ProfileKey& key ) : profileKey_(key) {}
    
public:
    const ProfileKey& getProfileKey() const { return profileKey_; }
    ProfileKey& getProfileKey() { return profileKey_; }
    void setProfileKey( const ProfileKey& key ) { profileKey_ = key; }

    virtual uint32_t getSeqNum() const { return getCommand()->getSeqNum(); }
    virtual void setSeqNum( uint32_t seqNum ) { return getCommand()->setSeqNum(seqNum); }

    virtual bool isValid() const {
        return profileKey_.isValid() && getCommand()->isValid();
    }
    virtual std::string toString() const {
        std::string rv;
        rv.reserve(100);
        if ( profileKey_.isValid() ) rv = profileKey_.toString();
        else rv = "key=NULL";
        rv += " ";
        rv += getCommand()->toString();
        return rv;
    }
    virtual const AbstractCommand* getCommand() const = 0;
    virtual AbstractCommand* getCommand() = 0;
    void clear() {
        profileKey_.clear();
        getCommand()->clear();
    }

    virtual bool visit( RequestVisitor& visitor ) throw (PvapException)
    {
        return visitor.visitProfileRequest(*this);
    }
    virtual AbstractProfileRequest* clone() const = 0;

    /// virtual ctor from AbstractCommand
    static AbstractProfileRequest* create(AbstractCommand* cmd);

protected:
    virtual ResponseTypeMatch& getResponseTypeMatch() const {
        return getCommand()->getResponseTypeMatch();
    }

private:
    ProfileKey profileKey_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ABSTRACTPROFILEREQUEST_H */
