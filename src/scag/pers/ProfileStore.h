/* $Id$ */

#ifndef SCAG_PERS_PROFILE_STORE
#define SCAG_PERS_PROFILE_STORE

#include <inttypes.h>
#include <string>
#include <stack>
#include <vector>

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/PageFile.hpp"

#include "VarRecSizeStore.h"
#include "Profile.h"
#include "Property.h"
#include "AbntAddr.hpp"
#include "FSDB.h"
#include "scag/exc/SCAGExceptions.h"
#include "Types.h"

namespace scag { namespace pers {
using std::string;
using std::stack;
using std::vector;

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
};

class StringProfileKey
{
    smsc::logger::Logger* log;
#define MAX_STRING_KEY_LEN  30
    char key[MAX_STRING_KEY_LEN];
    uint8_t key_len;

  public:

    StringProfileKey() : key_len(0){};
    void copy(const char*k) { strncpy(key, k, MAX_STRING_KEY_LEN); key[MAX_STRING_KEY_LEN - 1] = 0; key_len = strlen(key); };
        
    StringProfileKey(const StringProfileKey& k) { copy(k.key); };
    StringProfileKey(const char* k) { copy(k); };
    StringProfileKey(const std::string& k) { copy(k.c_str()); };

    bool operator==(const StringProfileKey& that)const { return !strcmp(key, that.key); }

    std::string toString() const
    {
        std::string str(key);
        return str;
    }

    static uint32_t Size() { return MAX_STRING_KEY_LEN; }

    void Read(File& f) { f.Read(key, MAX_STRING_KEY_LEN); key_len = strlen(key); }

    void Write(File& f)const { f.Write(key, MAX_STRING_KEY_LEN); }

    uint32_t HashCode(uint32_t attempt)const
    {
        uint32_t res = crc32(0, key, key_len);
        for(; attempt > 0; attempt--) res = crc32(res, key, key_len);
        return res;
    }
};

template <class Key>
class CacheItem
{
public:
    Key key;
    Profile *pf;

    CacheItem(const Key& k, Profile *p) { key = k; pf = p; };
    ~CacheItem() {
      if (pf) {
        delete pf; 
      }
    }
};

template <class Key>
class HashProfileStore : public CachedProfileStore<Key>
{
public:
    HashProfileStore():log(smsc::logger::Logger::getInstance("hashstore")) { };
    ~HashProfileStore() { 
        smsc_log_debug(log, "Shutdown store %s", storeName.c_str());
    };

    void init(const std::string& _storeName, uint32_t initRecCnt, uint32_t cacheSize, smsc::logger::Logger *_log)
    {
		CachedProfileStore::init(cacheSize);
		
        dblog = _log;

		storeName = _storeName;
        store.init(storeName, initRecCnt);
        smsc_log_debug(log, "Inited: %s", storeName.c_str());
    };

    void storeProfile(const Key& key, Profile* pf)
    {
//        pf->DeleteExpired();

        if(pf->GetCount() > 0)
        {
            if(!store.updateRecord(key, pf))
            {
                store.newRecord(key, pf);
                smsc_log_debug(log, "Profile %s created.", key.toString().c_str());
            }
        }
        else
            store.deleteRecord(key);
    }

    Profile* _getProfile(const Key& key, bool create)
    {
        Profile *pf = new Profile(key.toString(), dblog);
        if(store.getRecord(key, pf) || create)
            return pf;
        delete pf;
        return NULL;
    };

    //void Reset() {}

    //bool Next(Key& key, uint8_t& profile_state) {
      //return false;
    //}

protected:
    Profile* _createProfile(const Key &key) {
      return new Profile(key.toString(), dblog);
    }

    void _deleteProfile(const Key &key) {
      store.deleteRecord(key);
    }

/*    bool getProfile(Key& key, Profile& pf, bool create)
    {
        if(store.getRecord(key, &pf))
            return true;
        if(create)
        {
            pf.Empty();
            return true;
        }
            
        return false;
    };*/

protected:
    smsc::logger::Logger* log;
    smsc::logger::Logger *dblog;    
	std::string storeName;
    VarRecSizeStore<Key> store;
};


template <class Key>
class TreeProfileStore : public CachedProfileStore<Key>
{
public:
    TreeProfileStore() { };
    ~TreeProfileStore() { smsc_log_debug(log, "Shutdown store %s", storeName.c_str()); };

    void init(	const string& storageName, const string& storagePath,
				int indexGrowth, int blocksInFile, int dataBlockSize, int cacheSize,  smsc::logger::Logger *_log)
    {
		CachedProfileStore::init(cacheSize);
		
        log = smsc::logger::Logger::getInstance("treestore");;
        dblog = _log;
        
		storeName = storageName;
        if(store.Init(storageName, storagePath, indexGrowth, blocksInFile, dataBlockSize) != 0)
            throw Exception("Error init abonentstore");
//		smsc_log_info(log, "Inited: cacheSize = %d", cacheSize);
	};

    void storeProfile(const Key& key, Profile *pf)
    {
        //pf->DeleteExpired();
		sb.Empty();
        //pf->Serialize(sb, true);
//        delete pf;  
        store.Set(key, *pf);
        //if (store.Set(key, sb)) {
          //smsc_log_debug(log, "Set return TRUE");
        //} else {
          //smsc_log_debug(log, "Set return FALSE");
        //}
    }

    Profile* _getProfile(const Key& key, bool create)
    {
      Profile *pf = new Profile(key.toString(), dblog);
      try {
        sb.Empty();
        if(store.Get(key, *pf))
        {
            //pf->Deserialize(sb, true);
            return pf;
        }
        if(create)
            return pf;
        delete pf;
        return NULL;
      } catch (...) {
        delete pf;
        throw;
      }
    };

    void Reset() {
      store.resetStorage();
    }

    bool Next(Key& key, uint16_t& state_cnt) {
      sb.Empty();
      if (store.dataStorageNext(key, sb)) {
        state_cnt = sb.ReadInt16();
        return true;
      }
      return false;
    }

protected:
    Profile* _createProfile(const Key& key) {
      return new Profile(key.toString(), dblog);
    }

    void _deleteProfile(const Key &key) {
      //store.Remove(key);
    }

protected:
	std::string storeName;
    smsc::logger::Logger* log;
    smsc::logger::Logger *dblog;    
    FSDBProfiles<Key> store;
	SerialBuffer sb;
};

template <class Key>
class CachedProfileStore
{
public:
    CachedProfileStore(): cache(NULL) {};

    virtual ~CachedProfileStore()
    {
        if(cache != NULL)
        {
            for(int i = 0; i < max_cache_size; i++)
                if(cache[i] != NULL)
                {
//                    _storeProfile(cache[i]->key, cache[i]->pf);
//                    delete cache[i]->pf;
                    delete cache[i];
                }
            delete[] cache;
        }
//		smsc_log_debug(log, "Shutdown cached store %s....", storeName.c_str());		
    };
	
    void init(uint32_t _max_cache_size = 1000)
    {
        max_cache_size = _max_cache_size;
        cache = new CacheItem<Key>*[_max_cache_size];
        memset(cache, 0, sizeof(CacheItem<Key>*) * _max_cache_size);
        cache_log = smsc::logger::Logger::getInstance("cachestore");
    };

    Profile* getProfile(const Key& key, bool create)
    {
        uint32_t i = key.HashCode(0) % max_cache_size;
        if(cache[i] != NULL)
        {
            if(cache[i]->key == key) {
              return cache[i]->pf;
            }

            Profile* pf = _getProfile(key, create);
            if(pf)
            {
              if (cache[i]->pf) {
                delete cache[i]->pf;
              } else {
                smsc_log_warn(cache_log, "getProfile: cache[%d] profile already deleted, key=%s ",
                               i, cache[i]->key.toString().c_str());
              }
                cache[i]->key = key;
                cache[i]->pf = pf;
                return pf;
            }
            return NULL;
        }
        else
        {
            Profile* pf = _getProfile(key, create);
            if(pf)
                cache[i] = new CacheItem<Key>(key, pf);
            return pf;
        }
            
    };

    Profile* createCachedProfile(const Key& key) {
      uint32_t i = key.HashCode(0) % max_cache_size;
      Profile* pf = _createProfile(key);
      if (!pf) {
        return NULL;
      }
      if(cache[i] != NULL) {
        if (cache[i]->pf) {
          delete cache[i]->pf;
        } else {
          smsc_log_warn(cache_log, "createCachedProfile: cache[%d] profile already deleted, key=%s ",
                         i, cache[i]->key.toString().c_str());
        }
        cache[i]->key = key;
        cache[i]->pf = pf;
      } else {
        cache[i] = new CacheItem<Key>(key, pf);
      }
      return pf;
    }

    void deleteCachedProfile(const Key& key) {
      uint32_t i = key.HashCode(0) % max_cache_size;
      if(cache[i] != NULL && cache[i]->key == key) {
        if (!cache[i]->pf) {
          smsc_log_warn(cache_log, "deleteCachedProfile: cache[%d] profile already deleted, key=%s",
                         i, cache[i]->key.toString().c_str());
        }
          delete cache[i];
          cache[i] = NULL;
      }
      _deleteProfile(key);
    }

	virtual Profile* _getProfile(const Key& key, bool create) = 0;

protected:
    virtual Profile* _createProfile(const Key& key) = 0;
    virtual void _deleteProfile(const Key& key) = 0;
    //virtual void Reset() = 0;
    //virtual bool Next(Key& key, uint8_t& profile_state) = 0;
	
protected:
    uint32_t max_cache_size;
    CacheItem<Key> **cache;
    smsc::logger::Logger* cache_log;
};

enum PropertyCmd {
  PROP_ADD,
  PROP_SET,
  PROP_DEL
};

template <class Key, class RKey, class StorageType>
class ProfileStore : public StorageType
{
public:

  ProfilesStore():needBackup(false) {}; 

  void deleteProfile(const Key& key) {
    //MutexGuard mg(mtx);
    deleteCachedProfile(key);
  }

  Profile* createProfile(const Key &key) {
    //MutexGuard mg(mtx);
    return createCachedProfile(key);
  }

  void rollBack() {
    while (!backup.empty()) {
      Profile *pf = getProfile(backup.back().key, false);
      if (!pf) {
        continue;
      }
      backup.back().rollBack(pf, dblog);
      storeProfile(backup.back().key, pf);
      backup.pop_back();
    }
    needBackup = false;
  }

  void resetBackup() {
    backup.clear();
    needBackup = false;
  }

  void setNeedBackup(bool _needBackup) {
    needBackup = _needBackup;
  }


  bool setProperty(RKey rkey, Property& prop)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      if(prop.isExpired()) {
        smsc_log_info(dblog, "E key=\"%s\" property=%s", key.toString().c_str(), prop.toString().c_str());
        return true;
      }
      Profile* pf = getProfile(key, true);
      //TODO if (!pf) ?
      return _setProperty(pf, key, prop);
  };

  bool setProperty(Profile* pf, RKey rkey, Property& prop)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      if(prop.isExpired()) {
        smsc_log_info(dblog, "E key=\"%s\" property=%s", key.toString().c_str(), prop.toString().c_str());
        return false;
      }
      return _setProperty(pf, key, prop);
  };

  bool delProperty(RKey rkey, const char* nm)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      Profile *pf = getProfile(key, false);
      return _delProperty(pf, key, nm);
  };

  bool delProperty(Profile* pf, RKey rkey, const char* nm)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      return _delProperty(pf, key, nm);
  };

  bool getProperty(RKey rkey, const char* nm, Property& prop)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      Profile* pf = getProfile(key, false);
      return _getProperty(pf,key,nm,prop);
  };

  bool getProperty(Profile* pf, RKey rkey, const char* nm, Property& prop)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      return _getProperty(pf,key,nm,prop);
  };

  PersServerResponseType incProperty(RKey rkey, Property& prop, int& result)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      Profile* pf = getProfile(key, true);
      return _incModProperty(pf, key, prop, 0, result);
  };

  PersServerResponseType incProperty(Profile* pf, RKey rkey, Property& prop, int& result)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      return _incModProperty(pf, key, prop, 0, result);
  };

  PersServerResponseType incModProperty(RKey rkey, Property& prop, uint32_t mod, int& result)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      Profile* pf = getProfile(key, true);
      return _incModProperty(pf, key, prop, mod, result);
  };

  PersServerResponseType incModProperty(Profile *pf, RKey rkey, Property& prop, uint32_t mod, int& result)
  {
      //MutexGuard mt(mtx);
      Key key(rkey);
      return _incModProperty(pf, key, prop, mod, result);
  };

private:
  struct BackupProperty {
    Key key;
    Property value;
    PropertyCmd cmd_id;

    BackupProperty(const Key& _key, const Property& prop, PropertyCmd _cmd_id):
                   key(_key), value(prop), cmd_id(_cmd_id) {}

    BackupProperty(const BackupProperty& backupProp):key(backupProp.key), value(backupProp.value),
                                                     cmd_id(backupProp.cmd_id) {}

    void rollBack(Profile *pf, smsc::logger::Logger *dblog) {
      if (!pf) {
        return;
      }
      switch (cmd_id) {
      case PROP_SET: {
        //pf->AddProperty(value);
        smsc_log_debug(dblog, "rollback Set property command");
        Property *p = pf->GetProperty(value.getName());
        char action = 0;
        if (p) {
          *p = value;
          action = 'U';
        } else {
          pf->AddProperty(value);
          action = 'A';
        }
        smsc_log_info(dblog, "%c key=\"%s\" property=%s", action, key.toString().c_str(), value.toString().c_str());
        break;
      }
      case PROP_DEL: 
        pf->AddProperty(value);
        smsc_log_debug(dblog, "rollback Del property command");
        smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), value.toString().c_str());
        break;
      case PROP_ADD: 
        pf->DeleteProperty(value.getName()); 
        smsc_log_debug(dblog, "rollback Add property command");
        smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), value.getName());
        break;
      }
    }
    private:
    BackupProperty() {};
  };
  
  bool _setProperty(Profile* pf, const Key& key, Property& prop) {
    if (!pf) {
      //TODO require?
      smsc_log_error(log, "profile key=%s is NULL", key.toString().c_str());
      return false;
    }
    Property* p = pf->GetProperty(prop.getName());
    if(p != NULL)
    {
      if (needBackup) {
        backup.push_back(BackupProperty(key, *p, PROP_SET));
      }
      p->setValue(prop);
      p->WriteAccess();
    }
    else {
      if (needBackup) {
        backup.push_back(BackupProperty(key, prop, PROP_ADD));
      }
      if (!pf->AddProperty(prop)) {
        if (needBackup) {
          backup.pop_back();
        }
        return false;
      }
    }
    storeProfile(key, pf);
    smsc_log_info(dblog, "%c key=\"%s\" property=%s", p ? 'U' : 'A', key.toString().c_str(), p ? p->toString().c_str() : prop.toString().c_str());
    return true;
  }

  bool _delProperty(Profile *pf, const Key& key, const char* nm) {
    if (!pf) {
      //smsc_log_error(this->log, "profile key=%s is NULL", key.toString().c_str());
      smsc_log_debug(log, "profile key=%s not found", key.toString().c_str());
      return false;
    }
    if (needBackup) {
      Property *p = pf->GetProperty(nm);
      if (p) {
        backup.push_back(BackupProperty(key, *p, PROP_DEL));
      }
    }
    if (!pf->DeleteProperty(nm)) {
      smsc_log_debug(log, "profile %s, property '%s' not found", key.toString().c_str(), nm);
      return false;
    }
    storeProfile(key, pf);
    smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), nm);
    return true;
  }

  bool _getProperty(Profile *pf, const Key& key, const char* nm, Property& prop) {
    if (!pf) {
      //smsc_log_debug(this->log, "profile key=%s is NULL", key.toString().c_str());
      smsc_log_debug(log, "profile key=%s not found", key.toString().c_str());
      return false;
    }
    Property* p = pf->GetProperty(nm);
    if (!p) {
      smsc_log_debug(log, "profile %s, property '%s' not found", key.toString().c_str(), nm);
      return false;
    }
    if(p->getTimePolicy() == R_ACCESS)
    {
        p->ReadAccess();
        storeProfile(key, pf);
    }
    prop = *p;
    smsc_log_debug(log, "profile %s, getProperty=%s", key.toString().c_str(), prop.toString().c_str());
    return true;
  }

  
  PersServerResponseType _incModProperty(Profile *pf, Key key, Property& prop, uint32_t mod, int& result) {
    if (!pf) {
      smsc_log_error(log, "profile key=%s is NULL", key.toString().c_str());
      return RESPONSE_ERROR;
    }
    Property* p = pf->GetProperty(prop.getName());

    if (!p) {
      if (needBackup) {
        backup.push_back(BackupProperty(key, prop, PROP_ADD));
      }
      result = prop.getIntValue();
      if(mod) result %= mod;
      prop.setIntValue(result);
      if (!pf->AddProperty(prop)) {
        if (needBackup) {
          backup.pop_back();
        }
        return RESPONSE_ERROR;
      }
      storeProfile(key, pf);
      smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), prop.toString().c_str());            
      return RESPONSE_OK;
    }

    if (needBackup) {
      backup.push_back(BackupProperty(key, *p, PROP_SET));
    }
    if(p->getType() == INT && prop.getType() == INT)
    {
        result = p->getIntValue() + prop.getIntValue();
        if(mod) result %= mod;
        p->setIntValue(result);
        p->WriteAccess();
        storeProfile(key, pf);
        smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
        return RESPONSE_OK;
    }
    /*
    if(p->getType() == DATE && prop.getType() == DATE)
    {
        p->setDateValue(p->getDateValue() + prop.getDateValue());
        //p->setDateValue(result.getDateValue());
        p->WriteAccess();
        storeProfile(key, pf);
        smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
        return true;
    }*/
    if (p->convertToInt() && prop.convertToInt())
    {
        result = p->getIntValue() + prop.getIntValue();
        if(mod) result %= mod;
        p->setIntValue(result);
        p->WriteAccess();
        storeProfile(key, pf);
        smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), p->getName());                
        smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
        return RESPONSE_OK;
    }
    if (needBackup) {
      backup.pop_back();
    }
    return RESPONSE_TYPE_INCONSISTENCE;
  }
private:
    Mutex mtx;
    bool needBackup;
    vector<BackupProperty> backup;

};

typedef ProfileStore<IntProfileKey, uint32_t, HashProfileStore<IntProfileKey> > IntProfileStore;
//typedef ProfileStore<StringProfileKey, std::string, CachedProfileStore<HashProfileStore<StringProfileKey>, StringProfileKey> > StringProfileStore;

typedef ProfileStore<AbntAddr, const char*, TreeProfileStore<AbntAddr> > StringProfileStore;

}}

#endif // SCAG_PERS_PROFILE_STORE
