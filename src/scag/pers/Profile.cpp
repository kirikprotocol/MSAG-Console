/* $Id$ */

#include "Profile.h"
#include <unistd.h>

namespace scag{ namespace pers{

void Profile::Serialize(SerialBuffer& buf, bool toFSDB)
{
    uint16_t cnt;

    cnt = properties.GetCount();
    buf.WriteInt8(state);
    buf.WriteInt16(cnt);

    PropertyHash::Iterator it = properties.getIterator();
    Property* prop;
    char *key = 0;
    while(it.Next(key, prop))
        prop->Serialize(buf, toFSDB);
}

void Profile::Deserialize(SerialBuffer& buf, bool fromFSDB)
{
    uint16_t cnt;
    Property* prop;

    state = static_cast<ProfileState>(buf.ReadInt8());
    Empty();
    cnt = buf.ReadInt16();

	time_t cur_time = time(0);
    while(cnt) {
        prop = new Property();
        do{
            prop->Deserialize(buf, fromFSDB);
            cnt--;
            if(log && prop->isExpired(cur_time))
                smsc_log_info(log, "E key=\"%s\" name=%s", pkey.c_str(), prop->getName().c_str());
        }while(prop->isExpired(cur_time) && cnt);

        if(!prop->isExpired(cur_time))
            properties.Insert(prop->getName().c_str(), prop);
        else
            delete prop;
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

bool Profile::PropertyExists(const char* name)
{
    return properties.Exists(name);
}

Property* Profile::GetProperty(const char* name)
{
    try{
        Property *p = properties.Get(name);
        if(p->isExpired())
        {
            if(log) smsc_log_info(log, "E key=\"%s\" name=%s", pkey.c_str(), name);
            properties.Delete(name);
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
	time_t cur_time = time(0);
    while(it.Next(key, prop))
        if(prop->isExpired(cur_time))
        {
            i++;
            if(log) smsc_log_info(log, "E key=\"%s\" name=%s", pkey.c_str(), key);
            delete prop;
            properties.Delete(key);
        }

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
        return;
    }
    catch(HashInvalidKeyException& e)
    {
        if(!prop.isExpired())
            properties.Insert(prop.getName().c_str(), new Property(prop));
    }
}

Profile& Profile::operator=(const Profile& pf) {
  if (this == &pf) {
    return *this;
  }
  const smsc::logger::Logger* _log = pf.getLog();
  if (_log) {
    log = smsc::logger::Logger::getInstance(_log->getName());
  }
  log = NULL;
  pkey = pf.getKey();
  state = pf.getState();

  Empty();

  char *key = 0;
  Property* prop;
  PropertyHash::Iterator pf_it = pf.getProperties().getIterator();
  while (pf_it.Next(key, prop)) {
    properties.Insert(key, new Property(*prop));
  }
  return *this;
}

void Profile::addNewProperty(Property& prop) {
  properties.Insert(prop.getName().c_str(),  new Property(prop));
}

void Profile::copyPropertiesTo(Profile* pf) const {
  char *key = 0;
  Property* prop;

  PropertyHash::Iterator it = properties.getIterator();
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("profile");

  while(it.Next(key, prop)) {
    smsc_log_debug(logger, "copy property key=\'%s\' prop=\'%s\'", key, prop->toString().c_str());
    pf->addNewProperty(*prop);
  }
}

}}
