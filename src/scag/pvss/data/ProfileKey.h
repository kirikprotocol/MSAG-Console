#ifndef _SCAG_PVSS_DATA_PROFILEKEY_H
#define _SCAG_PVSS_DATA_PROFILEKEY_H

#include "scag/pvss/common/ScopeType.h"
#include "scag/pvss/profile/AbntAddr.hpp"

namespace scag2 {
namespace pvss {

/// NOTE: we don't need inheritance from IProfileKey (as we could return const ProfileKey& where needed).
class ProfileKey
{
public:
    ProfileKey() : scopeType_( ScopeType(0) ), intKey_(-1) {}
    ~ProfileKey() {}

    ScopeType getScopeType() const { return scopeType_; }

    bool hasAbonentKey() const { return scopeType_ == SCOPE_ABONENT; }
    bool hasOperatorKey() const { return scopeType_ == SCOPE_OPERATOR; }
    bool hasProviderKey() const { return scopeType_ == SCOPE_PROVIDER; }
    bool hasServiceKey() const { return scopeType_ == SCOPE_SERVICE; }

    const std::string& getAbonentKey() const { return abonentKey_; }
    const AbntAddr&    getAddress() const { return address_; }
    int32_t            getOperatorKey() const { return intKey_; }
    int32_t            getProviderKey() const { return intKey_; }
    int32_t            getServiceKey() const { return intKey_; }
    
    void setAbonentKey( const std::string& key ) { scopeType_ = SCOPE_ABONENT; abonentKey_ = key; address_.fromString(abonentKey_.c_str()); }
    void setOperatorKey( int32_t key ) { scopeType_ = SCOPE_OPERATOR; intKey_ = key; }
    void setProviderKey( int32_t key ) { scopeType_ = SCOPE_PROVIDER; intKey_ = key; }
    void setServiceKey( int32_t key ) { scopeType_ = SCOPE_SERVICE; intKey_ = key; }

    std::string toString() const {
        const char* intop;
        switch (scopeType_) {
        case SCOPE_ABONENT : return "abonent=" + abonentKey_;
        case SCOPE_OPERATOR : intop = "operator"; break;
        case SCOPE_PROVIDER : intop = "provider"; break;
        case SCOPE_SERVICE : intop = "service"; break;
        default: return "key=NULL";
        }
        char buf[30];
        sprintf(buf,"%s=%u",intop,intKey_);
        return buf;
    }

    bool isValid() const {
        return scopeType_ != ScopeType(0);
    }

    void clear() {
        scopeType_ = ScopeType(0);
        abonentKey_.clear();
        intKey_ = -1;
    }

    inline bool operator == ( const ProfileKey& key ) const {
      if (scopeType_ != key.getScopeType() ) {
        return false;
      }
      if (scopeType_ == SCOPE_ABONENT) {
        return address_ == key.getAddress();
      }
      return intKey_ == key.getOperatorKey();
    }

    inline bool operator != ( const ProfileKey& key ) const
    {
        return !(*this == key);
    }

private:
    ScopeType   scopeType_;
    std::string abonentKey_;
    AbntAddr    address_; 
    int32_t     intKey_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_COMMAND_H */
