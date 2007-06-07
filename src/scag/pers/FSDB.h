//------------------------------------
//  FSDB.h
//  Routman Michael, 2007
//------------------------------------
//
//  Файл содержит описание интерфейса FSDB.
//

#ifndef ___FSDB_H
#define ___FSDB_H

#include "DataBlock.h"
#include "RBTree.h"
#include "RBTreeFileAllocator.h"
//#include "BlocksMappedFileStorage.h"
#include "BlocksFileStorage.h"
//#include "FSDBCache.h"
#include <string>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>

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
    typedef RBTreeAllocator<Key, long>      IndexAllocator;
    typedef RBTree<Key, long>               IndexStorage;

public:
    FSDBProfiles(){}
    virtual ~FSDBProfiles(){}

    virtual int Init(const string& _dbName, const string& _dbPath="./",
        long indexGrowth = 1000000,
        long fileSize = 100000,
        long blockSize = 8192 - sizeof(templDataBlockHeader<Key>), // 8144
        int mode = FSDB_DEFAULT)
    {
        if(_dbName.length()==0) return ERR_DB_NAME;
        
        dbPath = _dbPath; dbName = _dbName;
        
        if(dbPath.length()==0)  dbPath = "./";
        else if(dbPath[dbPath.length()-1] != '/') dbPath += '/';
//      printf("%s\n", dbPath.c_str());
        if(!dbPathExists())
        {
    //      printf("FSDBProfiles::Init: db not Exists\n");
            if(mode & FSDB_CREATE) 
            {
                if(!createDBPath())
                {
//                  printf("createDBPath - failed\n");
                    return ERR_CANNOT_CREATE_DBPATH;
                }
            }
            else
                return ERR_DB_NOTEXISTS;
        }
        indexAllocator = new RBTreeFileAllocator<Key, long>(dbPath + dbName + '/' + dbName + "-index", indexGrowth);
        indexStorage.SetAllocator(indexAllocator);
//      cache.Init(cacheSize);//, (dbPath + dbName).c_str(), dbName + "-cache", mode & FSDB_CLEAR_CACHE);
        int ret;
        if(0 != (ret=dataStorage.Open(dbName + "-data", dbPath + '/' + dbName)))
        {
//          printf("Create Data Storage %d\n", ret);
            dataStorage.Create(dbName + "-data", dbPath + '/' + dbName, fileSize, blockSize);
        }
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
        long idx;
        if(indexStorage.Get(key, idx))
            return dataStorage.Change(idx, data, key);

        dataStorage.Add(data, idx, key);
        indexStorage.Insert(key, idx);
        return true;
    }
    virtual bool Get(const Key& key, DataBlock& data)
    {
        long idx;
//      bool ret;
//      if(!(ret = cache.Get(key, data)))
        if(indexStorage.Get(key, idx))
//          if(ret = dataStorage.Get(idx, data))
//              cache.Add(key, data);
            return dataStorage.Get(idx, data);
        return false;
    }

    virtual bool Change(const Key& key, const DataBlock& data)
    {
        long idx;
        bool ret;
        if(indexStorage.Get(key, idx))
            return dataStorage.Change(idx, data, key);
    }
    
    virtual void Remove(const Key& key)
    {
        long idx;
        if(indexStorage.Get(key, idx))
            dataStorage.Remove(idx);
    }

private:
    string              dbPath;
    string              dbName;

    IndexAllocator*     indexAllocator;
    IndexStorage        indexStorage;
//  Cache               cache;
    BlocksFileStorage<Key> dataStorage;
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
            printf("%s - ", (path.substr(0, pos)).c_str());
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
