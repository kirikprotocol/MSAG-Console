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

    bool hasAbonentKey() const { return scopeType_ == ABONENT; }
    bool hasOperatorKey() const { return scopeType_ == OPERATOR; }
    bool hasProviderKey() const { return scopeType_ == PROVIDER; }
    bool hasServiceKey() const { return scopeType_ == SERVICE; }

    const std::string& getAbonentKey() const { return abonentKey_; }
    const AbntAddr&    getAddress() const { return address_; }
    int32_t            getOperatorKey() const { return intKey_; }
    int32_t            getProviderKey() const { return intKey_; }
    int32_t            getServiceKey() const { return intKey_; }
    
    void setAbonentKey( const std::string& key ) { scopeType_ = ABONENT; abonentKey_ = key; address_.fromString(abonentKey_.c_str()); }
    void setOperatorKey( int32_t key ) { scopeType_ = OPERATOR; intKey_ = key; }
    void setProviderKey( int32_t key ) { scopeType_ = PROVIDER; intKey_ = key; }
    void setServiceKey( int32_t key ) { scopeType_ = SERVICE; intKey_ = key; }

    std::string toString() const {
        switch (scopeType_) {
        case ABONENT : return "abonent=" + abonentKey_;
        case OPERATOR : return "operator=" + intKey_;
        case PROVIDER : return "provider=" + intKey_;
        case SERVICE : return "service=" + intKey_;
        default: return "key=NULL";
        }
    }

    bool isValid() const {
        return scopeType_ != ScopeType(0);
    }

    void clear() {
        scopeType_ = ScopeType(0);
        abonentKey_.clear();
        intKey_ = -1;
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
