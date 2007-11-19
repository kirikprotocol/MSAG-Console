//------------------------------------
//  FSDB.h
//  Routman Michael, 2007
//------------------------------------
//
//  ���� �������� �������� ���������� FSDB.
//

#ifndef ___FSDB_H
#define ___FSDB_H

#include "DataBlock.h"
#include "RBTree.h"
#include "RBTreeHSAllocator.h"
#include "BlocksHSStorage.h"
#include <string>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>

#include "logger/Logger.h"

using std::string;

//const int defaultBlockSize = 8192 // in bytes
//typedef char* defaultKeyType;

const int FSDB_CREATE       = 0x00000001;
const int FSDB_OPEN         = 0x00000002;
const int FSDB_TRUNC        = 0x00000004;
const int FSDB_CLEAR_CACHE  = 0x00000008;

const int FSDB_DEFAULT      = FSDB_OPEN | FSDB_CREATE;

const int ERR_CANNOT_CREATE_DBPATH = 0x01;
const int ERR_DB_NOTEXISTS = 0x02;
const int ERR_DB_NAME = 0x03;


template<class Key = char*, class Value = long>
class FSDB
{
public:
    FSDB(){}
    virtual ~FSDB(){}

//  virtual int Init(const string& _dbName, const string& _dbPath, long indexGrowth, int mode) = 0;
    virtual int Close(void) = 0;

    virtual void Add(const Key& key, const Value& data) = 0;
    virtual bool Set(const Key& key, const Value& data) = 0;
    virtual bool Get(const Key& key, Value& data) = 0;
    virtual bool Change(const Key& key, const Value& data) = 0;
    virtual void Remove(const Key& key) = 0;
};


template<class Key = char*>
class FSDBProfiles: public FSDB<Key, DataBlock>
{
//  typedef FSDBCache<Key, DataBlock>       Cache;
    typedef RBTreeHSAllocator<Key, long>      IndexAllocator;
    typedef RBTree<Key, long>               IndexStorage;

public:
	FSDBProfiles():logger(smsc::logger::Logger::getInstance("FSDB")), indexAllocator(0) {}
    virtual ~FSDBProfiles(){
		if(indexAllocator)
			delete indexAllocator;
	}

    virtual int Init(const string& _dbName, const string& _dbPath="./",
        long _indexGrowth = 1000000,
        long blocksInFile = 100000,
        long blockSize = 2048 - sizeof(templDataBlockHeader<Key>), // 8144
        int mode = FSDB_DEFAULT)
    {
    	int ret;
        if(_dbName.length()==0) return ERR_DB_NAME;

        dbPath = _dbPath; dbName = _dbName;
        indexGrowth = _indexGrowth;

        if(dbPath.length()==0)  dbPath = "./";
        else if(dbPath[dbPath.length()-1] != '/') dbPath += '/';
    	if(indexGrowth <= 0) indexGrowth = 1000000;
    	if(blocksInFile <= 0) blocksInFile = 100000;
    	if(blockSize <= 0) blockSize = 2048 - sizeof(templDataBlockHeader<Key>);

        if(!dbPathExists())
        {
		smsc_log_info(logger, "FSDBProfiles::Init: db not Exists");
		if(mode & FSDB_CREATE) 
		{
			if(!createDBPath())
			{
				smsc_log_info(logger, "createDBPath - failed");
				return ERR_CANNOT_CREATE_DBPATH;
			}
		}
		else
		{
			smsc_log_info(logger, "FSDB not exist. Create DB first. %d", mode);
			return ERR_DB_NOTEXISTS;
		}
        }
//        indexAllocator = new RBTreeHSAllocator<Key, long>(dbPath + dbName + '/' + dbName + "-index", indexGrowth);
    	indexAllocator = new IndexAllocator();
    	if(0 != (ret = indexAllocator->Init(dbPath + dbName + '/' + dbName + "-index", indexGrowth)))
    	{
    		smsc_log_info(logger, "Init FSDB failed %d", ret);
    		//printf("Init FSDB failed %d\n", ret);
    		return ret;
    	}
    	indexStorage.SetAllocator(indexAllocator);
    	indexStorage.SetChangesObserver(indexAllocator);
//      cache.Init(cacheSize);//, (dbPath + dbName).c_str(), dbName + "-cache", mode & FSDB_CLEAR_CACHE);
        if(0 != (ret=dataStorage.Open(dbName + "-data", dbPath + '/' + dbName)))
        {
    		smsc_log_info(logger, "Create Data Storage %d", ret);
    		dataStorage.Create(dbName + "-data", dbPath + '/' + dbName, blocksInFile, blockSize);
        }
    	smsc_log_info(logger, "Inited: storageName = '%s',  storagePath = '%s'", dbName.c_str(), dbPath.c_str());
    	smsc_log_info(logger, "Inited: indexGrowth = %d,  blocksInFile = %d", indexGrowth, blocksInFile);
        return 0;
    }
    
    virtual int Close(void)
    {
//      cache.Release();
        return 0;
    }

    virtual void Add(const Key& key, const DataBlock& data)
    {
        long idx;
        dataStorage.Add(data, idx, key);
        indexStorage.Insert(key, idx);
//      cache.Add(key, data);
//      printf("FSDB - Add\n");
    }
    virtual bool Set(const Key& key, const DataBlock& data)
    {
    	smsc_log_debug(logger, "Set: %s, %d", key.toString().c_str(), data.length());
        long idx;
        if(indexStorage.Get(key, idx))
    	{
    		//return dataStorage.Change(idx, data, key);
          if (!dataStorage.Change(idx, data, key)) {
            dataStorage.Add(data, idx, key);
            return false;
          }
          return true;
    	}
        dataStorage.Add(data, idx, key);
    	indexStorage.Insert(key, idx);
    	return true;
    }
    virtual bool Get(const Key& key, DataBlock& data)
    {
        long idx;
		bool ret;
        if(indexStorage.Get(key, idx))
		{
//          if(ret = dataStorage.Get(idx, data))
//              cache.Add(key, data);
			ret = dataStorage.Get(idx, data);
			smsc_log_debug(logger, "Get: %s, %d", key.toString().c_str(), data.length());
            return ret;
		}
		smsc_log_debug(logger, "Get: %s, No data", key.toString().c_str(), data.length());
        return false;
    }

    virtual bool Change(const Key& key, const DataBlock& data)
    {
        long idx;
        if(indexStorage.Get(key, idx))
            return dataStorage.Change(idx, data, key);
        return false;
    }
    
    virtual void Remove(const Key& key)
    {
        long idx;
        if(indexStorage.Get(key, idx))
            dataStorage.Remove(idx);
    }

    virtual void Reset()
    {
       indexStorage.Reset();
    }
    
    virtual bool Next(Key& key, DataBlock& data)
    {
        long idx;
		bool ret;
        if(indexStorage.Next(key, idx))
		{
//          if(ret = dataStorage.Get(idx, data))
//              cache.Add(key, data);
			ret = dataStorage.Get(idx, data);
			smsc_log_debug(logger, "Next: %s, %d", key.toString().c_str(), data.length());
            return ret;
		}
		smsc_log_debug(logger, "Next: %s, No data", key.toString().c_str());
        return false;
    }
    
    bool RebuildIndex(std::string& _dbName, std::string& _dbPath, long _indexGrowth)
    {
    	BlocksHSStorage<Key> tempDataStorage;
        IndexStorage tempIdxStorage;
    	auto_ptr<IndexAllocator> tempIndexAllocator(new IndexAllocator());

        if(0 != tempDataStorage.Open(_dbName + "-data", _dbPath + "/" + _dbName))
        {
    		smsc_log_info(logger, "Cannot open data file: path=%s, name=%s", _dbPath.c_str(), _dbName.c_str());
            return false;
        }
        
        if(_dbPath.length()==0)  _dbPath = "./";
        else if(_dbPath[_dbPath.length()-1] != '/') _dbPath += '/';
        

        std::string n = _dbPath + _dbName + '/' + _dbName + "-index", t = n + "-temp", o = n + "-old";
        
    	if(0 != tempIndexAllocator->Init(t, _indexGrowth))
    	{
    		smsc_log_info(logger, "Init temp index failed: %s", t.c_str());
    		return false;
    	}
    	tempIdxStorage.SetAllocator(tempIndexAllocator.get());
    	tempIdxStorage.SetChangesObserver(tempIndexAllocator.get());
  		smsc_log_info(logger, "Rebuild: temp index inited: %s", t.c_str());        
        DataBlock data;
        long blockIndex, i = 0;
        Key key;
        while(tempDataStorage.Next(blockIndex, data, key))
        {
            smsc_log_debug(logger, "Rebuild: key=%s, idx=%d", key.toString().c_str(), blockIndex);
            tempIdxStorage.Insert(key, blockIndex);
            i++;
        }
        
        smsc_log_debug(logger, "Rebuild: %d records reindexed", i);

        rename(n.c_str(), o.c_str());
        rename(t.c_str(), n.c_str());
        t += ".trx";
        n += ".trx";
        o += ".trx";
        rename(n.c_str(), o.c_str());        
        rename(t.c_str(), n.c_str());
        return true;
    }

private:
	smsc::logger::Logger* logger;
	string              dbPath;
	string              dbName;
    long                indexGrowth;
	
	IndexAllocator*     indexAllocator;
	IndexStorage        indexStorage;
	//  Cache               cache;
	BlocksHSStorage<Key> dataStorage;
	//  BlocksMappedFileStorage dataStorage;
	
	bool dbPathExists(void)
	{
		char* pwd;
		pwd = getenv("PWD");
		if(0 == chdir((dbPath + dbName).c_str()))
		{
		chdir(pwd);
		return true;
		}
		return false;
	}
	
	bool createDBPath(void)
	{
		int pos = 0;
		int ret;
		string path = dbPath + dbName + '/';
		while(pos != path.length() - 1)
		{
		pos = path.find('/', pos + 1);
		//printf("%s - ", (path.substr(0, pos)).c_str());
		ret = mkdir((path.substr(0, pos)).c_str(), 0700);
		//if(-1 == (ret = mkdir((path.substr(0, pos)).c_str(), 0700)))
		//{
		//  printf(":(((((((\n");
		//}
		//else
		//  printf(":))))))))\n");
	
		}
		if(-1 == ret)
		return false;
		return true;
	}
	};


#endif
