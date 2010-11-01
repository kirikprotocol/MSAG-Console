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


// number of deliveries by user
struct UserDlvStats
{
    uint32_t paused;
    uint32_t planned;
    uint32_t active;
    uint32_t finished;
    uint32_t cancelled;
};


class UserInfo
{
    friend class EmbedRefPtr< UserInfo >;
public:
    UserInfo( const char* id, const char* pwd );
    ~UserInfo();
    const char* getUserId() const { return userId_.c_str(); }
    const char* getPassword() const { return password_.c_str(); }
    bool hasRole( UserRole role ) const;
    void addRole( UserRole role );

    /// get current stats
    void getStats( UserDlvStats& stats );

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
