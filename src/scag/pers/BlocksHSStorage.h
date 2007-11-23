//------------------------------------
//  BlocksHSStorage.h
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ BlocksHSStorage.
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

extern int errno;

#ifndef ___BLOCKS_HS_STORAGE_H
#define ___BLOCKS_HS_STORAGE_H

using namespace std;

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
//const long long BLOCK_USED   = static_cast<long long>(1) << 63;

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
class BlocksHSStorage
{
	typedef templDataBlockHeader<Key> DataBlockHeader;

public:
//	static const int SUCCESS			= 0;
//	static const int RBTREE_FILE_NOT_SPECIFIED	= -1;
	static const int CANNOT_CREATE_DESCR_FILE	= -2;
	static const int CANNOT_CREATE_DATA_FILE	= -3;
	static const int CANNOT_OPEN_DESCR_FILE	= -4;
	static const int CANNOT_OPEN_DATA_FILE		= -5;
	static const int DESCR_FILE_CREATE_FAILED		= -6;
	static const int DESCR_FILE_OPEN_FAILED		= -7;
	//static const int RBTREE_FILE_ERROR		= -10;

	static const int defaultBlockSize = 56; // in bytes
	static const int defaultFileSize = 3; // in blocks 
	static const long BLOCK_USED	= (long)1 << 63;
    //static const long BLOCK_USED	= (long)1 << 10;

	BlocksHSStorage():running(false), iterBlockIndex(0)
	{
        logger = smsc::logger::Logger::getInstance("BlkStore");
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
		
		int ret;
		if(0 != (ret = CreateDescriptionFile(_blockSize + sizeof(DataBlockHeader), _fileSize)))
			return ret;
//		for(int i=0; i<10; i++)
		if(0 != (ret = CreateDataFile()))
			return ret;
		
		running = true;
		//printf("Create Blocks File Storage\n");
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
		running = true;
		return 0;
	}
	
	void Close(void)
	{
		if(running)
		{
			for(int i = 0; i < dataFile_f.size(); i++)
			{
//				printf("Free: %p, %d\n", dataFile_f[i], i);			
				dataFile_f[i]->Close();
				delete dataFile_f[i];
			}
			dataFile_f.clear();
			descrFile_f.Close();			
			running = false;
		}
	
	}

	int Add(const DataBlock& data, long& blockIndex, Key key)
	{
		DataBlockHeader hdr;
		long effectiveBlockSize = descrFile.block_size - sizeof(DataBlockHeader);
		
		hdr.block_used = BLOCK_USED;
		hdr.key = key;
        hdr.head = true;
		hdr.total_blocks = (data.length() > 0) ? (data.length() + effectiveBlockSize - 1) / effectiveBlockSize : 1;
		hdr.data_size = data.length();//effectiveBlockSize;

//        smsc_log_debug(logger, "data_len=%d, block_used=%d, key=%s, total_blocks=%d", data.length(), hdr.block_used, hdr.key.getText().c_str(), hdr.total_blocks);
//		printf("data.length() = %d\n", data.length());
//		printf("sizeof(DataBlockHeader) = %d\n", sizeof(DataBlockHeader));
//		printf("hdr.block_used = 0x%X\n", hdr.block_used);
//		printf("hdr.key = %s\n", hdr.key.getText().c_str());
//		printf("hdr.total_blocks = %d\n", hdr.total_blocks);
//		printf("FilePos = %d\n", hdr.total_blocks);
		
		for(int i = 0; i < hdr.total_blocks; i++)
		{
			long curBlockIndex = descrFile.first_free_block;
			int file_number = curBlockIndex / descrFile.file_size;
			off_t offset = (curBlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
//			int fd = dataFile_fd[file_number];
//            smsc_log_debug(logger, "ffb=%d, file_size=%d, fn=%d", descrFile.first_free_block, descrFile.file_size, file_number);
			File* f = dataFile_f[file_number];
			f->Seek(offset, SEEK_SET);
			f->Read((void*)&(descrFile.first_free_block), sizeof(descrFile.first_free_block));
//            smsc_log_debug(logger, "ffb=%d offset=%d", descrFile.first_free_block, offset);
			if(descrFile.first_free_block == -1)
			{
				int ret;
				if(0 != (ret = CreateDataFile()))
					return ret;
			}
			size_t curBlockSize = (i == hdr.total_blocks - 1) ? data.length() - effectiveBlockSize*i:effectiveBlockSize;
			hdr.next_block = (i < hdr.total_blocks - 1) ? descrFile.first_free_block : -1;
			f->Seek(offset, SEEK_SET);
			f->Write((void*)&hdr, sizeof(DataBlockHeader));
			f->Write((void*)(data.c_ptr()+i*(effectiveBlockSize)), curBlockSize);
			if(i == 0) blockIndex = curBlockIndex;
//            smsc_log_debug(logger, "i=%d block_idx=%d", i, blockIndex);
//			printf("hdr.next_block = %d\n", hdr.next_block);
		}
		descrFile_f.Seek(0, SEEK_SET);
		descrFile_f.Write((char*)&descrFile, sizeof(DescriptionFile));
		return 0;
	}

// 	int Change(long blockIndex, const char* buff)
// 	{
// 		int file_number = blockIndex / descrFile.file_size;
// 		off_t offset = (blockIndex - file_number * descrFile.file_size)*descrFile.block_size;
// 		int fd = dataFile_fd[file_number];
// 		lseek(fd, offset, SEEK_SET);
// 		write(fd, buff, descrFile.block_size);
// 		return 	descrFile.block_size;
// 	}
	bool Change(long blockIndex, const DataBlock& data, Key key)
	{
		if(blockIndex == -1) return false;

		DataBlockHeader hdr;
		long effectiveBlockSize = descrFile.block_size - sizeof(DataBlockHeader);
		long curBlockIndex = blockIndex;
		long total_blocks = (data.length() > 0) ? (data.length() + effectiveBlockSize - 1) / effectiveBlockSize : 1;
		long old_total_blocks = 2;
		long data_size = data.length();
		long next_block;
		size_t curBlockSize = effectiveBlockSize;
		
		for(int i = 0; i < total_blocks; i++)
		{
//			if(!readDataHeader(curBlockIndex, hdr))
//				return false;
			int file_number = curBlockIndex / descrFile.file_size;
			off_t offset = (curBlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
			File* f = dataFile_f[file_number];
//			pritnf("1\n");
			f->Seek(offset, SEEK_SET);

			if(i < old_total_blocks)
			{
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
				//printf("i = %d, old_total_blocks = %d\n", i, old_total_blocks);
				hdr.next_block = descrFile.first_free_block;
				int fn = hdr.next_block / descrFile.file_size;
				off_t ofs = (hdr.next_block - fn * descrFile.file_size)*descrFile.block_size;
				File* f1 = dataFile_f[fn];
				f1->Seek(ofs, SEEK_SET);
//				pritnf("2\n");
				f1->Read((void*)&(descrFile.first_free_block), sizeof(descrFile.first_free_block));
				if(descrFile.first_free_block == -1)
					if(0 != CreateDataFile()) return false;
				//printf("hdr.next_block = %d\n", hdr.next_block);
			}
			if (i == total_blocks - 1)
			{
				hdr.next_block = -1;
				curBlockSize = data_size - effectiveBlockSize*i;
			}
			//printf("offset = %d\n", offset);
			f->Seek(offset, SEEK_SET);
			f->Write((void*)&hdr, sizeof(DataBlockHeader));
			f->Write(data.c_ptr()+i*(effectiveBlockSize), curBlockSize);
			curBlockIndex = hdr.next_block;
		}
		//printf("!next_block = %d\n", next_block);
		while(-1 != next_block)
		{
			int file_number = next_block / descrFile.file_size;
			off_t offset = (next_block - file_number * descrFile.file_size)*descrFile.block_size;
			File* f = dataFile_f[file_number];
			f->Seek(offset, SEEK_SET);
//			pritnf("3\n");
			f->Read((void*)&hdr, sizeof(DataBlockHeader));
			f->Seek(offset, SEEK_SET);
//			pritnf("4\n");
			if( hdr.block_used != BLOCK_USED)
				return false;
			f->Write((void*)&(descrFile.first_free_block), sizeof(descrFile.first_free_block));
			descrFile.first_free_block = next_block;
			next_block = hdr.next_block;
		}
		descrFile_f.Seek(0, SEEK_SET);
		descrFile_f.Write((char*)&descrFile, sizeof(DescriptionFile));
		return true;
	}

	bool Get(long blockIndex, DataBlock& data)
	{
		if(blockIndex == -1) return false;
		long effectiveBlockSize = descrFile.block_size - sizeof(DataBlockHeader);
		char* buff;
		long curBlockIndex = blockIndex;
		int i=0;
		do
		{
			DataBlockHeader hdr;

			int file_number = curBlockIndex / descrFile.file_size;
			off_t offset = (curBlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
			File* f = dataFile_f[file_number];
			f->Seek(offset, SEEK_SET);
			f->Read((void*)&hdr, sizeof(DataBlockHeader));
			if(hdr.block_used != BLOCK_USED)
				return false;
			if(curBlockIndex == blockIndex)
			{
//				//printf("hdr.data_size = %d\n", hdr.data_size);
				//if(hdr.data_size <= 0)
				//{
				//	printf("1\n");
				//	return true;
				//}
				data.setBuffLength(hdr.data_size);
				buff = data.ptr();
				data.setLength(hdr.data_size);
			}
			if(-1 == hdr.next_block)
				f->Read((void*)(buff+(i*effectiveBlockSize)), hdr.data_size - effectiveBlockSize*i);
			else
				f->Read((void*)(buff+(i*effectiveBlockSize)), effectiveBlockSize);
			
			//printf("hdr.block_used = 0x%X\n", hdr.block_used);
			//printf("hdr.key = %s\n", hdr.key.getText().c_str());
			//printf("hdr.total_blocks = %d\n", hdr.total_blocks);
			//printf("hdr.next_block = %d\n", hdr.next_block);
			
			curBlockIndex = hdr.next_block;
			i++;
		}
		while(-1 != curBlockIndex);
		return true;
	}

	void Remove(long blockIndex)
	{
		long next_block = blockIndex;
		DataBlockHeader hdr;
		
		while(-1 != next_block)
		{
			int file_number = next_block / descrFile.file_size;
			off_t offset = (next_block - file_number * descrFile.file_size)*descrFile.block_size;
			File* f = dataFile_f[file_number];
			f->Seek(offset, SEEK_SET);
			f->Read((void*)&hdr, sizeof(DataBlockHeader));
			f->Seek(offset, SEEK_SET);
			if( (hdr.block_used != BLOCK_USED))
				return;
			f->Write((void*)&(descrFile.first_free_block), sizeof(descrFile.first_free_block));
			descrFile.first_free_block = next_block;
			next_block = hdr.next_block;
		}
		descrFile_f.Seek(0, SEEK_SET);
		descrFile_f.Write((char*)&descrFile, sizeof(DescriptionFile));
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
			f->Read((void*)&hdr, sizeof(DataBlockHeader));
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
    smsc::logger::Logger* logger;
	bool				running;
	string				dbName;
	string				dbPath;
	DescriptionFile			descrFile;		
	File				descrFile_f;
	vector<File*>			dataFile_f;
    long                iterBlockIndex;
	
	//bool isFileExists(void)
	//{
	//	struct ::stat st;
	//	return ::stat(rbtree_file.c_str(),&st)==0;
	//}

	int CreateDescriptionFile(long _blockSize, long _fileSize)
	{
		try
		{
			descrFile_f.RWCreate((dbPath + '/' + dbName).c_str());
			descrFile_f.SetUnbuffered();
		}
		catch(FileException ex)
		{
		    smsc_log_error(logger, "FSStorage: error idx_file - %s\n", ex.what());
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
        
		descrFile_f.Seek(0, SEEK_SET);
		descrFile_f.Write((char*)&descrFile, sizeof(DescriptionFile));
		return 0;
	}
	int CreateDataFile(void)
	{
		char	buff[16];
		snprintf(buff, 16, "-%.7d", descrFile.files_count);
		string name = dbPath + '/' + dbName + buff;
	
		dataFile_f.push_back(new File());
//		smsc_log_debug(logger, "Alloc: %p, %d", dataFile_f[descrFile.files_count], descrFile.files_count);
		try
		{
			dataFile_f[descrFile.files_count]->RWCreate(name.c_str());
			dataFile_f[descrFile.files_count]->SetUnbuffered();
		}
		catch(FileException ex)
		{
		    smsc_log_debug(logger, "FSStorage: error data_file - %s\n", ex.what());
			return CANNOT_CREATE_DATA_FILE;
		}
		
		File *data_f = dataFile_f[descrFile.files_count];

		//	create list of free blocks
		char*	emptyBlock = new char[descrFile.block_size];
		long*	next_block = (long*)emptyBlock;
		long	startBlock = descrFile.files_count * descrFile.file_size;
		long	endBlock = (descrFile.files_count + 1) * descrFile.file_size;
		
		memset(emptyBlock, 0x00, descrFile.block_size);
		for(long i = startBlock + 1; i < endBlock; i++)
		{
			*next_block = i;
			data_f->Write(emptyBlock, descrFile.block_size);
		}
		*next_block = -1;
		data_f->Write(emptyBlock, descrFile.block_size);

		delete[] emptyBlock;
		
		//	change description file.
		descrFile.files_count++;
		descrFile.blocks_free = descrFile.file_size;
		descrFile.first_free_block = startBlock;
		descrFile_f.Seek(0, SEEK_SET);        
		descrFile_f.Write((char*)&descrFile, sizeof(DescriptionFile));
		return 0;
	}

	int OpenDescriptionFile(void)
	{
		try
		{
			descrFile_f.RWOpen((dbPath + '/' + dbName).c_str());
			descrFile_f.SetUnbuffered();
		}
		catch(FileException ex)
		{
		    smsc_log_debug(logger, "FSStorage: error idx_file - %s\n", ex.what());
			return DESCR_FILE_OPEN_FAILED;
		}
		descrFile_f.Read((char*)&descrFile, sizeof(DescriptionFile));
        smsc_log_debug(logger, "OpenDescrFile: files_count=%d, block_size=%d, file_size=%d, blocks_used=%d, blocks_free=%d, first_free_block=%d",
            descrFile.files_count, descrFile.block_size, descrFile.file_size, descrFile.blocks_used, descrFile.blocks_free, descrFile.first_free_block);
		return 0;
	}
	int OpenDataFiles(void)
	{
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
			catch(FileException ex)
			{
                smsc_log_error(logger, "Cannot open data file: %s", ex.what());
				return CANNOT_OPEN_DATA_FILE;
			}
		}
		return 0;
	}

//inline
//	bool readDataHeader(long blockIndex, DataHeader& hdr)
//	{
//		int file_number = BlockIndex / descrFile.file_size;
//		off_t offset = (BlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
//		int fd = dataFile_fd[file_number];
//		lseek(fd, offset, SEEK_SET);
//		read(fd, (void*)&hdr, sizeof(DataBlockHeader));
//	}
//	bool writeDataHeader(long blockIndex, DataHeader& hdr)
//	{
//		int file_number = BlockIndex / descrFile.file_size;
//		off_t offset = (BlockIndex - f	ile_number * descrFile.file_size)*descrFile.block_size;
//		int fd = dataFile_fd[file_number];
//		lseek(fd, offset, SEEK_SET);
//		read(fd, (void*)&hdr, sizeof(DataBlockHeader));
//	}
//	bool readData(long blockIndex, DataHeader& hdr)
//	{
//		int file_number = BlockIndex / descrFile.file_size;
//		off_t offset = (BlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
//		int fd = dataFile_fd[file_number];
//		lseek(fd, offset, SEEK_SET);
//		read(fd, (void*)&hdr, sizeof(DataBlockHeader));
//	}
//	bool writeData(long blockIndex, DataHeader& hdr)
//	{
//		int file_number = BlockIndex / descrFile.file_size;
//		off_t offset = (BlockIndex - file_number * descrFile.file_size)*descrFile.block_size;
//		int fd = dataFile_fd[file_number];
//		lseek(fd, offset, SEEK_SET);
//		read(fd, (void*)&hdr, sizeof(DataBlockHeader));
//	}

};


#endif
