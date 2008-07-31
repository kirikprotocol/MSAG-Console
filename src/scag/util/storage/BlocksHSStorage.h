//------------------------------------
//  BlocksHSStorage.h
//  Odarchenko Vitaly, 2008
//------------------------------------
//

#ifndef _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE_H
#define _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE_H

#include <string>
#include <vector>
#include <cstdio>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "core/buffers/File.hpp"
#include "SerialBuffer.h"
#include "GlossaryBase.h"
#include "DataBlockBackup.h"
#include "util/Exception.hpp"

namespace scag {
namespace util {
namespace storage {

using std::vector;
using std::string;
using smsc::logger::Logger;

const string dfPreambule= "RBTREE_FILE_STORAGE!";
const int dfVersion_32_1 = 0x01;
const int dfVersion_64_1 = 0x80000001;

const int dfMode = 0600;

//const int SUCCESS                 = 0;
//const int RBTREE_FILE_NOT_SPECIFIED   = -1;
const int CANNOT_CREATE_DESCR_FILE  = -2;
const int CANNOT_CREATE_DATA_FILE   = -3;
const int CANNOT_OPEN_DESCR_FILE    = -4;
const int CANNOT_OPEN_DATA_FILE     = -5;
const int DESCR_FILE_MAP_FAILED     = -6;
//const int RBTREE_FILE_ERROR           = -10;

//const int defaultBlockSize = 8192; // in bytes
//const int defaultFileSize = 10000; // in blocks

const int defaultBlockSize = 56; // in bytes
const int defaultFileSize = 3; // in blocks 
const long long BLOCK_USED   = (long long)1 << 63;

static const uint8_t TRX_COMPLETE   = 0;
static const uint8_t TRX_INCOMPLETE = 1;

static const size_t WRITE_BUF_SIZE = 10240;
static const size_t PROFILE_MAX_BLOCKS_COUNT = 10;
                                          


struct DescriptionFile   
{
    typedef long index_type;
    char preamble[20];
    int version;

    int files_count;
    int block_size;         // in bytes;
    int file_size;          // in blocks
    int blocks_used;
    int blocks_free;
    index_type first_free_block;
    char Reserved[12];
};

template<class Key>
struct templBackupHeader {
    typedef DescriptionFile::index_type index_type;
  long blocksCount;
  long dataSize;
  index_type curBlockIndex;
  Key key;
};


template<class Key>
struct templDataBlockHeader
{
    typedef DescriptionFile::index_type index_type;
    union
    {
        index_type next_free_block;
        long block_used;
    };
    Key     key;
    long    total_blocks;
    long    data_size;
    index_type    next_block;
    bool    head;
};

template<class Key>
struct templDataBlock {
  templDataBlockHeader<Key> hdr;
  DataBlock data;
  templDataBlock(const templDataBlockHeader<Key>& _hdr, const DataBlock& _data)
                 :hdr(_hdr), data(_data) {}
  templDataBlock(const templDataBlockHeader<Key>& _hdr)
                 :hdr(_hdr), data(0) {}
  templDataBlock():data(0) {
    memset((void*)&hdr, 0, sizeof(templDataBlockHeader<Key>));
  }
};


template<class Key, class Profile>
class BlocksHSStorage
{
public:
    typedef DescriptionFile::index_type index_type;

private:
    typedef templDataBlockHeader<Key> DataBlockHeader;
    typedef templDataBlock<Key> CompleteDataBlock; 
    typedef templBackupHeader<Key> BackupHeader;

public:
//	static const int SUCCESS			           = 0;
//	static const int RBTREE_FILE_NOT_SPECIFIED	   = -1;
	static const int CANNOT_CREATE_STORAGE	       = -1;
    static const int CANNOT_CREATE_DESCR_FILE	   = -2;
	static const int CANNOT_CREATE_DATA_FILE	   = -3;
	static const int CANNOT_OPEN_DESCR_FILE	       = -4;
	static const int CANNOT_OPEN_DATA_FILE		   = -5;
	static const int DESCR_FILE_CREATE_FAILED	   = -6;
	static const int DESCR_FILE_OPEN_FAILED		   = -7;
	//static const int RBTREE_FILE_ERROR		   = -10;
    static const int CANNOT_OPEN_EXISTS_DESCR_FILE = -8;
    static const int CANNOT_OPEN_EXISTS_DATA_FILE  = -9;
    static const int BACKUP_FILE_CREATE_FAILED	   = -11;
    static const int BACKUP_FILE_OPEN_FAILED	   = -12;

    static const int defaultBlockSize = 56; // in bytes
    static const int defaultFileSize = 3; // in blocks 
    static const long BLOCK_USED	= long(uint64_t(1) << 63);

    BlocksHSStorage(GlossaryBase* g = NULL): glossary_(g), running(false), iterBlockIndex(0)
    {
        if (!glossary_) {
          throw std::runtime_error("BlocksHSStorage: glossary should be provided!");
        }
        logger = smsc::logger::Logger::getInstance("BlkStore");
        hdrSize = sizeof(DataBlockHeader);
    }


    virtual ~BlocksHSStorage()
    {
        Close();
    }


    int Create(const string& _dbName, const string& _dbPath = "",
               long _fileSize = defaultFileSize,
               long _blockSize = defaultBlockSize - sizeof(DataBlockHeader))
    {
        dbName = _dbName;
        dbPath = _dbPath;
        if (_blockSize > WRITE_BUF_SIZE) {
            smsc_log_error(logger, "block size %d too large. max block size = %d", _blockSize, WRITE_BUF_SIZE);
            return CANNOT_CREATE_STORAGE;
        }
		
        int ret;
        if(0 != (ret = CreateDescriptionFile(_blockSize + hdrSize, _fileSize)))
            return ret;
        try {
            CreateDataFile();
        } catch (const std::exception& e) {
            return CANNOT_CREATE_DATA_FILE;
        }
        if(0 != (ret = CreateBackupFile()))
            return ret;

        running = true;
        return 0;
    }


    int Open(const string& _dbName, const string& _dbPath = 0)
    {
        dbName = _dbName;
        dbPath = _dbPath;
		
        int ret;
        if(0 != (ret = OpenDescriptionFile()))
            return ret;
        if(0 != (ret = OpenDataFiles()))
            return ret;
        if(0 != (ret = OpenBackupFile()))
            return ret;

        running = true;
        return 0;
    }
	

    void Close(void)
    {
        if(running)
        {
            for(int i = 0; i < dataFile_f.size(); i++)
            {
                dataFile_f[i]->Close();
                delete dataFile_f[i];
            }
            dataFile_f.clear();
            descrFile_f.Close();
            backupFile_f.Close();
            running = false;
        }
    }


    bool Add( DataBlockBackup<Profile>& prof, const Key& key, index_type& blockIndex) {
        Profile& profile = *prof.value;
        BlocksHSBackupData& bkp = *prof.backup;
        //SerialBuffer data;
        profileData.Empty();
        profile.Serialize(profileData, true, glossary_);
        smsc_log_debug(logger, "Add data block key='%s' length=%d", key.toString().c_str(), profileData.length());
        int dataLength = profileData.length();
        if (dataLength <= 0) {
            return false;
        }
        index_type ffb = descrFile.first_free_block;
        DescriptionFile oldDescrFile = descrFile;
        backupHeader.key = key;
        if (!backUpProfile(key, -1, bkp, dataLength, true)) {
            blockIndex = -1;
            smsc_log_error(logger, "Backup profile error");
            return false;
        }
        try {
            descrFile.first_free_block = addBlocks(ffb, 0, backupHeader.blocksCount, profileData,
                                                   key, bkp);
            changeDescriptionFile();
            clearBackup();
            blockIndex = ffb;
            bkp.setBackupData(profileData.c_ptr(), profileData.length());
            return true;
        } catch (const std::exception& e) {
            blockIndex = -1;
            restoreDataBlocks(oldDescrFile, bkp.getBackupData());
            // FIXME: profile cleanup is its own responsibility
            // profile.Empty();
            descrFile = oldDescrFile;
            changeDescriptionFile();
            //exit(-1);
            throw;
        }
    }

    
    bool Change(Profile& profile, const Key& key, index_type& blockIndex) {
        if (blockIndex == -1) {
            return Add(profile, key, blockIndex);
        }
        profileData.Empty();
        profile.Serialize(profileData, true, glossary_);
        smsc_log_debug(logger, "Change data block index=%d key='%s' length=%d",
                       blockIndex, key.toString().c_str(), profileData.length());
        DescriptionFile oldDescrFile = descrFile;
        index_type ffb = descrFile.first_free_block;
        backupHeader.key = key;
        int dataLength = profileData.length();
        if (!backUpProfile(key, blockIndex, profile, dataLength, true)) {
            smsc_log_error(logger, "Backup profile error");
            return false;
        }
        int blocksCount = (dataLength + effectiveBlockSize - 1) / effectiveBlockSize;
        int oldBlocksCount = (backupHeader.dataSize + effectiveBlockSize - 1) / effectiveBlockSize;
        int updateBlocksCount = blocksCount <= oldBlocksCount ? blocksCount : oldBlocksCount;

        try {
            profile.clearBackup();
            index_type curBlockIndex = addBlocks(blockIndex, 0, updateBlocksCount, profileData, key, profile); 
            if (blocksCount > oldBlocksCount) {
                smsc_log_debug(logger, "Add new blocks");
                descrFile.first_free_block = addBlocks(ffb, updateBlocksCount, blocksCount,
                                                       profileData, key, profile);
            }
            if (blocksCount < oldBlocksCount) {
                smsc_log_debug(logger, "Remove empty blocks");
                descrFile.first_free_block = removeBlocks(ffb, curBlockIndex, updateBlocksCount);
            }
            changeDescriptionFile();
            clearBackup();
            if (blocksCount == 0) {
                blockIndex = -1;
            }
            profile.setBackupData(profileData);
            return true;
        } catch (const std::exception& e) {
            profile.restoreBackup(dataBlockBackup, oldBlocksCount);
            restoreDataBlocks(oldDescrFile, profile.getBackupData());
            descrFile = oldDescrFile;
            changeDescriptionFile();
            //exit(-1);
            throw;
        }

    }
    

    bool Get(index_type blockIndex, DataBlock& data)
    {
        smsc_log_debug(logger, "Get data block index=%d ", blockIndex);
        if(blockIndex == -1) return false;
        char* buff;
        index_type curBlockIndex = blockIndex;
        int i = 0;
        do
        {
            DataBlockHeader hdr;

            int file_number = curBlockIndex / descrFile.file_size;
            if (!checkfn( file_number )) return false;
            off_t offset = (curBlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
            File* f = dataFile_f[file_number];
            f->Seek(offset, SEEK_SET);
            f->Read((void*)&hdr, hdrSize);
            if(hdr.block_used != BLOCK_USED)
                return false;
            if(curBlockIndex == blockIndex)
            {
                data.setBuffLength(hdr.data_size);
                buff = data.ptr();
                data.setLength(hdr.data_size);
            }
            if(-1 == hdr.next_block)
                f->Read((void*)(buff+(i*effectiveBlockSize)), hdr.data_size - effectiveBlockSize*i);
            else
                f->Read((void*)(buff+(i*effectiveBlockSize)), effectiveBlockSize);

            curBlockIndex = hdr.next_block;
            i++;
        }
        while(-1 != curBlockIndex);
        return true;
    }


    bool Get(index_type blockIndex, DataBlockBackup<Profile>& prof )
    {
        Profile& profile = *prof.value;
        BlocksHSBackupData& bkp = *prof.backup;
        smsc_log_debug(logger, "Get data block index=%d ", blockIndex);
        if(blockIndex == -1) return false;
        char* buff;
        index_type curBlockIndex = blockIndex;
        int i = 0;
        //SerialBuffer data;
        //TODO: catch file exceptions
        profileData.Empty();
        do
        {
            DataBlockHeader hdr;
            int file_number = curBlockIndex / descrFile.file_size;
            if ( !checkfn(file_number) ) return false;
            off_t offset = (curBlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
            File* f = dataFile_f[file_number];
            f->Seek(offset, SEEK_SET);
            f->Read((void*)&hdr, hdrSize);
            if(hdr.block_used != BLOCK_USED) {
                smsc_log_error(logger, "block index=%d unused", curBlockIndex);
                return false;
            }
            if(curBlockIndex == blockIndex)
            {
                profileData.setBuffLength(hdr.data_size);
                buff = profileData.ptr();
                profileData.setLength(hdr.data_size);
            }
            size_t dataSize = hdr.next_block == -1 ? 
                hdr.data_size - effectiveBlockSize*i : effectiveBlockSize;
            f->Read((void*)(buff+(i*effectiveBlockSize)), dataSize);
            curBlockIndex = hdr.next_block;
            ++i;
            bkp.addDataToBackup(hdr.next_block);
        }
        while(-1 != curBlockIndex);
        profile.Deserialize(profileData, true, glossary_);
        bkp.setBackupData(profileData.c_ptr(), profileData.length());
        return true;
    }


    void Remove(const Key& key, index_type blockIndex, const DataBlockBackup<Profile>& prof) {
        Profile& profile = *prof.value;
        BlocksHSBackupData& bkp = *prof.backup;
        smsc_log_debug(logger, "Remove data block index=%d ", blockIndex);
        if (blockIndex < 0) {
            smsc_log_warn(logger, "Can't remove data block index=%d ", blockIndex);
            return;
        }
        DescriptionFile oldDescrFile = descrFile;
        index_type ffb = descrFile.first_free_block;
        if (!backUpProfile(key, blockIndex, bkp, 0, true)) {
            return;
        }
        try {
            descrFile.first_free_block = removeBlocks(ffb, blockIndex, 0);
            changeDescriptionFile();
            clearBackup();
        } catch (const std::exception& e) {
            restoreDataBlocks(oldDescrFile, bkp.getBackupData());
            descrFile = oldDescrFile;
            changeDescriptionFile();
            exit(-1);
        }
    }


    void Reset()
    {
        iterBlockIndex = 0;
    }

    
    bool Next(index_type& blockIndex, DataBlock& data, Key& key)
    {
        uint64_t cnt = descrFile.files_count * descrFile.file_size;
        DataBlockHeader hdr;
        
        while(iterBlockIndex < cnt)
        {
            int file_number = iterBlockIndex / descrFile.file_size;
            off_t offset = (iterBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
            File* f = dataFile_f[file_number];
            f->Seek(offset, SEEK_SET);
            f->Read((void*)&hdr, hdrSize);
            blockIndex = iterBlockIndex++;            
            if(hdr.block_used == BLOCK_USED && hdr.head)
            {
                key = hdr.key;
                if(!Get(blockIndex, data))
                    smsc_log_error(logger, "Error reading block: %d", blockIndex);
                else
                    return true;
            }
        }
        return false;
    }


private:

  Logger* logger;
  GlossaryBase* glossary_;
  bool running;
  string dbName;
  string dbPath;
  DescriptionFile descrFile;		
  File descrFile_f;
  File backupFile_f;
  vector<File*> dataFile_f;
  index_type iterBlockIndex;
  long effectiveBlockSize;	

  vector<index_type> dataBlockBackup;
  char writeBuf[WRITE_BUF_SIZE];
  size_t hdrSize;
  SerialBuffer profileData;
  CompleteDataBlock completeDataBlock;
  BackupHeader backupHeader;

private:

    void printHdr(const DataBlockHeader& hdr) {
        smsc_log_debug(logger, "block header used=%d next_free=%d head=%d total=%d next=%d size=%d key='%s'",
                       hdr.block_used, hdr.next_free_block, hdr.head, hdr.total_blocks, 
                       hdr.next_block, hdr.data_size, hdr.key.toString().c_str());
    }


    void writeDataBlock(int fileNumber, off_t offset, const DataBlockHeader& hdr, const void* data, size_t curBlockSize)
    {
        smsc_log_debug(logger, "write data block fn=%d, offset=%d, blockSize=%d", fileNumber,
                       offset, curBlockSize);
        //printHdr(hdr);
        if ( !checkfn(fileNumber) ) {
            throw smsc::util::Exception("Invalid file number %d, max file number %d", fileNumber, descrFile.files_count-1);
        }
        size_t bufSize = hdrSize + curBlockSize;
        //TODO compare bufSize and WRITE_BUF_SIZE
        memcpy(writeBuf, &hdr, hdrSize);
        memcpy(writeBuf + hdrSize, data, curBlockSize);
        try {
            File* f = dataFile_f[fileNumber];
            f->Seek(offset, SEEK_SET);
            f->Write((void *)writeBuf, bufSize);
        } catch (const std::exception& e) {
            smsc_log_warn(logger, "Error writing block to data file %d. std::exception: '%s'",
                          fileNumber,e.what());
            throw;
        }
    }

    index_type addBlocks( index_type curBlockIndex, size_t backupStartIndex, size_t blocksCount,
                          const DataBlock& data, const Key& key, BlocksHSBackupData& bkp )
    {
        int dataLength = data.length();
        DataBlockHeader hdr;
        hdr.block_used = BLOCK_USED;
        hdr.key = key;
        hdr.head = backupStartIndex == 0 ? true : false;
        hdr.total_blocks = (dataLength + effectiveBlockSize - 1) / effectiveBlockSize;
        hdr.data_size = dataLength;//effectiveBlockSize;
        size_t curBlockSize = 0;
        for (int i = backupStartIndex; i < blocksCount; ++i) {
            int file_number = curBlockIndex / descrFile.file_size;
            off_t offset = (curBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
            curBlockIndex = dataBlockBackup[i];
            if (i == hdr.total_blocks - 1) {
                hdr.next_block =  -1; 
                curBlockSize = dataLength - effectiveBlockSize * i;
            } else {
                hdr.next_block = curBlockIndex; 
                curBlockSize = effectiveBlockSize;
            }
            writeDataBlock(file_number, offset, hdr,
                           (void*)(data.c_ptr() + i * (effectiveBlockSize)), curBlockSize);
            bkp.addDataToBackup(hdr.next_block);
            hdr.head = false;
        }
        smsc_log_debug(logger, "addBlocks curBlockIndex=%d", curBlockIndex);
        return curBlockIndex;
    }


    index_type getFirstFreeBlock(int fileNumber, off_t offset)
    {
        if (!checkfn(fileNumber)) {
            throw smsc::util::Exception("Invalid file number %d, max file number %d", fileNumber, descrFile.files_count-1);
        }
        File* f = dataFile_f[fileNumber];
        f->Seek(offset, SEEK_SET);
        index_type ffb = 0;
        f->Read((void*)&(ffb), sizeof(descrFile.first_free_block));
        if (ffb == -1) {
            ffb = descrFile.files_count * descrFile.file_size;
            CreateDataFile();
        }
        smsc_log_debug(logger, "First free block %d", ffb);
        return ffb;
    }

    index_type removeBlocks(index_type ffb, index_type blockIndex, size_t backupIndex) {
        smsc_log_debug(logger, "Remove %d blocks start block index=%d", 
                       backupHeader.blocksCount - backupIndex, blockIndex);
        if (backupIndex >= backupHeader.blocksCount) {
            smsc_log_warn(logger, "Can't remove unbackuped data blocks");
            return ffb;
        }
        while (blockIndex != -1) {
            smsc_log_debug(logger, "Remove %d block, ffb=%d", blockIndex, ffb); 
            int file_number = blockIndex / descrFile.file_size;
            if (!checkfn(file_number)) {
                throw smsc::util::Exception("Invalid file number %d, max file number %d", file_number, descrFile.files_count-1);
            }
            off_t offset = (blockIndex - file_number * descrFile.file_size) * descrFile.block_size;
            File* f = dataFile_f[file_number];
            f->Seek(offset);
            f->Write((void*)(&ffb), sizeof(descrFile.first_free_block));
            ffb = blockIndex;
            blockIndex = dataBlockBackup[backupIndex];
            ++backupIndex;
        }
        return ffb;
    }

    int CreateBackupFile() {
        try {
            smsc_log_debug(logger, "Create backup file");
            backupFile_f.RWCreate((dbPath + '/' + dbName + ".trx").c_str());
            backupFile_f.SetUnbuffered();
            backupFile_f.Write((void*)&TRX_COMPLETE, sizeof(TRX_COMPLETE));
            return 0;
        } catch (const std::exception& e) {
            smsc_log_error(logger, "FSStorage: error create backup file: '%s'", e.what());
            return BACKUP_FILE_CREATE_FAILED;
        }
    }

    int CreateDescriptionFile(long _blockSize, long _fileSize)
    {
        try
        {
            string name = dbPath + '/' + dbName;
            if (File::Exists(name.c_str())) {
                smsc_log_error(logger, "FSStorage: error create description file: file '%s' already exists",
                               name.c_str());
                return DESCR_FILE_CREATE_FAILED;
            }
            descrFile_f.RWCreate(name.c_str());
            descrFile_f.SetUnbuffered();
        }
        catch(const std::exception& e)
        {
            smsc_log_error(logger, "FSStorage: error create description file: '%s'", e.what());
            return DESCR_FILE_CREATE_FAILED;
        }

        // initialize Description File structure
        memcpy((void*)&(descrFile.preamble), dfPreambule.c_str(), sizeof(descrFile.preamble));
        descrFile.version = dfVersion_64_1;
        descrFile.block_size = _blockSize;
        descrFile.file_size = _fileSize;
        descrFile.files_count = 0;
        descrFile.blocks_free = 0;
        descrFile.blocks_used = 0;
        descrFile.first_free_block = 0;
        memset((void*)descrFile.Reserved, 0, sizeof(descrFile.Reserved));
        effectiveBlockSize = descrFile.block_size - sizeof(DataBlockHeader);
        changeDescriptionFile();
        return 0;
    }


    std::string makeDataFileName( int num, int backnum = -1 ) const
    {
        char	buff[60];
        int pos = snprintf(buff, sizeof(buff), "-%.7d", num );
        if ( backnum >= 0 ) {
            snprintf( buff+pos, sizeof(buff)-pos, ".%07d.backup", backnum );
        }
        return dbPath + '/' + dbName + buff;
    }


    int CreateDataFile(void)
    {
        const std::string name = makeDataFileName(descrFile.files_count);
        smsc_log_debug(logger, "Create data file: '%s'", name.c_str());
        if (File::Exists(name.c_str())) {
            // we move the old file and create a new one
            for ( int backnum = 0;; ++backnum ) {
                const std::string backname = 
                    makeDataFileName( descrFile.files_count, backnum );
                if ( File::Exists(backname.c_str()) ) {
                    if ( backnum < 10 ) continue;
                    smsc_log_error(logger, "FSStorage: error create data file: file '%s' already exists",
                                   name.c_str());
                    throw FileException(FileException::errOpenFailed, name.c_str());
                }

                smsc_log_debug( logger, "Renaming unregistered data file %s into %s", name.c_str(), backname.c_str() );
                File::Rename( name.c_str(), backname.c_str() );
                smsc_log_info( logger, "unregisterd data file %s is renamed into %s",
                               name.c_str(), backname.c_str() );
                break;
            }
        }
	
        dataFile_f.push_back(new File());
        smsc_log_debug(logger, "Alloc: %p, %d", dataFile_f[descrFile.files_count], descrFile.files_count);
        char* emptyBlock = 0;
        index_type startBlock = descrFile.files_count * descrFile.file_size;
        try
        {
            dataFile_f[descrFile.files_count]->RWCreate(name.c_str());
            dataFile_f[descrFile.files_count]->SetUnbuffered();
            File *data_f = dataFile_f[descrFile.files_count];

            //	create list of free blocks
            emptyBlock = new char[descrFile.block_size];
            index_type* next_block = (index_type*)emptyBlock;
            index_type endBlock = (descrFile.files_count + 1) * descrFile.file_size;

            memset(emptyBlock, 0x00, descrFile.block_size);
            for(index_type i = startBlock + 1; i < endBlock; i++)
            {
                *next_block = i;
                data_f->Write(emptyBlock, descrFile.block_size);
            }
            *next_block = -1;
            data_f->Write(emptyBlock, descrFile.block_size);
            delete[] emptyBlock;
        }
        catch (const std::exception& ex)
        {
            smsc_log_debug(logger, "Error create data file. std::exception: '%s'", ex.what());
            if (emptyBlock) {
                delete[] emptyBlock;
            }
            throw;
        }
        descrFile.files_count++;
        descrFile.blocks_free = descrFile.file_size;
        descrFile.first_free_block = startBlock;
        printDescrFile();
        changeDescriptionFile();
        return 0;
    }


    void changeDescriptionFile() {
        try {
            smsc_log_debug(logger, "Change description file");
            descrFile_f.Seek(0, SEEK_SET);        
            descrFile_f.Write((char*)&descrFile, sizeof(DescriptionFile));
            printDescrFile();
            return;
        } catch (const std::exception& ex) {
            smsc_log_error(logger, "Can't write to description file. std::exception: '%s'", ex.what());
            printDescrFile();
        } 
        try {
            string name = descrFile_f.getFileName() + ".tmp";
            File tmpFile;
            tmpFile.RWCreate(name.c_str());
            tmpFile.SetUnbuffered();
            tmpFile.Seek(0, SEEK_SET);
            tmpFile.Write((char*)&descrFile, sizeof(DescriptionFile));
            smsc_log_error(logger, "Last description data saved in file '%s'", name.c_str());
        } catch (const std::exception& ex) {
            smsc_log_error(logger, "Can't save temp description file. std::exception: '%s'", ex.what());
        }
        exit(-1);
    }


    void printDescrFile()
    {
        smsc_log_debug(logger, "DescrFile: files_count=%d, block_size=%d, file_size=%d",
                       descrFile.files_count, descrFile.block_size, descrFile.file_size);
        smsc_log_debug(logger, "DescrFile: blocks_used=%d, blocks_free=%d, first_free_block=%d",
                       descrFile.blocks_used, descrFile.blocks_free, descrFile.first_free_block);
    }


    void loadBackupData() {
        try {
            dataBlockBackup.clear();
            backupFile_f.Seek(0);
            uint8_t trx = backupFile_f.ReadByte();
            if (trx == TRX_COMPLETE) {
                smsc_log_debug(logger, "Last transaction is complited");
                return;
            }
            smsc_log_warn(logger, "Last transaction is incomplited. Load transaction data");
            backupFile_f.Read((void*)&descrFile, sizeof(DescriptionFile));
            backupFile_f.Read((void*)&backupHeader, sizeof(BackupHeader));
            for (int i = 0; i < backupHeader.blocksCount; ++i) {
                index_type nextBlock;
                backupFile_f.Read((void *)&nextBlock, sizeof(nextBlock));
                dataBlockBackup.push_back(nextBlock);
            }
            if (backupHeader.dataSize > 0) {
                profileData.Empty();
                profileData.setBuffLength(backupHeader.dataSize);
                char* dataBuf = profileData.ptr();
                profileData.setLength(backupHeader.dataSize);
                backupFile_f.Read((void*)(dataBuf), backupHeader.dataSize);
            }
            restoreDataBlocks(descrFile, profileData.c_ptr());
            return;
        } catch (const std::exception& e) {
            smsc_log_error(logger, "Error loading transaction data: '%s'", e.what());
            dataBlockBackup.clear();
            throw;
        }
    }

    int OpenBackupFile() {
        string name = dbPath + '/' + dbName + ".trx";
        try {
            backupFile_f.RWOpen(name.c_str());
            backupFile_f.SetUnbuffered();
            loadBackupData();
            return 0;
        } catch (const std::exception& e) {
            smsc_log_warn(logger, "FSStorage: error open backup file: '%s'\n", e.what());
            if (File::Exists(name.c_str())) {
                smsc_log_error(logger, "FSStorage: backup file - exists, but can't be opened correct: '%s'\n", e.what());
                return CANNOT_OPEN_EXISTS_DESCR_FILE;
            } else {
                return CreateBackupFile();
            }
        }
    }

    int OpenDescriptionFile(void)
    {
        string name = dbPath + '/' + dbName;
        try
        {
            descrFile_f.RWOpen(name.c_str());
            descrFile_f.SetUnbuffered();
            descrFile_f.Read((char*)&descrFile, sizeof(DescriptionFile));
            smsc_log_debug(logger, "OpenDescrFile: files_count=%d, block_size=%d, file_size=%d, blocks_used=%d, blocks_free=%d, first_free_block=%d",
                           descrFile.files_count, descrFile.block_size, descrFile.file_size, descrFile.blocks_used, descrFile.blocks_free, descrFile.first_free_block);
            effectiveBlockSize = descrFile.block_size - sizeof(DataBlockHeader);
            return 0;
        }
        catch(const FileException& ex)
        {
            if (File::Exists(name.c_str())) {
                smsc_log_error(logger, "FSStorage: idx_file - exists, but can't be opened : %s\n", ex.what());
                return CANNOT_OPEN_EXISTS_DESCR_FILE;
            }
            smsc_log_debug(logger, "FSStorage: error idx_file - %s\n", ex.what());
            return DESCR_FILE_OPEN_FAILED;
        }
    }


    int OpenDataFiles(void)
    {
        if (descrFile.files_count <= 0) {
            smsc_log_error(logger, "Open data files error: files count=%d", descrFile.files_count);
            return CANNOT_OPEN_DATA_FILE;
        }
        char	buff[10];
        string path = dbPath + '/' + dbName;
        for(long i = 0; i < descrFile.files_count; i++)
        {
            snprintf(buff, 10, "-%.7d", i);
            string name = path + buff;
            dataFile_f.push_back(new File());
            try
            {
                smsc_log_debug(logger, "Open data file: %s", name.c_str());
                dataFile_f[i]->RWOpen(name.c_str());
                dataFile_f[i]->SetUnbuffered();
            }
            catch(const std::exception& ex)
            {
                smsc_log_error(logger, "Cannot open data file: %s", ex.what());
                return CANNOT_OPEN_DATA_FILE;
            }
        }
        return 0;
    }

    
    void restoreDataBlocks(const DescriptionFile& _descrFile, const char* data) {
        if (dataBlockBackup.size() <= 1) {
            smsc_log_warn(logger, "data blocks backup is empty");
            return;
        }
        try {
            smsc_log_warn(logger, "Restoring data storage...");
            size_t profileBlocksCount = (backupHeader.dataSize + effectiveBlockSize - 1) / effectiveBlockSize;
            index_type curBlockIndex = backupHeader.curBlockIndex;
            DataBlockHeader hdr;
            hdr.block_used = BLOCK_USED;
            hdr.key = backupHeader.key;
            hdr.head = true;
            hdr.data_size = backupHeader.dataSize;
            hdr.total_blocks = profileBlocksCount;
            int dataSize = 0;
            for (int i = 0; i < profileBlocksCount; ++i) {
                int file_number = curBlockIndex / descrFile.file_size;
                if (!checkfn(file_number)) {
                    throw smsc::util::Exception("Invalid file number %d, max file number %d", file_number, descrFile.files_count-1);
                }
                off_t offset = (curBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
                curBlockIndex = dataBlockBackup[i];
                if (i == profileBlocksCount - 1) {
                    hdr.next_block = -1;
                    dataSize = backupHeader.dataSize - effectiveBlockSize * i;
                } else {
                    hdr.next_block = curBlockIndex;
                    dataSize = effectiveBlockSize;
                }
                writeDataBlock(file_number,offset, hdr, 
                               (void*)(data + i * effectiveBlockSize), dataSize);
            }
            for (int i = profileBlocksCount; i < backupHeader.blocksCount; ++i) {
                int file_number = curBlockIndex / descrFile.file_size;
                off_t offset = (curBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
                curBlockIndex = dataBlockBackup[i];
                File* f = dataFile_f[file_number];
                f->Seek(offset);
                f->Write((void*)(&curBlockIndex), sizeof(descrFile.first_free_block));
                smsc_log_debug(logger, "restore next_free_block=%d offset=%d", curBlockIndex, offset);
            }
            clearBackup();
            smsc_log_warn(logger, "Restoring complite");
        } catch (const std::exception& e) {
            smsc_log_warn(logger, "Error restore data storage: '%s'", e.what());
            exit(-1);
        }
    }


    void writeBackup(File& f, const DescriptionFile& _descrFile, const char* backupData) {
        size_t bufSize = sizeof(TRX_INCOMPLETE) + sizeof(DescriptionFile) + sizeof(BackupHeader) +
            backupHeader.blocksCount * sizeof(descrFile.first_free_block) + backupHeader.dataSize;
        char* buf = new char[bufSize];
        try {
            memcpy(buf, &TRX_INCOMPLETE, 1);
            size_t bufOffset = 1;
            memcpy(buf + bufOffset, &_descrFile, sizeof(DescriptionFile));
            bufOffset += sizeof(DescriptionFile);
            memcpy(buf + bufOffset, &backupHeader, sizeof(BackupHeader));
            bufOffset += sizeof(BackupHeader);
            for (int i = 0; i < backupHeader.blocksCount; ++i) {
                memcpy(buf + bufOffset, (void*)(&dataBlockBackup[i]), sizeof(descrFile.first_free_block));
                bufOffset += sizeof(descrFile.first_free_block);
            }
            if (backupHeader.dataSize > 0) {
                memcpy(buf + bufOffset, backupData, backupHeader.dataSize);
            }
            f.Seek(0);
            f.Write(buf, bufSize);
            delete[] buf;
        } catch (...) {
            delete[] buf;
            throw;
        }
    }


    void saveBackupToFile(const DescriptionFile& _descrFile, const char* backupData) {
        try {
            //smsc_log_debug(logger, "Save transaction data to file");
            //printDescrFile();
            writeBackup(backupFile_f, _descrFile, backupData);
            return;
        } catch (const std::exception& e) {
            smsc_log_error(logger, "Error saving transaction data: '%s'", e.what());
        }
        try {
            string tmpName = backupFile_f.getFileName() + ".tmp";
            smsc_log_error(logger, "Trying to save transaction data to file '%s'...", tmpName.c_str());
            File tmpFile;
            tmpFile.RWCreate(tmpName.c_str());
            tmpFile.SetUnbuffered();
            writeBackup(tmpFile, _descrFile, backupData);
        } catch (const std::exception& e) {
            smsc_log_error(logger, "%s", e.what());
        }
        exit (-1);
    }


    void clearBackup() {
        try {
            smsc_log_debug(logger, "clear backup");
            backupFile_f.Seek(0);
            backupFile_f.WriteByte(TRX_COMPLETE);
            dataBlockBackup.clear();
        } catch (const std::exception& e) {
            smsc_log_error(logger, "Error clear transaction data: '%s'", e.what());
        }
    }


    index_type readFreeBlocks(int blocksCount) {
        smsc_log_debug(logger, "Read %d free blocks", blocksCount);
        index_type curBlockIndex = descrFile.first_free_block;
        for (int i = 0; i < blocksCount; ++i) {
            int file_number = curBlockIndex / descrFile.file_size;
            off_t offset = (curBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
            curBlockIndex = getFirstFreeBlock(file_number, offset);
            dataBlockBackup.push_back(curBlockIndex);
        }
        return curBlockIndex;
    }


    bool backUpProfile(const Key& key, index_type blockIndex, const BlocksHSBackupData& profile, int dataLength, bool saveToFile = false) 
    {
        smsc_log_debug(logger, "backup profile key=%s block index=%d, data length=%d",
                       key.toString().c_str(), blockIndex, dataLength);
        dataBlockBackup.clear();
        int blocksCount = (dataLength + effectiveBlockSize - 1) / effectiveBlockSize;
        if (blockIndex == -1 && blocksCount == 0) {
            return true;
        }
        DescriptionFile oldDescrFile = descrFile;
        printDescrFile();
        try {
            if (blockIndex < 0) {
                backupHeader.blocksCount = blocksCount;
                backupHeader.dataSize = 0;
                backupHeader.curBlockIndex = oldDescrFile.first_free_block;
                index_type curBlockIndex = readFreeBlocks(blocksCount);
                if (saveToFile) {
                    saveBackupToFile(oldDescrFile, NULL ); // profile.getBackupData());
                }
                smsc_log_debug(logger, "backup profile size=%d", dataBlockBackup.size());
                return true;
            }
            backupHeader.curBlockIndex = blockIndex;
            backupHeader.dataSize = profile.getBackupDataSize(); 
            dataBlockBackup = profile.getBackup();
            size_t backupSize = dataBlockBackup.size();
            backupHeader.blocksCount = blocksCount >= backupSize ? blocksCount : backupSize;
            if (backupSize == 0) {
                smsc_log_warn(logger, "Error backup profile: key=%s backup is empty", key.toString().c_str() );
                return false;
            }
            if (backupSize < blocksCount) {
                dataBlockBackup[backupSize - 1] = oldDescrFile.first_free_block;
                readFreeBlocks(blocksCount - backupSize);
            }
            smsc_log_debug(logger, "backup profile key=%s size=%d", key.toString().c_str(), dataBlockBackup.size());
            if (saveToFile) {
                saveBackupToFile(oldDescrFile, profile.getBackupData());
            }
            return true;
        } catch (const std::exception& e) {
            smsc_log_warn(logger, "Error backup profile key=%s. std::exception: '%s'",
                          key.toString().c_str(), e.what());
            dataBlockBackup.clear();
            descrFile = oldDescrFile;
            changeDescriptionFile();
            exit(-1);
        }
    }


    /// check file number
    inline bool checkfn( int fn ) const {
        if ( fn < descrFile.files_count ) return true;
        smsc_log_error(logger, "Invalid file number %d, max file number %d", fn, descrFile.files_count-1 );
        return false;
    }
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE_H */
