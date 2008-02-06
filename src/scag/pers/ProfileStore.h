/* $Id$ */

#ifndef SCAG_PERS_PROFILE_STORE
#define SCAG_PERS_PROFILE_STORE

#include <inttypes.h>
#include <string>

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/PageFile.hpp"

#include "VarRecSizeStore.h"
#include "Profile.h"
#include "Property.h"
#include "mcisme/AbntAddr.hpp"
#include "FSDB.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag{ namespace pers{
using smsc::mcisme::AbntAddr;
using std::string;

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
    HashProfileStore() { };
    ~HashProfileStore() { smsc_log_debug(log, "Shutdown store %s", storeName.c_str());};

    void init(const std::string& _storeName, uint32_t initRecCnt, uint32_t cacheSize, smsc::logger::Logger *_log)
    {
		CachedProfileStore::init(cacheSize);
		
        log = smsc::logger::Logger::getInstance("hashstore");
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
        if(store.Init(storageName, storagePath, indexGrowth, blocksInFile, dataBlockSize))
            throw Exception("Error init abonentstore");
//		smsc_log_info(log, "Inited: cacheSize = %d", cacheSize);
	};

    void storeProfile(const Key& key, Profile *pf)
    {
//        pf->DeleteExpired();
		sb.Empty();
        pf->Serialize(sb, true);
//        delete pf;  
        store.Set(key, sb);
        //if (store.Set(key, sb)) {
          //smsc_log_debug(log, "Set return TRUE");
        //} else {
          //smsc_log_debug(log, "Set return FALSE");
        //}
    }

    Profile* _getProfile(const Key& key, bool create)
    {
        Profile *pf = new Profile(key.toString(), dblog);
		sb.Empty();
        if(store.Get(key, sb))
        {
            pf->Deserialize(sb, true);
            return pf;
        }
        if(create)
            return pf;
        delete pf;
        return NULL;
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
      store.Remove(key);
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

template <class Key, class RKey, class StorageType>
class ProfileStore : public StorageType
{
    Mutex mtx;
public:

  void deleteProfile(const Key& key) {
    MutexGuard mg(mtx);
    deleteCachedProfile(key);
  }

  Profile* createProfile(const Key &key) {
    MutexGuard mg(mtx);
    return createCachedProfile(key);
  }

    void setProperty(RKey rkey, Property& prop)
    {
        MutexGuard mt(mtx);
        Key key(rkey);

        if(prop.isExpired())
            return;

        Profile* pf = getProfile(key, true);
        Property* p = pf->GetProperty(prop.getName().c_str());
        if(p != NULL)
        {
            p->setValue(prop);
            p->WriteAccess();
        }
        else
            pf->AddProperty(prop);
        storeProfile(key, pf);
        smsc_log_info(dblog, "%c key=\"%s\" property=%s", p ? 'U' : 'A', key.toString().c_str(), p ? p->toString().c_str() : prop.toString().c_str());
    };

    void setProperty(Profile* pf, RKey rkey, Property& prop)
    {
        MutexGuard mt(mtx);
        Key key(rkey);

        if(prop.isExpired())
            return;
        if (!pf) {
          return;
        }

        //Profile* pf = getProfile(key, true);
        Property* p = pf->GetProperty(prop.getName().c_str());
        if(p != NULL)
        {
            p->setValue(prop);
            p->WriteAccess();
        }
        else
            pf->AddProperty(prop);
        storeProfile(key, pf);
        smsc_log_info(dblog, "%c key=\"%s\" property=%s", p ? 'U' : 'A', key.toString().c_str(), p ? p->toString().c_str() : prop.toString().c_str());
    };

    bool delProperty(RKey rkey, const char* nm)
    {
        MutexGuard mt(mtx);
        Key key(rkey);
        Profile *pf = getProfile(key, false);

        bool res = false;

        if(pf != NULL)
        {
            res = pf->DeleteProperty(nm);
            storeProfile(key, pf);
            smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), nm);
        }
        return res;
    };

    bool delProperty(Profile* pf, RKey rkey, const char* nm)
    {
        MutexGuard mt(mtx);
        Key key(rkey);
        //Profile *pf = getProfile(key, false);

        bool res = false;

        if(pf != NULL)
        {
            res = pf->DeleteProperty(nm);
            storeProfile(key, pf);
            smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), nm);
        }
        return res;
    };

    bool getProperty(RKey rkey, const char* nm, Property& prop)
    {
        MutexGuard mt(mtx);
        Key key(rkey);

        Profile* pf = getProfile(key, false);

//        smsc_log_debug(dblog, "G key=\"%s\" name=\"%s\"", key.toString().c_str(), nm);
        if(pf != NULL)
        {
            Property* p = pf->GetProperty(nm);

            if(p != NULL)
            {
                if(p->getTimePolicy() == R_ACCESS)
                {
                    p->ReadAccess();
                    storeProfile(key, pf);
                }
                prop = *p;
                smsc_log_debug(log, "profile %s, getProperty=%s", key.toString().c_str(), prop.toString().c_str());
                return true;
            }
        }
        return false;
    };

    bool getProperty(Profile* pf, RKey rkey, const char* nm, Property& prop)
    {
        MutexGuard mt(mtx);
        Key key(rkey);

        //Profile* pf = getProfile(key, false);

//        smsc_log_debug(dblog, "G key=\"%s\" name=\"%s\"", key.toString().c_str(), nm);
        if(pf != NULL)
        {
            Property* p = pf->GetProperty(nm);

            if(p != NULL)
            {
                if(p->getTimePolicy() == R_ACCESS)
                {
                    p->ReadAccess();
                    storeProfile(key, pf);
                }
                prop = *p;
                smsc_log_debug(log, "profile %s, getProperty=%s", key.toString().c_str(), prop.toString().c_str());
                return true;
            }
        }
        return false;
    };

    bool incProperty(RKey rkey, Property& prop)
    {
        MutexGuard mt(mtx);
        Key key(rkey);
        Profile* pf = getProfile(key, true);
        Property* p = pf->GetProperty(prop.getName().c_str());
        if(p != NULL)
        {
            if(p->getType() == INT && prop.getType() == INT)
            {
                p->setIntValue(p->getIntValue() + prop.getIntValue());
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            }
            else if(p->getType() == DATE && prop.getType() == DATE)
            {
                p->setDateValue(p->getDateValue() + prop.getDateValue());
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            }
            if (p->convertToInt() && prop.convertToInt())
            {
                p->setIntValue(p->getIntValue() + prop.getIntValue());
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), p->getName().c_str());                
                smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            }
        }
        else
        {
            pf->AddProperty(prop);
            storeProfile(key, pf);
            smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), prop.toString().c_str());                                    
            return true;
        }
        return false;
    };

    bool incProperty(Profile* pf, RKey rkey, Property& prop)
    {
        MutexGuard mt(mtx);
        Key key(rkey);
        //Profile* pf = getProfile(key, true);
        if (!pf) {
          return false;
        }
        Property* p = pf->GetProperty(prop.getName().c_str());
        if(p != NULL)
        {
            if(p->getType() == INT && prop.getType() == INT)
            {
                p->setIntValue(p->getIntValue() + prop.getIntValue());
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            }
            else if(p->getType() == DATE && prop.getType() == DATE)
            {
                p->setDateValue(p->getDateValue() + prop.getDateValue());
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            }
            if (p->convertToInt() && prop.convertToInt())
            {
                p->setIntValue(p->getIntValue() + prop.getIntValue());
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), p->getName().c_str());                
                smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            }
        }
        else
        {
            pf->AddProperty(prop);
            storeProfile(key, pf);
            smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), prop.toString().c_str());                                    
            return true;
        }
        return false;
    };

    bool incModProperty(RKey rkey, Property& prop, uint32_t mod, int& res)
    {
        MutexGuard mt(mtx);
        Key key(rkey);
        Profile* pf = getProfile(key, true);
        Property* p = pf->GetProperty(prop.getName().c_str());
        if(p != NULL)
        {
            if(p->getType() == INT && prop.getType() == INT)
            {
                res = p->getIntValue() + prop.getIntValue();
                if(mod) res %= mod;
                p->setIntValue(res);
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            } 
            if (p->convertToInt() && prop.convertToInt())
            {
              res = p->getIntValue() + prop.getIntValue();
              if(mod) res %= mod;
              p->setIntValue(res);
              p->WriteAccess();
              storeProfile(key, pf);
              smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), p->getName().c_str());                
              smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
              return true;
            }
        }
        else
        {
            res = prop.getIntValue();
            if(mod) res %= mod;
            prop.setIntValue(res);
            pf->AddProperty(prop);
            storeProfile(key, pf);
            smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), prop.toString().c_str());            
            return true;
        }
        return false;
    };

    bool incModProperty(Profile *pf, RKey rkey, Property& prop, uint32_t mod, int& res)
    {
        MutexGuard mt(mtx);
        Key key(rkey);
        //Profile* pf = getProfile(key, true);
        if (!pf) {
          return false;
        }
        Property* p = pf->GetProperty(prop.getName().c_str());
        if(p != NULL)
        {
            if(p->getType() == INT && prop.getType() == INT)
            {
                res = p->getIntValue() + prop.getIntValue();
                if(mod) res %= mod;
                p->setIntValue(res);
                p->WriteAccess();
                storeProfile(key, pf);
                smsc_log_info(dblog, "U key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
                return true;
            }
            if (p->convertToInt() && prop.convertToInt())
            {
              res = p->getIntValue() + prop.getIntValue();
              if(mod) res %= mod;
              p->setIntValue(res);
              p->WriteAccess();
              storeProfile(key, pf);
              smsc_log_info(dblog, "D key=\"%s\" name=\"%s\"", key.toString().c_str(), p->getName().c_str());                
              smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), p->toString().c_str());                
              return true;
            }
        }
        else
        {
            res = prop.getIntValue();
            if(mod) res %= mod;
            prop.setIntValue(res);
            pf->AddProperty(prop);
            storeProfile(key, pf);
            smsc_log_info(dblog, "A key=\"%s\" property=%s", key.toString().c_str(), prop.toString().c_str());            
            return true;
        }
        return false;
    };
};

typedef ProfileStore<IntProfileKey, uint32_t, HashProfileStore<IntProfileKey> > IntProfileStore;
//typedef ProfileStore<StringProfileKey, std::string, CachedProfileStore<HashProfileStore<StringProfileKey>, StringProfileKey> > StringProfileStore;

typedef ProfileStore<AbntAddr, const char*, TreeProfileStore<AbntAddr> > StringProfileStore;

}}

#endif // SCAG_PERS_PROFILE_STORE
