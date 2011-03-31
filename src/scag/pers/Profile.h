/* $Id$ */

#ifndef SCAG_PERS_STORE_PROFILE
#define SCAG_PERS_STORE_PROFILE

#include <list>
#include <string>
#include <vector>

#include <core/buffers/File.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include "logger/Logger.h"

#include "Property.h"
#include "AbntAddr.hpp"

#include "scag/util/io/Serializer.h"
#include "scag/pers/Glossary.h"



namespace scag{ namespace pers{

using namespace std;
using namespace smsc::core::buffers;
using smsc::logger::Logger;
using std::vector;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;
using scag::util::storage::Serializable;
using scag::util::io::GlossaryBase;
using smsc::core::buffers::File;

typedef Hash<Property*> PropertyHash;

extern const uint8_t PROPERTIES_COUNT_SIZE;
extern const uint16_t MAX_PROPERTIES_COUNT; 

class IntProfileKey
{
    uint32_t key;

  public:

    IntProfileKey(){};
    IntProfileKey(const IntProfileKey& k):key(k.key) {};
    IntProfileKey(const uint32_t k):key(k) {};

    bool operator==(const IntProfileKey& that)const { return key==that.key; }

    std::string toString() const
    {
      char buf[16];
      sprintf(buf,"%hu", key);
      std::string str = buf;
      return str;
    }

    static uint32_t Size() { return sizeof(uint32_t); }

    void Read(File& f) { key = f.ReadNetInt32(); }

    void Write(File& f)const { f.WriteNetInt32(key); }

    uint32_t HashCode(uint32_t attempt)const
    {
        uint32_t res = crc32(0, &key, sizeof(key));
        for(; attempt > 0; attempt--) res = crc32(res, &key, sizeof(key));
        return res;
    }
    const uint32_t* getKey() const {
      return &key;
    }

    void reset() {
      key = 0;
    }

    static uint32_t CalcHash(IntProfileKey pk) {
      return  crc32(0, pk.getKey(), IntProfileKey::Size());
    }
};

enum ProfileState{
    OK,
    LOCKED,
    DELETED
};

class Profile : public Serializable
{

    PropertyHash properties;
    smsc::logger::Logger* log;
    ProfileState state;
    std::string pkey;
    bool changed;

    vector<long> backup;
    SerialBuffer dataCopy;
public:
    Profile( smsc::logger::Logger* logger = NULL ):log(logger), state(OK), changed(false) {};
    Profile(const std::string& _pkey, smsc::logger::Logger* _log = NULL) : log(_log), state(OK), pkey(_pkey) {};
    Profile(const AbntAddr& address, smsc::logger::Logger* _log = NULL) : log(_log), state(OK), pkey(address.toString()) {};
    Profile(const IntProfileKey& intKey, smsc::logger::Logger* _log = NULL) : log(_log), state(OK), pkey(intKey.toString()) {};
    ~Profile();
    Profile& operator=(const Profile& pf);

    Property* GetProperty(const char* name);
    bool PropertyExists(const char* str);
    bool AddProperty(Property& prop);
    bool DeleteProperty(const char* str);
    void DeleteExpired();
    void Empty();
    uint32_t GetCount() { return properties.GetCount(); };
    const PropertyHash& getProperties() const { return properties; }
    const string& getKey() const { return pkey; }
    void setKey(const string& key) { pkey = key; }
    const smsc::logger::Logger* getLog() const { return log; }
    void setLog(Logger* dblog) { log = dblog; }

    void Serialize(SerialBuffer& buf, bool toFSDB = false, GlossaryBase* glossary = NULL) const;
    void Deserialize(SerialBuffer& buf, bool fromFSDB = false, GlossaryBase* glossary = NULL);
    ProfileState getState() const { return state; };
    void setLocked() { state = LOCKED; };
    void setDeleted() { Empty(); state = DELETED; };
    void setOk() { state = OK; };
    void addNewProperty(Property& prop);
    void copyPropertiesTo(Profile* pf) const;

    void addDataToBackup(long nextBlock);
    void clearBackup();
    const vector<long>& getBackup() const;
    void setBackup(const vector<long>& _backup);
    int getBackupDataSize() const; 
    const char* getBackupData() const;
    void setBackupData(const SerialBuffer& data);
    void restoreBackup(const vector<long>& _backup, int backupSize);

    bool isChanged() const { return changed; };
    void setChanged(bool change) { changed = change; };

};

}}
#endif

