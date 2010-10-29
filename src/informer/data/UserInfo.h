#ifndef _INFORMER_USERINFO_H
#define _INFORMER_USERINFO_H

#include "core/buffers/FixedLengthString.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "util/int.h"

namespace eyeline {
namespace informer {

typedef smsc::core::buffers::FixedLengthString<64> userid_type;

typedef enum
{
    USERROLE_ADMIN = 0,
    USERROLE_USER = 1
} UserRole;


class UserInfo
{
    friend class EmbedRefPtr< UserInfo >;
public:
    UserInfo( const char* id, const char* pwd );
    const char* getUserId() const { return userId_.c_str(); }
    const char* getPassword() const { return password_.c_str(); }
    bool hasRole( UserRole role ) const;
    void addRole( UserRole role );

private:
    void ref();
    void unref();
private:
    smsc::core::synchronization::Mutex lock_;
    unsigned    ref_;
    userid_type userId_;
    std::string password_;
    uint64_t    roles_;
};

typedef EmbedRefPtr<UserInfo> UserInfoPtr;

} // informer
} // smsc

#endif
