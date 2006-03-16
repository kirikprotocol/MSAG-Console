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

namespace scag{ namespace pers{

class IntProfileKey
{
	uint32_t key;

  public:

    IntProfileKey(){};
    IntProfileKey(const IntProfileKey& k):key(k.key) {};
    IntProfileKey(const uint32_t k):key(k) {};

    bool operator==(const IntProfileKey& that)const { return key==that.key; }

    string toString() const
    {
      char buf[16];
      sprintf(buf,":%hu", key);
      string str = buf;
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
#define MAX_STRING_KEY_LEN	30
	char key[MAX_STRING_KEY_LEN];
	uint8_t key_len;

  public:

	StringProfileKey() : key_len(0){};
	void copy(const char*k) { strncpy(key, k, MAX_STRING_KEY_LEN); key[MAX_STRING_KEY_LEN - 1] = 0; key_len = strlen(key); };
		
	StringProfileKey(const StringProfileKey& k) { copy(k.key); };
	StringProfileKey(const char* k) { copy(k); };

	bool operator==(const StringProfileKey& that)const { return !strcmp(key, that.key); }

	string toString() const
	{
		string str(key);
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

	CacheItem(Key& k, Profile *p) { key = k; pf = p; };
	~CacheItem() { delete pf; };
};

template <class Key>
class ProfileStore
{
public:

	ProfileStore(): cache(NULL) {};

	~ProfileStore()
	{
		if(cache != NULL)
		{
			for(int i = 0; i < max_cache_size; i++)
				if(cache[i] != NULL)
				{
					storeProfile(cache[i]->key, cache[i]->pf);
					delete cache[i];
				}
			delete[] cache;
		}
	};

	void init(const string& storeName, uint32_t _max_cache_size)
	{
		log = smsc::logger::Logger::getInstance("store");

		MutexGuard mt(mtx);

		max_cache_size = _max_cache_size;
		cache = new CacheItem<Key>*[_max_cache_size];
		memset(cache, 0, sizeof(CacheItem<Key>*) * _max_cache_size);

		store.init(storeName);
		name = storeName;

		smsc_log_debug(log, "Inited: %s", name.c_str());
	};

/*	void shutdown() {
		store.shutdown();

		smsc_log_debug(log, "Halted: %s", name.c_str());
	}*/

	void setProperty(Key& key, Property& prop)
	{
		MutexGuard mt(mtx);
		Profile *pf;
		Property *p;

		if(prop.isExpired())
			return;

		pf = getProfile(key, true);
		p = pf->GetProperty(prop.getName().c_str());
		if(p != NULL)
		{
			p->setValue(prop);
			p->WriteAccess();
		}
		else
			pf->AddProperty(prop);
		storeProfile(key, pf);
		smsc_log_debug(log, "profile %s, setProperty: %s", key.toString().c_str(), prop.toString().c_str());
	};

	void delProperty(Key& key, const char* nm)
	{
		MutexGuard mt(mtx);
		Profile *pf = getProfile(key, false);

		if(pf != NULL)
		{
			pf->DeleteProperty(nm);
			storeProfile(key, pf);
		}
	};

	bool getProperty(Key& key, const char* nm, Property& prop)
	{
		MutexGuard mt(mtx);
		Profile *pf;
		Property *p;

		pf = getProfile(key, false);

		if(pf != NULL)
		{
			p = pf->GetProperty(nm);

			if(p != NULL)
			{
				if(p->getTimePolicy() == R_ACCESS)
				{
					p->ReadAccess();
					storeProfile(key, pf);
				}
				prop = *p;
				smsc_log_debug(log, "profile %s, getProperty: %s", key.toString().c_str(), prop.toString().c_str());
				return true;
			}
		}

		return false;
	};

	bool incProperty(Key& key, const char* nm, int32_t inc)
	{
		MutexGuard mt(mtx);

		Profile *pf;
		Property *prop;

		pf = getProfile(key, false);

		if(pf != NULL)
		{
			prop = pf->GetProperty(nm);
			if(prop != NULL)
			{
				smsc_log_debug(log, "profile %s, getProperty: %s", key.toString().c_str(), prop->toString().c_str());
				switch(prop->getType())
				{
					case INT:
					{
						int32_t i = prop->getIntValue();
						i += inc;
						prop->setIntValue(i);
						prop->WriteAccess();
						storeProfile(key, pf);
						return true;
					}
					case DATE:
					{
						time_t i = prop->getDateValue();
						i += inc;
						prop->setDateValue(i);
						prop->WriteAccess();
						storeProfile(key, pf);
						return true;
					}
					default:
						return false;
				}
			}
		}

		return false;
	};

	void storeProfile(Key& key, Profile *pf)
	{
		pf->DeleteExpired();

		if(pf->GetCount() > 0)
		{
			try{
				store.updateRecord(key, pf);
			}
			catch(VarRecordNotFound &f) {
				store.newRecord(key, pf);
				smsc_log_debug(log, "Profile %s created.", key.toString().c_str());
			}
		}
		else
			store.deleteRecord(key);
	}

	Profile* getProfile(Key& key, bool create)
	{
		uint32_t i = key.HashCode(0) % max_cache_size;
		if(cache[i] != NULL)
		{
			if(cache[i]->key == key)
				return cache[i]->pf;

//			storeProfile(cache[i]->key, cache[i]->pf);

			try{
				store.getRecord(key, cache[i]->pf);
				cache[i]->key = key;
				return cache[i]->pf;
			}
			catch(VarRecordNotFound &f) {
				if(create)
				{
					cache[i]->pf->Empty();
					cache[i]->key = key;
					return cache[i]->pf;
					smsc_log_debug(log, "Profile %s in cache created.", key.toString().c_str());
				} else
					return NULL;
			}
			catch(SerialBufferOutOfBounds &f) {
				smsc_log_error(log, "Seems storage is corrupted. Profile %s.", key.toString().c_str());
				return NULL;
			}
		}
		else
		{
			Profile *pf = new Profile();
			try{
				store.getRecord(key, pf);
			}
			catch(VarRecordNotFound &f) {
				if(!create)
				{
					delete pf;
					return NULL;
				}
				else
					smsc_log_debug(log, "Profile %s in cache created.", key.toString().c_str());
			}
			catch(SerialBufferOutOfBounds &f) {
				delete pf;
				smsc_log_error(log, "Seems storage is corrupted. Profile %s.", key.toString().c_str());
				return NULL;
			}
			cache[i] = new CacheItem<Key>(key, pf);
			return pf;
		}
			
	};

protected:
	smsc::logger::Logger* log;
	Mutex mtx;
	uint32_t max_cache_size;
	VarRecSizeStore<Key> store;
	string name;
	CacheItem<Key> **cache;
};

typedef ProfileStore<IntProfileKey> IntProfileStore;
typedef ProfileStore<StringProfileKey> StringProfileStore;

}}

#endif // SCAG_PERS_PROFILE_STORE
