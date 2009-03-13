//------------------------------------
//  FSDB.h
//  Routman Michael, 2007
//------------------------------------
//
//  ���� �������� �������� ���������� FSDB.
//

#ifndef ___FSDB_H
#define ___FSDB_H

#include "scag/util/storage/DataBlock.h"
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
using scag::pers::Profile;
using scag::util::storage::DataBlock;

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

    virtual int Close(void) = 0;

    virtual void Add(const Key& key, Value& data) = 0;
    virtual bool Set(const Key& key, Value& data) = 0;
    virtual bool Get(const Key& key, Value& data) = 0;
    virtual bool Change(const Key& key, Value& data) = 0;
    virtual void Remove(const Key& key, Value& data) = 0;
};


template<class Key = char*>
class FSDBProfiles: public FSDB<Key, Profile>
{
    typedef RBTreeHSAllocator<Key, long>    IndexAllocator;
    typedef RBTree<Key, long>               IndexStorage;
    typedef templRBTreeNode<Key, long>      IndexNode;
    typedef IndexStorage::nodeptr_type      nodeptr_type;

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
    	indexAllocator = new IndexAllocator();
    	if(0 != (ret = indexAllocator->Init(dbPath + dbName + '/' + dbName + "-index", indexGrowth)))
    	{
    		smsc_log_info(logger, "Init FSDB failed %d", ret);
    		return ret;
    	}
    	indexStorage.SetAllocator(indexAllocator);
    	indexStorage.SetChangesObserver(indexAllocator);
        if(0 != (ret=dataStorage.Open(dbName + "-data", dbPath + '/' + dbName)))
        {
          if (ret == BlocksHSStorage<Key>::CANNOT_OPEN_DATA_FILE ||
              ret == BlocksHSStorage<Key>::CANNOT_OPEN_EXISTS_DESCR_FILE) {
            smsc_log_error(logger, "Data Storage Open Failed %d", ret);
            return ret;
          } 
          smsc_log_info(logger, "Create Data Storage %d", ret);
          if (0 != (ret = dataStorage.Create(dbName + "-data", dbPath + '/' + dbName, blocksInFile, blockSize))) {
            smsc_log_error(logger, "Data Storage Create Failed %d", ret);
            return ret;
          }
        }
    	smsc_log_info(logger, "Inited: storageName = '%s',  storagePath = '%s'", dbName.c_str(), dbPath.c_str());
    	smsc_log_info(logger, "Inited: indexGrowth = %d,  blocksInFile = %d", indexGrowth, blocksInFile);
        return 0;
    }
    
    virtual int Close(void)
    {
        return 0;
    }

    virtual void Add(const Key& key, Profile& profile)
    {
        long idx;
        smsc_log_debug(logger, "Add: %s", key.toString().c_str());
        if (!dataStorage.Add(profile, key, idx)) {
          smsc_log_warn(logger, "Error Add: %s", key.toString().c_str());
          return;
        }
        indexStorage.Insert(key, idx);
    }

    virtual bool Set(const Key& key, Profile& profile)
    {
        smsc_log_debug(logger, "Set: %s", key.toString().c_str());
        nodeptr_type node = indexStorage.Get(key);
        if (node) {
            IndexNode* n = indexStorage.realAddr(node);
          long nodeValue = n->value;
          bool res = dataStorage.Change(profile, key, nodeValue);
          if (res) {
            indexStorage.setNodeValue(node, nodeValue);
          }
          return res;
        }
        smsc_log_debug(logger, "Set: %s, No data", key.toString().c_str());
        Add(key, profile);
        return true;
    }

    virtual bool Get(const Key& key, Profile& profile)
    {
        long idx;
		bool ret;
        if(indexStorage.Get(key, idx))
		{
			ret = dataStorage.Get(idx, profile);
            smsc_log_debug(logger, "Get: %s", key.toString().c_str());
            return ret;
		}
		smsc_log_debug(logger, "Get: %s, No data", key.toString().c_str());
        return false;
    }

    virtual bool Change(const Key& key, Profile& profile)
    {
        long idx;
        if(indexStorage.Get(key, idx))
            return dataStorage.Change(profile, key, idx);
        return false; 
    }
    
    virtual void Remove(const Key& key, Profile& profile)
    {
        long idx;
        if(indexStorage.Get(key, idx)) {
          dataStorage.Remove(idx, profile);
        }
    }

    virtual void Reset()
    {
       indexStorage.Reset();
    }

    void resetStorage() {
      indexStorage.Reset();
      dataStorage.Reset();
    }

    bool dataStorageNext(Key& key, DataBlock& dataBlock) {
      long blockIndex;
      if (dataStorage.Next(blockIndex, dataBlock, key)) {
        return true;
      }
      return false;
    }
    
    virtual bool Next(Key& key, DataBlock& data)
    {
        long idx;
		bool ret;
        if(indexStorage.Next(key, idx))
		{
			ret = dataStorage.Get(idx, data);
			smsc_log_debug(logger, "Next: %s, %d ret=%d", key.toString().c_str(), data.length(), ret);
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
	BlocksHSStorage<Key> dataStorage;
	
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
          ret = mkdir((path.substr(0, pos)).c_str(), 0700);
		}
		if(-1 == ret)
		return false;
		return true;
	}
	};


#endif

    /*
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
        //dataStorage.Add(data, idx, key);
        //indexStorage.Insert(key, idx);
        Add(key, data);
        return true;
    }*/


