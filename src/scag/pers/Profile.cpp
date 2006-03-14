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
	buf.Append((char*)&cnt, sizeof(uint16_t));

	while(it.Next(key, prop))
		prop->Serialize(buf);
}

void Profile::Deserialize(SerialBuffer& buf)
{
	const char *key;
	uint16_t cnt;
	Property* prop;

	Empty();
	buf.Read((char*)&cnt, sizeof(uint16_t));
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

bool Profile::GetProperty(const char* str, Property& prop)
{
	try{
		Property *p = properties.Get(str);
		prop = *p;
		return true;
	} 
	catch(HashInvalidKeyException &k)
	{
		return false;
	}
}

void Profile::DeleteProperty(const char* str)
{
	Property *prop;

	if(properties.Exists(prop->getName().c_str()))
	{
		prop = properties.Get(prop->getName().c_str());
		delete prop;
		properties.Delete(prop->getName().c_str());
	}
}

void Profile::DeleteExpired()
{
	char *key = 0;
	Property* prop;

	PropertyHash::Iterator it = properties.getIterator();

	while(it.Next(key, prop))
		if(prop->isExpired())
		{
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
	if(properties.Exists(prop.getName().c_str()))
	{
		Property* p = properties.Get(prop.getName().c_str());
		delete p;
		properties.Delete(prop.getName().c_str());
	}

	if(!prop.isExpired())
		properties.Insert(prop.getName().c_str(), new Property(prop));
}

}}
