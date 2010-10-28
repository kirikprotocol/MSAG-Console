#ifndef _INFORMER_USERINFO_H
#define _INFORMER_USERINFO_H

namespace eyeline {
namespace informer {

class UserInfo
{
public:
    const char* getPassword() const { return "password"; }
    const char* getUserId() const { return "user"; }
};

} // informer
} // smsc

#endif
