//------------------------------------
//  BlocksHSStorage.h
//  Odarchenko Vitaly, 2008
//------------------------------------
//

#include <string>
#include <vector>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/mman.h"

#include "core/buffers/File.hpp"

#include "DataBlock.h"
#include "SerialBuffer.h"
#include "Profile.h"

#include <cerrno>

#ifndef ___BLOCKS_HS_STORAGE_H
#define ___BLOCKS_HS_STORAGE_H

using std::vector;
using std::string;
using smsc::logger::Logger;
using scag::pers::Profile;

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
    char preamble[20];
    int version;

    int files_count;
    int block_size;         // in bytes;
    int file_size;          // in blocks
    int blocks_used;
    int blocks_free;
    long first_free_block;
    char Reserved[12];
};

template<class Key>
struct templBackupHeader {
  long blocksCount;
  long dataSize;
  long curBlockIndex;
  Key key;
};

template<class Key>
struct templDataBlockHeader
{
    union
    {
        long next_free_block;
        long block_used;
    };
    Key     key;
    long    total_blocks;
    long    data_size;
    long    next_block;
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


template<class Key>
class BlocksHSStorage
{
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
	static const long BLOCK_USED	= long(1) << 63;

	BlocksHSStorage():running(false), iterBlockIndex(0)
	{
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

    bool Add(Profile& profile, const Key& key, long& blockIndex) {
      //SerialBuffer data;
      profileData.Empty();
      profile.Serialize(profileData, true);
      smsc_log_debug(logger, "Add data block key='%s' length=%d", key.toString().c_str(), profileData.length());
      int dataLength = profileData.length();
      if (dataLength <= 0) {
        return false;
      }
      long ffb = descrFile.first_free_block;
      DescriptionFile oldDescrFile = descrFile;
      backupHeader.key = key;
      if (!backUpProfile(-1, profile, dataLength, true)) {
        blockIndex = -1;
        smsc_log_error(logger, "Backup profile error");
        return false;
      }
      try {
        descrFile.first_free_block = addBlocks(ffb, 0, backupHeader.blocksCount, profileData,
                                               key, profile);
        changeDescriptionFile();
        clearBackup();
        blockIndex = ffb;
        profile.setBackupData(profileData);
        return true;
      } catch (const std::exception& e) {
        blockIndex = -1;
        restoreDataBlocks(oldDescrFile, profile.getBackupData());
        profile.Empty();
        descrFile = oldDescrFile;
        changeDescriptionFile();
        //exit(-1);
        throw;
      }
    }

    
    bool Change(Profile& profile, const Key& key, long& blockIndex) {
      if (blockIndex == -1) {
        return Add(profile, key, blockIndex);
      }
      profileData.Empty();
      profile.Serialize(profileData, true);
      smsc_log_debug(logger, "Change data block index=%d key='%s' length=%d",
                      blockIndex, key.toString().c_str(), profileData.length());
      DescriptionFile oldDescrFile = descrFile;
      long ffb = descrFile.first_free_block;
      backupHeader.key = key;
      int dataLength = profileData.length();
      if (!backUpProfile(blockIndex, profile, dataLength, true)) {
        smsc_log_error(logger, "Backup profile error");
        return false;
      }
      int blocksCount = (dataLength + effectiveBlockSize - 1) / effectiveBlockSize;
      int oldBlocksCount = (backupHeader.dataSize + effectiveBlockSize - 1) / effectiveBlockSize;
      int updateBlocksCount = blocksCount <= oldBlocksCount ? blocksCount : oldBlocksCount;

      try {
        profile.clearBackup();
        long curBlockIndex = addBlocks(blockIndex, 0, updateBlocksCount, profileData, key, profile); 
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
    
	bool Get(long blockIndex, DataBlock& data)
	{
        smsc_log_debug(logger, "Get data block index=%d ", blockIndex);
		if(blockIndex == -1) return false;
		char* buff;
		long curBlockIndex = blockIndex;
		int i = 0;
		do
		{
			DataBlockHeader hdr;

			int file_number = curBlockIndex / descrFile.file_size;
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

    bool Get(long blockIndex, Profile& profile)
    {
        smsc_log_debug(logger, "Get data block index=%d ", blockIndex);
        if(blockIndex == -1) return false;
        char* buff;
        long curBlockIndex = blockIndex;
        int i = 0;
        //SerialBuffer data;
        profileData.Empty();
        do
        {
            DataBlockHeader hdr;
            int file_number = curBlockIndex / descrFile.file_size;
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
            profile.addDataToBackup(hdr.next_block);
        }
        while(-1 != curBlockIndex);
        profile.Deserialize(profileData, true);
        profile.setBackupData(profileData);
        return true;
    }

    void Remove(long blockIndex, const Profile& profile) {
      smsc_log_debug(logger, "Remove data block index=%d ", blockIndex);
      if (blockIndex < 0) {
        smsc_log_warn(logger, "Can't remove data block index=%d ", blockIndex);
        return;
      }
      DescriptionFile oldDescrFile = descrFile;
      long ffb = descrFile.first_free_block;
      if (!backUpProfile(blockIndex, profile, 0, true)) {
        return;
      }
      try {
        descrFile.first_free_block = removeBlocks(ffb, blockIndex, 0);
        changeDescriptionFile();
        clearBackup();
      } catch (const std::exception& e) {
        restoreDataBlocks(oldDescrFile, profile.getBackupData());
        descrFile = oldDescrFile;
        changeDescriptionFile();
        exit(-1);
      }
    }


    void Reset()
    {
        iterBlockIndex = 0;
    }
    
    bool Next(long& blockIndex, DataBlock& data, Key& key)
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
  bool running;
  string dbName;
  string dbPath;
  DescriptionFile descrFile;		
  File descrFile_f;
  File backupFile_f;
  vector<File*> dataFile_f;
  long iterBlockIndex;
  long effectiveBlockSize;	

  vector<long> dataBlockBackup;
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

    long addBlocks(long curBlockIndex, size_t backupStartIndex, size_t blocksCount,
                   const DataBlock& data, const Key& key, Profile& profile) {
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
        profile.addDataToBackup(hdr.next_block);
        hdr.head = false;
      }
      smsc_log_debug(logger, "addBlocks curBlockIndex=%d", curBlockIndex);
      return curBlockIndex;
    }

    long getFirstFreeBlock(int fileNumber, off_t offset)
    {
      File* f = dataFile_f[fileNumber];
      f->Seek(offset, SEEK_SET);
      long ffb = 0;
      f->Read((void*)&(ffb), sizeof(descrFile.first_free_block));
      if (ffb == -1) {
        ffb = descrFile.files_count * descrFile.file_size;
        CreateDataFile();
      }
      smsc_log_debug(logger, "First free block %d", ffb);
      return ffb;
    }

    long removeBlocks(long ffb, long blockIndex, size_t backupIndex) {
      smsc_log_debug(logger, "Remove %d blocks start block index=%d", 
                     backupHeader.blocksCount - backupIndex, blockIndex);
      if (backupIndex >= backupHeader.blocksCount) {
        smsc_log_warn(logger, "Can't remove unbackuped data blocks");
        return ffb;
      }
      while (blockIndex != -1) {
        smsc_log_debug(logger, "Remove %d block, ffb=%d", blockIndex, ffb); 
        int file_number = blockIndex / descrFile.file_size;
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
        effectiveBlockSize = descrFile.block_size - sizeof(DataBlockHeader);
        changeDescriptionFile();
        return 0;
    }

    int CreateDataFile(void)
	{
		char	buff[16];
		snprintf(buff, 16, "-%.7d", descrFile.files_count);
		string name = dbPath + '/' + dbName + buff;
        smsc_log_debug(logger, "Create data file: '%s'", name.c_str());
        if (File::Exists(name.c_str())) {
          smsc_log_error(logger, "FSStorage: error create data file: file '%s' already exists",
                          name.c_str());
          throw FileException(FileException::errOpenFailed, name.c_str());
        }
	
		dataFile_f.push_back(new File());
		smsc_log_debug(logger, "Alloc: %p, %d", dataFile_f[descrFile.files_count], descrFile.files_count);
        char* emptyBlock = 0;
        long startBlock = descrFile.files_count * descrFile.file_size;
		try
		{
			dataFile_f[descrFile.files_count]->RWCreate(name.c_str());
			dataFile_f[descrFile.files_count]->SetUnbuffered();
            File *data_f = dataFile_f[descrFile.files_count];

            //	create list of free blocks
            emptyBlock = new char[descrFile.block_size];
            long* next_block = (long*)emptyBlock;
            long endBlock = (descrFile.files_count + 1) * descrFile.file_size;

            memset(emptyBlock, 0x00, descrFile.block_size);
            for(long i = startBlock + 1; i < endBlock; i++)
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
          long nextBlock;
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
        long curBlockIndex = backupHeader.curBlockIndex;
        DataBlockHeader hdr;
        hdr.block_used = BLOCK_USED;
        hdr.key = backupHeader.key;
        hdr.head = true;
        hdr.data_size = backupHeader.dataSize;
        hdr.total_blocks = profileBlocksCount;
        int dataSize = 0;
        for (int i = 0; i < profileBlocksCount; ++i) {
          int file_number = curBlockIndex / descrFile.file_size;
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
          memcpy(buf + bufOffset, (void*)backupData, backupHeader.dataSize);
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

    long readFreeBlocks(int blocksCount) {
      smsc_log_debug(logger, "Read %d free blocks", blocksCount);
      long curBlockIndex = descrFile.first_free_block;
      for (int i = 0; i < blocksCount; ++i) {
        int file_number = curBlockIndex / descrFile.file_size;
        off_t offset = (curBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
        curBlockIndex = getFirstFreeBlock(file_number, offset);
        dataBlockBackup.push_back(curBlockIndex);
      }
      return curBlockIndex;
    }


    bool backUpProfile(long blockIndex, const Profile& profile, int dataLength, bool saveToFile = false) {
      smsc_log_debug(logger, "Backup profile key='%s'. block index=%d, data length=%d",
                     profile.getKey().c_str(), blockIndex, dataLength);
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
          long curBlockIndex = readFreeBlocks(blocksCount);
          if (saveToFile) {
            saveBackupToFile(oldDescrFile, profile.getBackupData());
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
          smsc_log_warn(logger, "Error backup profile key='%s': backup is empty", profile.getKey().c_str());
          return false;
        }
        if (backupSize < blocksCount) {
          dataBlockBackup[backupSize - 1] = oldDescrFile.first_free_block;
          readFreeBlocks(blocksCount - backupSize);
        }
        smsc_log_debug(logger, "backup profile size=%d", dataBlockBackup.size());
        if (saveToFile) {
          saveBackupToFile(oldDescrFile, profile.getBackupData());
        }
        return true;
      } catch (const std::exception& e) {
        smsc_log_warn(logger, "Error backup profile key='%s'. std::exception: '%s'",
                       profile.getKey().c_str(), e.what());
        dataBlockBackup.clear();
        descrFile = oldDescrFile;
        changeDescriptionFile();
        exit(-1);
      }
    }

};

#endif

/*
    bool Change(const DataBlock& data, const Key& key, long& blockIndex)
	{
        smsc_log_debug(logger, "Change data block index=%d key='%s' length=%d",
                        blockIndex, key.toString().c_str(), data.length());
		//if (blockIndex == -1) {
          //return _Add(data, key, blockIndex);
        //}
        //return _Change(data, key, blockIndex);

		DataBlockHeader hdr;
		long curBlockIndex = blockIndex;
		long total_blocks = (data.length() > 0) ? (data.length() + effectiveBlockSize - 1) / effectiveBlockSize : 0;
		long old_total_blocks = 2;
		long data_size = data.length();
		long next_block = blockIndex;
		size_t curBlockSize = effectiveBlockSize;

        try {
          for(int i = 0; i < total_blocks; i++)
          {
              int file_number = curBlockIndex / descrFile.file_size;
              off_t offset = (curBlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
              long ffb = 0;
              if(i < old_total_blocks)
              {
                  File* f = dataFile_f[file_number];
                  f->Seek(offset, SEEK_SET);
                  f->Read((void*)&hdr, sizeof(DataBlockHeader));
                  if( (hdr.block_used != BLOCK_USED) || (hdr.key != key) )
                      return false;
                  if(i == 0) old_total_blocks = hdr.total_blocks;
                  hdr.total_blocks = total_blocks;
                  hdr.data_size = data_size;
                  next_block = hdr.next_block;
              }
              if( (i >= old_total_blocks - 1) && (i != total_blocks - 1) )
              {
                  hdr.next_block = descrFile.first_free_block;
                  int fn = hdr.next_block / descrFile.file_size;
                  off_t ofs = (hdr.next_block - fn * descrFile.file_size)*descrFile.block_size;
                  ffb = getFirstFreeBlock(fn, ofs);
                  if (!ffb) {
                    smsc_log_warn(logger, "Error first free block index %d", ffb);
                    return false;
                  }
              }
              if (i == total_blocks - 1)
              {
                  hdr.next_block = -1;
                  curBlockSize = data_size - effectiveBlockSize * i;
              }
              writeDataBlock(file_number, offset, hdr,
                              (void *)(data.c_ptr()+i*(effectiveBlockSize)), curBlockSize);
              descrFile.first_free_block = ffb > 0 ? ffb : descrFile.first_free_block;
              curBlockIndex = hdr.next_block;
          }
        } catch (const std::exception& e) {
          smsc_log_warn(logger, "Error changing data block %d. std::exception: '%s'", blockIndex);
          changeDescriptionFile();
          exit(-1);
        }
        if (next_block != -1) {
          Remove(next_block);
          return true;
        }
        changeDescriptionFile();
		return true;
	}
*/

        /*
        long curBlockIndex = blockIndex;
        int i = 0;
        do {
          int fileNumber = curBlockIndex / descrFile.file_size;
          off_t offset = (curBlockIndex - fileNumber * descrFile.file_size) * descrFile.block_size;
          smsc_log_debug(logger, "backup profile fn=%d offset=%d", fileNumber, offset);
          File *f = dataFile_f[fileNumber];
          f->Seek(offset);
          CompleteDataBlock dataBlock;
          f->Read(&dataBlock.hdr, sizeof(DataBlockHeader));
          if (dataBlock.hdr.block_used != BLOCK_USED) {
            smsc_log_error(logger, "Error backup: block %d unused", blockIndex);
            //TODO this is error!!!
            dataBlockBackup.clear();
            descrFile = oldDescrFile;
            changeDescriptionFile();
            return false;
          }
          size_t dataSize = dataBlock.hdr.next_block == -1 ?
                            dataBlock.hdr.data_size - effectiveBlockSize * i : effectiveBlockSize;
          dataBlock.data.setBuffLength(dataSize);
          char* dataBuf = dataBlock.data.ptr();
          dataBlock.data.setLength(dataSize);
          f->Read((void*)(dataBuf), dataSize);
          dataBlockBackup.push_back(dataBlock);
          curBlockIndex = dataBlock.hdr.next_block;
          ++i;
        } while (curBlockIndex != -1);
        */
    /*
	void Remove(long blockIndex)
	{
        smsc_log_debug(logger, "Remove data block index=%d ", blockIndex);
        //_Remove(blockIndex);
        //return;
		long next_block = blockIndex;
		DataBlockHeader hdr;
        try {
          while(-1 != next_block)
          {
              int file_number = next_block / descrFile.file_size;
              off_t offset = (next_block - file_number * descrFile.file_size) * descrFile.block_size;
              File* f = dataFile_f[file_number];
              f->Seek(offset, SEEK_SET);
              f->Read((void*)&hdr, sizeof(DataBlockHeader));
              f->Seek(offset, SEEK_SET);
              if ((hdr.block_used != BLOCK_USED))
                  break;
              f->Write((void*)&(descrFile.first_free_block), sizeof(descrFile.first_free_block));
              descrFile.first_free_block = next_block;
              next_block = hdr.next_block;
          }
          changeDescriptionFile();
        } catch (const std::exception& e) {
          smsc_log_warn(logger, "Error removing data block %d. std::exception: '%s'",
                         blockIndex, e.what());
          changeDescriptionFile();
          exit(-1);
        }
	}*/
/*
	bool Add(const DataBlock& data, const Key& key, long& blockIndex)
	{
      smsc_log_debug(logger, "Add data block key='%s' length=%d", key.toString().c_str(), data.length());
      DataBlockHeader hdr;
      hdr.block_used = BLOCK_USED;
      hdr.key = key;
      hdr.head = true;
      hdr.total_blocks = (data.length() + effectiveBlockSize - 1) / effectiveBlockSize;
      hdr.data_size = data.length();//effectiveBlockSize;

      try {
        for(int i = 0; i < hdr.total_blocks; i++)
        {
            long curBlockIndex = descrFile.first_free_block;
            int file_number = curBlockIndex / descrFile.file_size;
            off_t offset = (curBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
            long ffb = getFirstFreeBlock(file_number, offset);
            if (!ffb) {
              smsc_log_warn(logger, "Error first free block index %d", ffb);
              return false;
            }
            size_t curBlockSize = (i == hdr.total_blocks - 1) ?
                                   (data.length() - effectiveBlockSize * i) : effectiveBlockSize;
            hdr.next_block = (i < hdr.total_blocks - 1) ? ffb : -1;
            writeDataBlock(file_number, offset, hdr,
                           (void*)(data.c_ptr() + i * (effectiveBlockSize)), curBlockSize);
            descrFile.first_free_block = ffb;
            if (i == 0) blockIndex = curBlockIndex;
        }
        changeDescriptionFile();
        smsc_log_debug(logger, "Data block key='%s' added, index=%d", key.toString().c_str(), blockIndex);
        return true;
      } catch (const std::exception& e) {
        smsc_log_warn(logger, "Error adding data block. std::exception: '%s'", e.what());
        changeDescriptionFile();
        exit(-1);
      }
	}
*/

