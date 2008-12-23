/* $Id$ */

#include <unistd.h>
#include "Profile.h"

namespace scag2 { namespace pvss {

const uint8_t PROPERTIES_COUNT_SIZE = 14; //14 bits for profile properties count
const uint16_t MAX_PROPERTIES_COUNT = 16383; 

void Profile::Serialize(SerialBuffer& buf, bool toFSDB, GlossaryBase* glossary) const
{
  //TODO: add glossary check
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
    //TODO: add glossary check
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

void Profile::deserialize(const char* data, uint32_t dataSize, GlossaryBase* glossary) {
  if (!glossary) {
    return;
  }
  if (!data || !dataSize) {
    Empty();
    return;
  }
  SerialBuffer sb(dataSize);
  sb.Append(data, dataSize);
  sb.SetPos(0);
  Deserialize(sb, true, glossary);
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

}//pvss
}//scag2

