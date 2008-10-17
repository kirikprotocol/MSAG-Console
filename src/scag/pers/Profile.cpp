/* $Id$ */

#include "Profile.h"
#include <unistd.h>

namespace scag{ namespace pers{

const uint8_t PROPERTIES_COUNT_SIZE = 14; //14 bits for profile properties count
const uint16_t MAX_PROPERTIES_COUNT = 16383; 

void Profile::Serialize(SerialBuffer& buf, bool toFSDB, GlossaryBase* glossary) const
{
    uint16_t cnt = properties.GetCount();
    if (cnt == 0 && toFSDB && state != LOCKED) {
      return;
    }
    uint16_t serialized_state = (uint16_t)state << PROPERTIES_COUNT_SIZE;
    serialized_state |= cnt;
    buf.WriteInt16(serialized_state);

    PropertyHash::Iterator it = properties.getIterator();
    Property* prop;
    char *key = 0;
    while(it.Next(key, prop))
        prop->Serialize(buf, toFSDB, glossary);
}

void Profile::Deserialize(SerialBuffer& buf, bool fromFSDB, GlossaryBase* glossary)
{
    if (!buf.GetSize() || buf.GetPos() >= buf.GetSize()) {
      Empty();
      return;
    }
    uint16_t state_cnt = buf.ReadInt16();
    uint16_t cnt = state_cnt & MAX_PROPERTIES_COUNT;
    state_cnt >>= PROPERTIES_COUNT_SIZE;
    state = static_cast<ProfileState>(state_cnt);

    Empty();
    Property* prop;
	time_t cur_time = time(0);
    while(cnt) {
        prop = new Property();
        do{
            prop->Deserialize(buf, fromFSDB, glossary);
            cnt--;
            if(log && prop->isExpired(cur_time))
                smsc_log_info(log, "E key=\"%s\" name=%s", pkey.c_str(), prop->getName());
        }while(prop->isExpired(cur_time) && cnt);

        if(!prop->isExpired(cur_time))
            properties.Insert(prop->getName(), prop);
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
  Property **p = properties.GetPtr(name);
  if (!p) {
    return NULL;
  }
  if(*p && (*p)->isExpired())
  {
      if(log) smsc_log_info(log, "E key=\"%s\" name=%s", pkey.c_str(), name);
      delete *p;
      properties.Delete(name);
      return NULL;
  }
  return *p;
}

bool Profile::DeleteProperty(const char* name)
{
  Property **prop = properties.GetPtr(name);
  if (!prop) {
    return false;
  }
  if (*prop) {
    delete *prop;
  }
  properties.Delete(name);
  return true;
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

bool Profile::AddProperty(Property& prop)
{
  uint16_t cnt = properties.GetCount();
  if (cnt == MAX_PROPERTIES_COUNT) {
    smsc_log_warn(log, "can't add property \'%s\', profile key=%s already has maximum properties count=%d",
                  prop.getName(), pkey.c_str(), cnt);
    return false;
  }
  properties.Insert(prop.getName(), new Property(prop));
  return true;
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
  properties.Insert(prop.getName(),  new Property(prop));
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

void Profile::addDataToBackup(long nextBlock) {
  backup.push_back(nextBlock);
}
void Profile::clearBackup() {
  backup.clear();
}
const vector<long>& Profile::getBackup() const {
  return backup;
}
void Profile::setBackup(const vector<long>& _backup) {
  backup = _backup;
}

int Profile::getBackupDataSize() const {
  return dataCopy.length();
}

const char* Profile::getBackupData() const {
  return dataCopy.c_ptr();
}

void Profile::setBackupData(const SerialBuffer& data) {
  dataCopy.blkcpy(data.c_ptr(), data.length());
}

void Profile::restoreBackup(const vector<long>& _backup, int backupSize) {
  clearBackup();
  backup.assign(_backup.begin(), _backup.begin() + backupSize);
  dataCopy.setPos(0);
  Deserialize(dataCopy, true);
}

}}

/*
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
*/
/*
void Profile::AddProperty(Property& prop)
{
    try{
        Property *p = properties.Get(prop.getName());
        *p = prop;
        return;
    }
    catch(HashInvalidKeyException& e)
    {
      uint16_t cnt = properties.GetCount();
      if (cnt == MAX_PROPERTIES_COUNT) {
        smsc_log_warn(log, "can't add property \'%s\', profile key=%s already has maximum properties count=%d",
                      prop.getName(), pkey.c_str(), cnt);
        return;
      }
        if(!prop.isExpired())
            properties.Insert(prop.getName(), new Property(prop));
    }
}*/
/*
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
}*/

