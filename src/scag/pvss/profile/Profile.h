/* $Id$ */

#ifndef _SCAG_PVSS_PROFILE_PROFILE_H_
#define _SCAG_PVSS_PROFILE_PROFILE_H_

#include <list>
#include <string>
#include <vector>
#include <memory>

#include "util/crc32.h"
#include "logger/Logger.h"
#include "core/buffers/File.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/util/io/Serializer.h"
#include "scag/util/io/GlossaryBase.h"
#include "scag/util/storage/BlocksHSBackupData.h"

#include "scag/pvss/data/Property.h"
#include "AbntAddr.hpp"

namespace scag2 {
namespace pvss {

using namespace std;
// using namespace smsc::core::buffers;
using smsc::logger::Logger;
using std::vector;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;
using scag::util::storage::Serializable;
using scag::util::io::GlossaryBase;
using scag::util::storage::BlocksHSBackupData;
using smsc::core::buffers::File;

typedef smsc::core::buffers::Hash<Property*> PropertyHash;

extern const uint8_t PROPERTIES_COUNT_SIZE;
extern const uint16_t MAX_PROPERTIES_COUNT; 

class ProfileBackup;

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

    static uint32_t Size() { return static_cast<uint32_t>(sizeof(uint32_t)); }

    void Read(File& f) { key = f.ReadNetInt32(); }

    void Write(File& f)const { f.WriteNetInt32(key); }

    uint32_t HashCode(uint32_t attempt)const
    {
        uint32_t res = smsc::util::crc32(0, &key, sizeof(key));
        for(; attempt > 0; attempt--) res = smsc::util::crc32(res, &key, sizeof(key));
        return res;
    }
    const uint32_t* getKey() const {
      return &key;
    }

    static uint32_t CalcHash(IntProfileKey pk) {
        return smsc::util::crc32(0, pk.getKey(), IntProfileKey::Size());
    }
};

/*
enum ProfileState {
    OK,
    LOCKED,
    DELETED
};
 */

class Profile : public Serializable
{
private:
    static smsc::logger::Logger* loga_;
    void initLog();

public:
    Profile( ProfileBackup* backup = 0 );
    Profile(const std::string& _pkey, ProfileBackup* backup = 0); // : log(_log), state(OK), pkey(_pkey) {};
    Profile(const AbntAddr& address, ProfileBackup* backup = 0); // : log(_log), state(OK), pkey(address.toString()) {};
    Profile(const IntProfileKey& intKey, ProfileBackup* backup = 0); // : log(_log), state(OK), pkey(intKey.toString()) {};
    virtual ~Profile();

    Property* GetProperty(const char* name);
    bool PropertyExists(const char* str);
    bool AddProperty(const Property& prop);
    bool DeleteProperty(const char* str, std::auto_ptr<Property>* holder = 0);
    void DeleteExpired();
    void Empty();
    uint32_t GetCount() const { return properties.GetCount(); };
    const PropertyHash& getProperties() const { return properties; }
    const string& getKey() const { return pkey; }
    void setKey(const string& key); // { pkey = key; }
    // smsc::logger::Logger* getLog() const { return log; }
    // void setLog(Logger* dblog) { log = dblog; }
    // void setBackup( ProfileBackup& backup ) { backup_ = &backup; }

    void Serialize(SerialBuffer& buf, bool toFSDB = false, GlossaryBase* glossary = NULL) const;
    void Deserialize(SerialBuffer& buf, bool fromFSDB = false, GlossaryBase* glossary = NULL);
    // ProfileState getState() const { return state; };
    // void setLocked() { state = LOCKED; };
    // void setDeleted() { Empty(); state = DELETED; };
    // void setOk() { state = OK; };
    void addNewProperty(Property& prop);
    void copyPropertiesTo(Profile* pf) const;

    bool isChanged() const { return changed; };
    void setChanged(bool change) { changed = change; };
    void deserialize(const char* data, uint32_t dataSize, GlossaryBase* glossary = NULL);

    /// for recovery
    void deserialize(BlocksHSBackupData& backup,GlossaryBase* glossary=0) {
        deserialize(backup.getBackupData(),backup.getBackupDataSize(),glossary);
    }

    std::string toString() const;

private:
    Profile( const Profile& pf );
    Profile& operator=(const Profile& pf);

private:
  PropertyHash properties;
    // smsc::logger::Logger* log;
  ProfileBackup* backup_;
  // ProfileState state;
  std::string pkey;
  bool changed;
};


class LockableProfile : public Profile
{
public:
    inline LockableProfile( ProfileBackup* backup = 0 ) : Profile(backup) {}
    inline LockableProfile( const std::string& pkey, ProfileBackup* backup = 0 ) : Profile(pkey,backup) {}
    inline LockableProfile( const AbntAddr& addr, ProfileBackup* backup = 0 ) : Profile(addr,backup) {}
    inline LockableProfile( const IntProfileKey& intKey, ProfileBackup* backup = 0) : Profile(intKey,backup) {}

    inline void Lock() { lock_.Lock(); }
    inline void Unlock() { lock_.Unlock(); }
private:
    smsc::core::synchronization::Mutex lock_;
};


}//pvss
}//scag2

inline scag::util::io::Serializer& operator << (scag::util::io::Serializer& ser,
                                                const scag2::pvss::Profile& pf) { 
  scag::util::storage::SerialBuffer buf;
  pf.Serialize(buf, true, ser.getGlossary());
  ser.write(buf.GetSize(), buf.c_ptr());
  return ser; 
};

inline scag::util::io::Deserializer& operator >> (scag::util::io::Deserializer& deser,
                                                 scag2::pvss::Profile& pf) { 
  uint32_t size = 0;
  const char* buf = deser.read(size);
  scag::util::storage::SerialBuffer sb(size);
  sb.Append(buf, size);
  sb.SetPos(0);
  pf.Deserialize(sb, true, deser.getGlossary());
  return deser;
};

#endif

