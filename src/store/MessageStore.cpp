#include <stdlib.h>
#include <string.h>

#include "MessageStore.h"

namespace smsc { namespace store 
{

StoreConfig::StoreConfig(int _type, 
    const char* db, const char* usr, const char* pwd):
	type(_type)
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

