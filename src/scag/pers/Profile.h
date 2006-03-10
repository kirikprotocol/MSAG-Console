#ifndef SCAG_PERS_STORE_PROFILE
#define SCAG_PERS_STORE_PROFILE

#include <list>
#include <string>

#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>

#include "VarRecSizeStore.h"

#include "Property.h"

namespace scag{ namespace pers{

using namespace std;
using namespace smsc::core::buffers;

/*struct ProfileKey
{
	smsc::sms::Address  abonentAddr;

	bool operator ==(const ProfileKey& sk) const
	{
		return this->abonentAddr == sk.abonentAddr;
	}
};*/


typedef Hash<Property*> PropertyHash;

class Profile : public Serializable
{
	PropertyHash properties;
	smsc::logger::Logger* log;

public:
	~Profile();
	Profile() {log = smsc::logger::Logger::getInstance("scag.pers.profile");};

	Property* GetProperty(const char* name);
	bool PropertyExists(const char* str);
	void AddProperty(Property* prop);
	void DeleteProperty(const char* str);
	void DeleteExpired();
	void Empty();
	uint32_t GetCount() { return properties.GetCount(); };

	void Serialize(SerialBuffer& buf);
	void Deserialize(SerialBuffer& buf);
};

}}

#endif
