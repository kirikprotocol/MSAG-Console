#include <stdlib.h>
#include <string.h>

#include "sms.h"

namespace smsc { namespace sms
{

/* --------------- StoreConfig ------------------ */
StoreConfig::StoreConfig(
    int _type, const char* db, const char* user, const char* pwd) 
        : StoreConfig(_type) 
{
    userName = strdup(user);
    userPwd = strdup(pwd);
    dbName = strdup(db);    
}
StoreConfig::~StoreConfig()
{
    if (userName) free(userName);
    if (userPwd) free(userPwd);
    if (dbName) free(dbName);
}
/* --------------- StoreConfig ------------------ */

/* -------------------- SMS --------------------- */
char* SMS::getDecodedText() 
{
    return 0L;
}
/* -------------------- SMS --------------------- */

}}


