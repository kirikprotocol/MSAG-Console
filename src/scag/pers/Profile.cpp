/* $Id$ */

#include "Profile.h"

namespace scag{ namespace pers{

void Profile::Serialize(SerialBuffer& buf)
{
    char *key = 0;
    uint16_t cnt;
    Property* prop;

    PropertyHash::Iterator it = properties.getIterator();
    cnt = properties.GetCount();
    smsc_log_debug(log, "before store profile size: %d", (int)cnt);
    buf.WriteInt16(cnt);

    while(it.Next(key, prop))
        prop->Serialize(buf);
}

void Profile::Deserialize(SerialBuffer& buf)
{
    const char *key;
    uint16_t cnt;
    Property* prop;

    Empty();
    cnt = buf.ReadInt16();
    smsc_log_debug(log, "profile size: %d", (int)cnt);

    while(cnt--) {
        prop = new Property();
        prop->Deserialize(buf);

        if(!prop->isExpired()) {
            key = prop->getName().c_str();
            properties.Insert(key, prop);
        }
    }
}

Profile::~Profile()
{
    char *key = 0;
    Property *prop;

    PropertyHash::Iterator it = properties.getIterator();

    while(it.Next(key, prop))
        if(prop) delete prop;
}

bool Profile::PropertyExists(const char* str)
{
    return properties.Exists(str);
}

Property* Profile::GetProperty(const char* str)
{
    try{
        Property *p = properties.Get(str);
        if(p->isExpired())
        {
            properties.Delete(str);
            delete p;
            return NULL;
        }
        return p;
    } 
    catch(HashInvalidKeyException &k)
    {
        return NULL;
    }
}

bool Profile::DeleteProperty(const char* str)
{
    try{
        Property *prop = properties.Get(str);
        delete prop;
        properties.Delete(str);
        return true;
    }
    catch(HashInvalidKeyException& e)
    {
        return false;
    }
}

void Profile::DeleteExpired()
{
    int i = 0;
    char *key = 0;
    Property* prop;

    PropertyHash::Iterator it = properties.getIterator();

    while(it.Next(key, prop))
        if(prop->isExpired())
        {
            i++;
            delete prop;
            properties.Delete(key);
        }
    smsc_log_debug(log, "Delete %d expired properties.", i);
}

void Profile::Empty()
{
    char *key = 0;
    Property* prop;

    PropertyHash::Iterator it = properties.getIterator();

    while(it.Next(key, prop))
        delete prop;

    properties.Empty();
}

void Profile::AddProperty(Property& prop)
{
    try{
        Property *p = properties.Get(prop.getName().c_str());
        *p = prop;
        smsc_log_debug(log, "property %s already exists", prop.getName().c_str());
        return;
    }
    catch(HashInvalidKeyException& e)
    {
        if(!prop.isExpired())
            properties.Insert(prop.getName().c_str(), new Property(prop));
    }
}

}}
