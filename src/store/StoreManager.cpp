#include <stdlib.h>
#include <string.h>

#include "StoreManager.h"

namespace smsc { namespace store 
{

StoreConfig::StoreConfig(const char* db, const char* usr, const char* pwd):
{
    userName = strdup(usr);
    userPwd = strdup(pwd);
    dbName = strdup(db);
}
StoreConfig::~StoreConfig() 
{
    if (userName) free(userName);
    if (userPwd) free(userPwd);
    if (dbName) free(dbName);
}

}}

