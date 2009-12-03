/* $Id$ */

#include <unistd.h>
#include "Profile.h"

namespace {
smsc::core::synchronization::Mutex logamutex;
}


namespace scag2 {
namespace pvss {

const uint8_t PROPERTIES_COUNT_SIZE = 14; //14 bits for profile properties count
const uint16_t MAX_PROPERTIES_COUNT = 16383; 

smsc::logger::Logger* Profile::loga_ = 0;

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



void Profile::initLog()
{
    if (!loga_) {
        MutexGuard mg(logamutex);
        if (!loga_) loga_ = smsc::logger::Logger::getInstance("prof.alloc");
    }
    smsc_log_debug(loga_,"ctor %p key=%s",this,pkey.c_str());
}


Profile::Profile() :
log(0), state(OK), changed(false)
{
    initLog();
}
    

Profile::Profile(const std::string& _pkey, smsc::logger::Logger* _log ) :
log(_log), state(OK), pkey(_pkey), changed(false)
{
    initLog();
}


Profile::Profile(const AbntAddr& address, smsc::logger::Logger* _log ) :
log(_log), state(OK), pkey(address.toString()), changed(false)
{
    initLog();
}


Profile::Profile(const IntProfileKey& intKey, smsc::logger::Logger* _log ) :
log(_log), state(OK), pkey(intKey.toString()), changed(false)
{
    initLog();
}


Profile::~Profile()
{
    char *key = 0;
    Property *prop;

    PropertyHash::Iterator it = properties.getIterator();

    while(it.Next(key, prop))
        if(prop) delete prop;
    smsc_log_debug(loga_,"dtor: %p key=%s",this,pkey.c_str());
}


Profile& Profile::operator=(const Profile& pf) {
  if (this == &pf) {
    return *this;
  }
  smsc_log_debug(loga_,"op=: %p key=%s <== %p key=%s",this,pkey.c_str(),&pf,pf.pkey.c_str());
  const smsc::logger::Logger* _log = pf.getLog();
  if (_log) {
    log = smsc::logger::Logger::getInstance(_log->getName());
  }
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


void Profile::setKey( const std::string& k )
{
    smsc_log_debug(loga_,"setk %p key=%s newkey=%s",this,pkey.c_str(),k.c_str());
    pkey = k;
}


bool Profile::AddProperty(const Property& prop)
{
  uint16_t cnt = properties.GetCount();
  if (cnt == MAX_PROPERTIES_COUNT) {
    Logger* wlog = log ? log : Logger::getInstance("pvss.profile");
    smsc_log_warn(wlog, "can't add property \'%s\', profile key=%s overflow properties count=%d",
                  prop.getName(), pkey.c_str(), cnt);
    return false;
  }
  properties.Insert(prop.getName(), new Property(prop));
  return true;
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


std::string Profile::toString() const
{
    std::string res;
    res.reserve(400);
    res.append( "profile \"" );
    res.append( pkey );
    res.append( "\" sz=" );
    char buf[10];
    snprintf(buf,sizeof(buf),"%u {",unsigned(properties.GetCount()));
    res.append(buf);
    char *key;
    Property* prop;
    bool comma = false;
    for ( PropertyHash::Iterator i = properties.getIterator(); i.Next(key,prop); ) {
        if ( comma ) {
            res.append(", ");
        } else {
            comma = true;
        }
        if (prop) {
            res.append( prop->toString() );
        } else {
            res.append( "0x0" );
        }
    }
    res.append("}");
    return res;
}

}//pvss
}//scag2

