#ifndef _INFORMER_USERINFO_H
#define _INFORMER_USERINFO_H

namespace eyeline {
namespace informer {

typedef enum
{
        USERROLE_ADMIN = 1,
        USERROLE_USER = 2
} UserRole;


class UserInfo
{
public:
    const char* getPassword() const { return "password"; }
    const char* getUserId() const { return "user"; }

    bool hasRole( UserRole role ) const { return USERROLE_USER; }
};

} // informer
} // smsc

#endif
