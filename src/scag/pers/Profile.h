/* $Id$ */

#ifndef SCAG_PERS_STORE_PROFILE
#define SCAG_PERS_STORE_PROFILE

#include <list>
#include <string>

#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include "logger/Logger.h"

#include "Property.h"

namespace scag{ namespace pers{

using namespace std;
using namespace smsc::core::buffers;

typedef Hash<Property*> PropertyHash;

enum ProfileState{
    OK = 1,
    LOCKED,
    DELETED
};

class Profile : public Serializable
{
    PropertyHash properties;
    smsc::logger::Logger* log;
    ProfileState state;
    std::string pkey;
public:
    ~Profile();
    Profile(const std::string& _pkey, smsc::logger::Logger* _log = NULL) : state(OK), pkey(_pkey), log(_log) {};
    Profile& operator=(const Profile& pf);

    Property* GetProperty(const char* name);
    bool PropertyExists(const char* str);
    void AddProperty(Property& prop);
    bool DeleteProperty(const char* str);
    void DeleteExpired();
    void Empty();
    uint32_t GetCount() { return properties.GetCount(); };
    const PropertyHash& getProperties() const { return properties; }
    const string getKey() const { return pkey; }
    const smsc::logger::Logger* getLog() const { return log; }

    void Serialize(SerialBuffer& buf, bool toFSDB = false);
    void Deserialize(SerialBuffer& buf, bool fromFSDB = false);
    ProfileState getState() const { return state; };
    void setLocked() { state = LOCKED; };
    void setDeleted() { state = DELETED; };
    void setOk() { state = OK; };
    void addNewProperty(Property& prop);
    void copyPropertiesTo(Profile* pf) const;
};

}}

#endif

