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
#include "scag/util/io/GlossaryBase.h"
#include "DataBlockBackup.h"
#include "util/Exception.hpp"
#include "util/Uint64Converter.h"
#include "DataFileCreator.h"
#include "FixupLogger.h"
#include "scag/util/io/EndianConverter.h"


namespace scag {
namespace util {
namespace storage {

using std::vector;
using std::string;
using smsc::logger::Logger;
using smsc::util::Uint64Converter;

const string dfPreambule= "RBTREE_FILE_STORAGE!";
const int dfVersion_32_1 = 0x01;
const int32_t dfVersion_64_1 = 0x80000001; // pers 
const int32_t dfVersion_64_2 = 0x80000002; // PVSS 2.0
const uint32_t PREAMBULE_SIZE = 20;
const uint32_t RESERVED_SIZE = 12;

const int dfMode = 0600;

const int CANNOT_CREATE_DESCR_FILE  = -2;
const int CANNOT_CREATE_DATA_FILE   = -3;
const int CANNOT_OPEN_DESCR_FILE    = -4;
const int CANNOT_OPEN_DATA_FILE     = -5;

static const uint8_t TRX_COMPLETE   = 0;
static const uint8_t TRX_INCOMPLETE = 1;

static const size_t WRITE_BUF_SIZE = 10240;
static const size_t BACKUP_BUF_SIZE = 10240;
static const size_t PROFILE_MAX_BLOCKS_COUNT = 10;
static const size_t MIN_BLOCK_SIZE = 10;

struct DescriptionFile   
{
private:
    static uint32_t size_;
    
public:
    DescriptionFile(): version(dfVersion_64_2), files_count(0), block_size(0), file_size(0), blocks_used(0), blocks_free(0), first_free_block(0) {
      memcpy(preamble, dfPreambule.c_str(), PREAMBULE_SIZE);
      memset(Reserved, 0, RESERVED_SIZE);
    }
    DescriptionFile& operator=(const DescriptionFile& descr) {
      files_count = descr.files_count;
      block_size = descr.block_size;         
      file_size = descr.file_size;          
      blocks_used = descr.blocks_used;
      blocks_free = descr.blocks_free;
      first_free_block = descr.first_free_block;
      return *this;
    }
    typedef int64_t index_type;
    int32_t version;
    int32_t files_count;
    int32_t block_size;         // in bytes;
    int32_t file_size;          // in blocks
    int32_t blocks_used;
    int32_t blocks_free;
    index_type first_free_block;
    char preamble[PREAMBULE_SIZE];
    char Reserved[RESERVED_SIZE];

    void deserialize(io::Deserializer& deser) {
      deser.setrpos(0);
      uint32_t val = 0;
      const char* buf = deser.readAsIs(PREAMBULE_SIZE);
      //memcpy(preamble, buf, PREAMBULE_SIZE);
      deser >> val;
      version = val;
      if (version != dfVersion_64_2) {
        return;
      }

      deser >> val;
      files_count = val;

      deser >> val;
      block_size = val;

      deser >> val;
      file_size = val;

      deser >> val;
      blocks_used = val;

      deser >> val;
      blocks_free = val;

      uint64_t ffb;
      deser >> ffb;
      first_free_block = ffb;

      buf = deser.readAsIs(RESERVED_SIZE);
      //memcpy(Reserved, buf, RESERVED_SIZE);
    }

    void serialize(io::Serializer& ser) const {
      ser.writeAsIs(PREAMBULE_SIZE, preamble);
      ser << (uint32_t)version;
      ser << (uint32_t)files_count;
      ser << (uint32_t)block_size;
      ser << (uint32_t)file_size;
      ser << (uint32_t)blocks_used;
      ser << (uint32_t)blocks_free;
      ser << (uint64_t)first_free_block;
      ser.writeAsIs(RESERVED_SIZE, Reserved);
    }

    // persistent size!!
    static uint32_t persistentSize() {
        if ( ! size_ ) {
            std::vector< unsigned char > buf;
            io::Serializer ser(buf);
            DescriptionFile df;
            df.serialize( ser );
            size_ = static_cast<uint32_t>(ser.size());
        }
        return size_;
    }
};


template<class Key>
struct templBackupHeader 
{
private:
    static uint32_t size_;
public:
  typedef DescriptionFile::index_type index_type;
    void deserialize(io::Deserializer& deser) {
    deser.setrpos(0);
    uint64_t val;
    deser >> val;
    blocksCount = val;

    deser >> val;
    dataSize = val;

    deser >> val;
    curBlockIndex = val;

    deser >> key;
  }
  void serialize(io::Serializer& ser) const {
    ser << (uint64_t)blocksCount;
    ser << (uint64_t)dataSize;
    ser << (uint64_t)curBlockIndex;
    ser << key;
  }

    // persistent size!!
    static uint32_t persistentSize() {
        if ( ! size_ ) {
            std::vector< unsigned char > buf;
            io::Serializer ser(buf);
            templBackupHeader<Key> df;
            df.serialize( ser );
            size_ = ser.size();
        }
        return size_;
    }

public:
  uint64_t blocksCount;
  uint64_t dataSize;
  index_type curBlockIndex;
  Key key;
};

template< class Key > uint32_t templBackupHeader< Key >::size_ = 0;

// static const uint32_t DATA_BLOCK_HEADER_SIZE = 44;

template<class Key>
struct templDataBlockHeader
{
private:
    static uint32_t size_;

public:
    typedef DescriptionFile::index_type index_type;
    union
    {
        index_type next_free_block;
        index_type block_used;
    };
    uint64_t   total_blocks;
    uint64_t   data_size;
    index_type next_block;
    Key        key;
    bool       head;

    void deserialize(io::Deserializer& deser) {
      deser.setrpos(0);
      uint64_t val;
      deser >> val;
      next_free_block = val;

      deser >> key;

      deser >> val;
      total_blocks = val;

      deser >> val;
      data_size = val;

      deser >> val;
      next_block = val;

      uint8_t boolval;
      deser >> boolval;
      head = (bool)boolval;
    }

    void serialize(io::Serializer& ser) const {
      ser << (uint64_t)next_free_block;
      ser << key;
      ser << (uint64_t)total_blocks;
      ser << (uint64_t)data_size;
      ser << (uint64_t)next_block;
      ser << (uint8_t)head;
    }

    // persistent size!!!
    static uint32_t persistentSize() {
        if ( ! size_ ) {
            std::vector< unsigned char > buf;
            io::Serializer ser(buf);
            templDataBlockHeader<Key> hdr;
            hdr.serialize(ser);
            size_ = ser.size();
        }
        return size_;
    }
};

template< class Key > uint32_t templDataBlockHeader< Key >::size_ = 0;

template<class Key>
struct templDataBlock {
  templDataBlockHeader<Key> hdr;
  DataBlock data;
  templDataBlock(const templDataBlockHeader<Key>& _hdr, const DataBlock& _data)
                 :hdr(_hdr), data(_data) {}
  templDataBlock(const templDataBlockHeader<Key>& _hdr)
                 :hdr(_hdr), data(0) {}
  templDataBlock():data(0) {
    memset((void*)&hdr, 0, sizeof(hdr));
  }
};

template<class Key, class Profile>
class BlocksHSStorage
{
public:
    typedef DescriptionFile::index_type index_type;
    typedef DataBlockBackup<Profile>    data_type;

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
    static const int FIRST_FREE_BLOCK_FAILED	   = -13;

    static const int defaultBlockSize = 2048; // in bytes
    static const int defaultFileSize = 100; // in blocks 
    static const int64_t BLOCK_USED	= int64_t(1) << 63;

    BlocksHSStorage(DataFileManager& manager, io::GlossaryBase* g = NULL,
                    smsc::logger::Logger* thelog = 0): glossary_(g), running(false), deserBuf_(0), dataFileCreator_(manager, thelog)
    {
        /*
        if (!glossary_) {
          throw std::runtime_error("BlocksHSStorage: glossary should be provided!");
        }
         */
        logger = thelog;
        hdrSize = DataBlockHeader::persistentSize();
        memset(&backupHeader, 0, sizeof(BackupHeader));
        deserBuf_ = new unsigned char[deserBufSize_ = 70];
        memset(backupBuf, 0, BACKUP_BUF_SIZE);
    }


    virtual ~BlocksHSStorage()
    {
        Close();
        if (deserBuf_) delete[] deserBuf_;
    }


    static inline index_type invalidIndex() {
        return -1;
    }


    int Create(const string& _dbName, const string& _dbPath, int32_t _fileSize, int32_t _blockSize)
    {
        dbName = _dbName;
        dbPath = _dbPath;
        if (_blockSize > WRITE_BUF_SIZE) {
            if (logger) smsc_log_error(logger, "block size %d too large. max block size = %d", _blockSize, WRITE_BUF_SIZE);
            return CANNOT_CREATE_STORAGE;
        }
        if (_blockSize < hdrSize + MIN_BLOCK_SIZE) {
            if (logger) smsc_log_error(logger, "block size %d too small. min block size = %d", _blockSize, hdrSize + MIN_BLOCK_SIZE);
          return CANNOT_CREATE_STORAGE;
        }
		
        int ret;
        if(0 != (ret = CreateDescriptionFile(_blockSize, _fileSize)))
            return ret;
        try {
            dataFileCreator_.create(descrFile.first_free_block, descrFile.files_count);
            CreateDataFile();
        } catch (const std::exception& e) {
            return CANNOT_CREATE_DATA_FILE;
        }
        if(0 != (ret = CreateBackupFile()))
            return ret;

        running = true;
        return 0;
    }


    int Open(const string& _dbName, const string& _dbPath)
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
        if (!checkFirstFreeBlock()) {
          if (logger) smsc_log_warn(logger, "try to rollback last transaction");
          try {
              loadBackupData(true);
              if (!checkFirstFreeBlock()) {
                  if (!findFirstFreeBlock()) {
                      return FIRST_FREE_BLOCK_FAILED;
                  }
              }
          } catch ( std::exception& e ) {
              if (logger) smsc_log_warn(logger,"exception: %s", e.what());
              return FIRST_FREE_BLOCK_FAILED;
          }
        }

        dataFileCreator_.init(descrFile.file_size, descrFile.block_size, descrFile.files_count, dbPath, dbName);
        dataFileCreator_.openPreallocatedFile(descrFile.first_free_block);

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
        profileData.Empty();
        profile.Serialize(profileData, true, glossary_);
        if (logger) smsc_log_debug(logger, "Add data block key='%s' length=%d", key.toString().c_str(), profileData.length());
        int dataLength = profileData.length();
        if (dataLength <= 0) {
            blockIndex = -1;
            return false;
        }
        index_type ffb = descrFile.first_free_block;
        DescriptionFile oldDescrFile = descrFile;
        backupHeader.key = key;
        if (!backUpProfile(key, -1, bkp, dataLength, true)) {
            blockIndex = -1;
            if (logger) smsc_log_error(logger, "Backup profile error");
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
            SerialBuffer sb;
            profile.Deserialize(sb);
            bkp.clearBackup();
            descrFile = oldDescrFile;
            changeDescriptionFile();
            //exit(-1);
            throw;
        }
    }

    
    bool Change(DataBlockBackup<Profile>& prof, const Key& key, index_type& blockIndex) {
        if (blockIndex == -1) {
            return Add(prof, key, blockIndex);
        }
        Profile& profile = *prof.value;
        BlocksHSBackupData& bkp = *prof.backup;
        profileData.Empty();
        profile.Serialize(profileData, true, glossary_);
        if (logger) 
            smsc_log_debug(logger, "Change data block index=%d key='%s' length=%d",
                           blockIndex, key.toString().c_str(), profileData.length());
        DescriptionFile oldDescrFile = descrFile;
        index_type ffb = descrFile.first_free_block;
        backupHeader.key = key;
        int dataLength = profileData.length();
        if (!backUpProfile(key, blockIndex, bkp, dataLength, true)) {
            if (logger) smsc_log_error(logger, "Backup profile error");
            return false;
        }
        int blocksCount = (dataLength + effectiveBlockSize - 1) / effectiveBlockSize;
        int oldBlocksCount = (backupHeader.dataSize + effectiveBlockSize - 1) / effectiveBlockSize;
        int updateBlocksCount = blocksCount <= oldBlocksCount ? blocksCount : oldBlocksCount;

        try {
            bkp.clearBackup();
            index_type curBlockIndex = addBlocks(blockIndex, 0, updateBlocksCount, profileData, key, bkp); 
            if (blocksCount > oldBlocksCount) {
                if (logger) smsc_log_debug(logger, "Add new blocks");
                descrFile.first_free_block = addBlocks(ffb, updateBlocksCount, blocksCount,
                                                       profileData, key, bkp);
            }
            if (blocksCount < oldBlocksCount) {
                if (logger) smsc_log_debug(logger, "Remove empty blocks");
                descrFile.first_free_block = removeBlocks(ffb, curBlockIndex, updateBlocksCount);
            }
            //optimization
            if (oldDescrFile.first_free_block != descrFile.first_free_block || oldDescrFile.files_count != descrFile.files_count) { 
              changeDescriptionFile();
            }
            clearBackup();
            if (blocksCount == 0) {
                blockIndex = -1;
            }
            bkp.setBackupData(profileData.c_ptr(), profileData.length());
            return true;
        } catch (const std::exception& e) {
            bkp.setBackup(vector<index_type>(dataBlockBackup.begin(), dataBlockBackup.begin() + oldBlocksCount));
            size_t backupSize = bkp.getBackupDataSize();
            SerialBuffer sb(backupSize);
            sb.Append(bkp.getBackupData(), backupSize);
            sb.SetPos(0);
            profile.Deserialize(sb, true, glossary_);
            restoreDataBlocks(oldDescrFile, bkp.getBackupData());
            descrFile = oldDescrFile;
            changeDescriptionFile();
            //exit(-1);
            throw;
        }

    }
    

    bool recoverFromBackup( data_type& prof ) const
    {
        // prof.recoverFromBackup(glossary_);
        prof.value->deserialize( prof.backup->getBackupData(),
                                 prof.backup->getBackupDataSize(),
                                 glossary_ );
        return true;
    }


    bool Get(index_type blockIndex, DataBlock& data)
    {
        if (logger) smsc_log_debug(logger, "Get data block index=%d ", blockIndex);
        if (blockIndex == invalidIndex()) return false;
        char* buff;
        index_type curBlockIndex = blockIndex;
        int i = 0;
        do {
            DataBlockHeader hdr;

            const int file_number = getFileNumber(curBlockIndex);
            if ( file_number < 0 ) return false;
            const off_t offset = getOffset(curBlockIndex);
            File* f = dataFile_f[file_number];
            f->Seek(offset, SEEK_SET);
            deserialize(f, hdr);

            if (hdr.block_used != BLOCK_USED) { return false; }

            if(curBlockIndex == blockIndex)
            {
                data.setBuffLength(hdr.data_size);
                buff = data.ptr();
                data.setLength(hdr.data_size);
            }
            if ( invalidIndex() == hdr.next_block)
                f->Read((void*)(buff+(i*effectiveBlockSize)), hdr.data_size - effectiveBlockSize*i);
            else
                f->Read((void*)(buff+(i*effectiveBlockSize)), effectiveBlockSize);

            curBlockIndex = hdr.next_block;
            i++;
        } while ( curBlockIndex != invalidIndex() );
        return true;
    }


    bool Get(index_type blockIndex, DataBlockBackup<Profile>& prof )
    {
        Profile& profile = *prof.value;
        BlocksHSBackupData& bkp = *prof.backup;
        if (logger) smsc_log_debug(logger, "Get data block index=%d ", blockIndex);
        if (blockIndex == invalidIndex()) return false;
        char* buff;
        index_type curBlockIndex = blockIndex;
        int i = 0;
        profileData.Empty();
        do {

            DataBlockHeader hdr;
            const int file_number = getFileNumber(curBlockIndex);
            if ( file_number < 0 ) return false;
            const off_t offset = getOffset(curBlockIndex);
            File* f = dataFile_f[file_number];
            f->Seek(offset, SEEK_SET);
            deserialize(f, hdr);

            if (hdr.block_used != BLOCK_USED) {
                if (logger) smsc_log_error(logger, "block index=%d unused", curBlockIndex);
                return false;
            }
            if(curBlockIndex == blockIndex)
            {
                profileData.setBuffLength(hdr.data_size);
                buff = profileData.ptr();
                profileData.setLength(hdr.data_size);
            }
            size_t dataSize = hdr.next_block == invalidIndex() ?
                hdr.data_size - effectiveBlockSize*i : effectiveBlockSize;
            f->Read((void*)(buff+(i*effectiveBlockSize)), dataSize);
            curBlockIndex = hdr.next_block;
            ++i;
            bkp.addDataToBackup(hdr.next_block);
        } while( curBlockIndex != invalidIndex() );
        profile.Deserialize(profileData, true, glossary_);
        bkp.setBackupData(profileData.c_ptr(), profileData.length());
        return true;
    }


    void Remove(const Key& key, index_type blockIndex, const DataBlockBackup<Profile>& prof) {
        Profile& profile = *prof.value;
        BlocksHSBackupData& bkp = *prof.backup;
        if (logger) smsc_log_debug(logger, "Remove data block index=%d ", blockIndex);
        if (blockIndex < 0) {
            if (logger) smsc_log_warn(logger, "Can't remove data block index=%d ", blockIndex);
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
            if (logger) smsc_log_error(logger, "error remove data block index:%d : %s", blockIndex, e.what());
            ::abort();
        }
    }


private:
    /// recovery procedures
    class Recovery 
    {
    public:
        Recovery( BlocksHSStorage& bhs ) : bhs_(bhs), df_(bhs_.descrFile) {
            if ( df_.files_count <= 0 ) {
                if (bhs_.logger) smsc_log_error(bhs_.logger,"cannot recover as files_count = 0");
                throw smsc::util::Exception("cannot recover as files_count = 0");
            }
            fixup_.reset(new FixupLogger(bhs_.dbPath + '/' + bhs_.dbName + ".fixup"));
            std::vector< unsigned char > buf;
            io::Serializer ser(buf);
            df_.serialize(ser);
            fixup_->makeComment(2,ser.size(),ser.data());
            lastFreeBlock_ = df_.first_free_block = bhs_.invalidIndex();
            df_.blocks_free = 0;
            df_.blocks_used = df_.files_count * df_.file_size;
        }

        ~Recovery() {
            // we have to write down a new description file
            bhs_.changeDescriptionFile();
        }

        /// the methods frees a chain of used blocks
        void freeBlocks( index_type blockIndex, DataBlockHeader& hdr ) {
            const index_type totalBlocks = df_.files_count * df_.file_size;
            do {
                if ( bhs_.logger ) smsc_log_info( bhs_.logger, "freeing block %lld, next %lld", blockIndex, hdr.next_block );
                freeBlock( blockIndex, hdr.next_free_block );
                blockIndex = hdr.next_block;
                if ( blockIndex < 0 || blockIndex >= totalBlocks ) {
                    // this block points to nowhere
                    if (bhs_.logger) smsc_log_info(bhs_.logger, "next block %lld points to nowhere");
                    break;
                } else {
                    // reading header
                    int fn = bhs_.getFileNumber(blockIndex);
                    off_t offset = bhs_.getOffset(blockIndex);
                    File* f = bhs_.dataFile_f[fn];
                    f->Seek(offset);
                    bhs_.deserialize(f,hdr);
                }
            } while ( hdr.block_used == BLOCK_USED );
        }

        void chainFreeBlock( index_type blockIndex, DataBlockHeader& hdr ) {
            freeBlock( blockIndex, hdr.next_free_block );
        }

    private:
        void freeBlock( index_type blockIndex, index_type saved_next_free_block )
        {
            // EndianConverter cvt;
            if ( lastFreeBlock_ != bhs_.invalidIndex() ) {
                int fn = bhs_.getFileNumber(lastFreeBlock_);
                off_t offset = bhs_.getOffset(lastFreeBlock_);
                File* f = bhs_.dataFile_f[fn];
                f->Seek(offset);
                f->WriteNetInt64(blockIndex);
            } else {
                // lastFreeBlock was not set
                df_.first_free_block = blockIndex;
            }
            int fn = bhs_.getFileNumber(blockIndex);
            off_t offset = bhs_.getOffset(blockIndex);
            char cvtbuf[8];
            io::EndianConverter::set64(cvtbuf,uint64_t(saved_next_free_block));
            fixup_->save(offset,8,cvtbuf);
            File* f = bhs_.dataFile_f[fn];
            f->Seek(offset);
            f->WriteNetInt64(bhs_.invalidIndex());
            lastFreeBlock_ = blockIndex;
            ++df_.blocks_free;
            --df_.blocks_used;
        }

    private:
        BlocksHSStorage&           bhs_;
        DescriptionFile&           df_;
        std::auto_ptr<FixupLogger> fixup_;
        index_type                 lastFreeBlock_;
    };

public:
    /// used to iterate over BlocksHSStorage.
    /// NOTE: modification operation may invalidate the iterator!
    class Iterator {
        friend class BlocksHSStorage;
    private:
        Iterator( BlocksHSStorage& bhs, Recovery* rec = 0 ) :
        bhs_(&bhs), iterBlockIndex_(0), recovery_(rec) {}
    public:
        Iterator( const Iterator& o ) {
            Iterator& i(const_cast<Iterator&>(o));
            bhs_ = i.bhs_;
            iterBlockIndex_ = i.iterBlockIndex_;
            recovery_ = i.recovery_;
            curBlockIndex_ = i.curBlockIndex_;
            blockData_ = i.blockData_;
            key_ = i.key_;
        }

        inline void reset() { iterBlockIndex_ = 0; }
        
        /// find the next iteration
        bool next() {
            if (!bhs_) return false;
            DataBlockHeader hdr;
            const index_type cnt = bhs_->descrFile.files_count * bhs_->descrFile.file_size;
            while ( iterBlockIndex_ < cnt ) {

                const int fn = bhs_->getFileNumber(iterBlockIndex_);
                if ( fn < 0 ) return false;
                const off_t offset = bhs_->getOffset(iterBlockIndex_);
                File* f = bhs_->dataFile_f[fn];
                f->Seek(offset,SEEK_SET);
                bhs_->deserialize(f,hdr);
                curBlockIndex_ = iterBlockIndex_++;
                // if ( recovery_.get() ) recovery_->recoverBlock(curBlockIndex_,hdr);
                if (hdr.block_used == BLOCK_USED) {
                    if (hdr.head) {
                        key_ = hdr.key;
                        if ( bhs_->Get(curBlockIndex_,blockData_) ) { return true; }
                        if ( bhs_->logger ) smsc_log_error( bhs_->logger, "Error reading block chain at %llu", curBlockIndex_ );
                        if ( recovery_.get() ) recovery_->freeBlocks(curBlockIndex_,hdr);
                    }
                } else {
                    // unused block
                    if ( recovery_.get() ) recovery_->chainFreeBlock(curBlockIndex_,hdr);
                }
            }
            return false;
        }
        
        inline index_type blockIndex() const { return curBlockIndex_; }
        inline const DataBlock& blockData() const { return blockData_; }
        inline const Key& key() const { return key_; }
        
    private:
        BlocksHSStorage*                 bhs_;
        index_type                       iterBlockIndex_;
        std::auto_ptr<Recovery>          recovery_;

        // data which are set as a result of a positive next() call.
        index_type       curBlockIndex_;
        DataBlock        blockData_;
        Key              key_;
    };


    Iterator begin() { return Iterator(*this); }
    
    /// NOTE: this version of iterator automatically fixup (free) broken blocks.
    /// Use it only for recovery.
    Iterator beginWithRecovery() { return Iterator(*this,new Recovery(*this)); }

    /*
    void Reset()
    {
        iterBlockIndex = 0;
    }
    
    // NOTE: this method is used for index rebuilding
    bool next( index_type& blockIndex, Key& key )
    {
        DataBlock db;
        return Next(blockIndex,db,key);
    }

    bool Next( index_type& blockIndex, DataBlock& data, Key& key )
    {
        uint64_t cnt = descrFile.files_count * descrFile.file_size;
        DataBlockHeader hdr;
        
        while(iterBlockIndex < cnt)
        {
            int file_number = iterBlockIndex / descrFile.file_size;
            off_t offset = (iterBlockIndex - file_number * descrFile.file_size) * descrFile.block_size;
            File* f = dataFile_f[file_number];
            f->Seek(offset, SEEK_SET);
            deserialize(f, hdr);

            blockIndex = iterBlockIndex++;
            if(hdr.block_used == BLOCK_USED && hdr.head)
            {
                key = hdr.key;
                if ( Get(blockIndex,data) ) {
                    return true;
                }
                if (logger) smsc_log_error(logger, "Error reading block: %d", blockIndex);
            }
        }
        return false;
    }
     */

private:

  template<class T> void deserialize(File* f, T& hdr) {
      unsigned needsize = T::persistentSize();
      if ( deserBufSize_ < needsize ) {
          delete [] deserBuf_;
          deserBuf_ = new unsigned char[deserBufSize_ = needsize];
      }
      f->Read( (void*)deserBuf_, needsize );
      io::Deserializer ds( deserBuf_, needsize );
      hdr.deserialize(ds);
  }

    void printHdr(const DataBlockHeader& hdr) {
        if (!logger) return;
        smsc_log_debug(logger, "block header used=%d next_free=%d head=%d total=%d next=%d size=%d key='%s'",
                       hdr.getBlockUsed(), hdr.next_free_block, hdr.head, hdr.total_blocks, 
                       hdr.next_block, hdr.data_size, hdr.key.toString().c_str());
    }


    void writeDataBlock(int fileNumber, off_t offset, const DataBlockHeader& hdr, const void* data, size_t curBlockSize)
    {
        if (logger)
            smsc_log_debug(logger, "write data block fn=%d, offset=%d, blockSize=%d", fileNumber,
                           offset, curBlockSize);
        /*
         * NOTE: fn and offset must be already checked
        if ( !checkfn(fileNumber) ) {
            throw smsc::util::Exception("Invalid file number %d, max file number %d", fileNumber, descrFile.files_count-1);
        }
         */
        size_t bufSize = hdrSize + curBlockSize;
        // memset(&(serHdrBuf_[0]), 0, DataBlockHeader::persistentSize());
        io::Serializer ser(serHdrBuf_);
        hdr.serialize(ser);
        memcpy(writeBuf, ser.data(), ser.size());
        memcpy(writeBuf + ser.size(), data, curBlockSize);
        try {
            File* f = dataFile_f[fileNumber];
            f->Seek(offset, SEEK_SET);
            f->Write((void *)writeBuf, bufSize);
        } catch (const std::exception& e) {
            if (logger)
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
        memset(&hdr, 0, sizeof(DataBlockHeader));
        hdr.block_used = BLOCK_USED;
        hdr.key = key;
        hdr.head = backupStartIndex == 0 ? true : false;
        hdr.total_blocks = (dataLength + effectiveBlockSize - 1) / effectiveBlockSize;
        hdr.data_size = dataLength;//effectiveBlockSize;
        size_t curBlockSize = 0;
        for (int i = backupStartIndex; i < blocksCount; ++i) {
            const int file_number = getFileNumber(curBlockIndex);
            if (file_number < 0) {
                throw smsc::util::Exception("invalid file number %d, max file number %d", file_number, descrFile.files_count-1);
            }
            const off_t offset = getOffset(curBlockIndex);
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
        if (logger) smsc_log_debug(logger, "addBlocks curBlockIndex=%d", curBlockIndex);
        return curBlockIndex;
    }

    index_type getFirstFreeBlock(int fileNumber, off_t offset)
    {
        /*
         * fileNumber is already checked in getFileNumber!
        if (!checkfn(fileNumber)) {
            throw smsc::util::Exception("Invalid file number %d, max file number %d", fileNumber, descrFile.files_count-1);
        }
         */
        File* f = dataFile_f[fileNumber];
        f->Seek(offset, SEEK_SET);
        index_type ffb = f->ReadNetInt64();
        dataFileCreator_.create(ffb, descrFile.files_count);
        if (ffb == -1) {
            ffb = descrFile.files_count * descrFile.file_size;
            CreateDataFile();
        }
        if (logger) smsc_log_debug(logger, "First free block %d", ffb);
        return ffb;
    }

    index_type removeBlocks(index_type ffb, index_type blockIndex, size_t backupIndex) {
        if (logger) smsc_log_debug(logger, "Remove %d blocks start block index=%d", 
                                   backupHeader.blocksCount - backupIndex, blockIndex);
        if (backupIndex >= backupHeader.blocksCount) {
            if (logger) smsc_log_warn(logger, "Can't remove unbackuped data blocks");
            return ffb;
        }
        while (blockIndex != -1) {
            if (logger) smsc_log_debug(logger, "Remove %d block, ffb=%d", blockIndex, ffb); 
            const int file_number = getFileNumber(blockIndex);
            if ( file_number < 0 ) {
                throw smsc::util::Exception("Invalid file number %d, max file number %d", file_number, descrFile.files_count-1);
            }
            const off_t offset = getOffset(blockIndex);
            File* f = dataFile_f[file_number];
            f->Seek(offset);
            f->WriteNetInt64(ffb);
            ffb = blockIndex;
            blockIndex = dataBlockBackup[backupIndex];
            ++backupIndex;
        }
        return ffb;
    }


    int CreateBackupFile() {
        try {
            if (logger) smsc_log_debug(logger, "Create backup file");
            backupFile_f.RWCreate((dbPath + '/' + dbName + ".trx").c_str());
            backupFile_f.SetUnbuffered();
            backupFile_f.WriteByte(TRX_COMPLETE);
            return 0;
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "FSStorage: error create backup file: '%s'", e.what());
            return BACKUP_FILE_CREATE_FAILED;
        }
    }

    int CreateDescriptionFile(int32_t _blockSize, int32_t _fileSize)
    {
        try
        {
            string name = dbPath + '/' + dbName;
            if (File::Exists(name.c_str())) {
                if (logger) smsc_log_error(logger, "FSStorage: error create description file: file '%s' already exists",
                               name.c_str());
                return DESCR_FILE_CREATE_FAILED;
            }
            descrFile_f.RWCreate(name.c_str());
            descrFile_f.SetUnbuffered();
        }
        catch(const std::exception& e)
        {
            if (logger) smsc_log_error(logger, "FSStorage: error create description file: '%s'", e.what());
            return DESCR_FILE_CREATE_FAILED;
        }

        // initialize Description File structure
        descrFile.block_size = _blockSize;
        descrFile.file_size = _fileSize;
        effectiveBlockSize = descrFile.block_size - hdrSize;
        changeDescriptionFile();
        dataFileCreator_.init(descrFile.file_size, descrFile.block_size, 0, dbPath, dbName); 
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
        if (logger) smsc_log_info(logger, "Create data file: '%s'", name.c_str());
        index_type startBlock = descrFile.files_count * descrFile.file_size;
        try
        {
          File *f = dataFileCreator_.getFile();
          if (!f) {
            throw FileException(FileException::errOpenFailed, dataFileCreator_.getFileName());
          }
          dataFile_f.push_back(f);
        }
        catch (const std::exception& ex)
        {
            if (logger) smsc_log_error(logger, "Error create data file. std::exception: '%s'", ex.what());
            throw;
        }
        descrFile.files_count++;
        descrFile.blocks_free = descrFile.file_size;
        descrFile.first_free_block = startBlock;
        printDescrFile();
        changeDescriptionFile();
        if (logger) smsc_log_info(logger, "data file:'%s' created", name.c_str());
        return 0;
    }


    void changeDescriptionFile() {
        try {
            if (logger) smsc_log_debug(logger, "Change description file");
            descrFile_f.Seek(0, SEEK_SET);        
            io::Serializer ser(serDescrBuf_);
            descrFile.serialize(ser);
            descrFile_f.Write(ser.data(), ser.size());
            printDescrFile();
            return;
        } catch (const std::exception& ex) {
            if (logger) smsc_log_error(logger, "Can't write to description file. std::exception: '%s'", ex.what());
            printDescrFile();
        } 
        try {
            string name = descrFile_f.getFileName() + ".tmp";
            File tmpFile;
            tmpFile.RWCreate(name.c_str());
            tmpFile.SetUnbuffered();
            tmpFile.Seek(0, SEEK_SET);
            io::Serializer ser(serDescrBuf_);
            descrFile.serialize(ser);
            tmpFile.Write(ser.data(), ser.size());
            if (logger) smsc_log_error(logger, "Last description data saved in file '%s'", name.c_str());
        } catch (const std::exception& ex) {
            if (logger) smsc_log_error(logger, "Can't save temp description file. std::exception: '%s'", ex.what());
        }
        ::abort();
    }


    void printDescrFile()
    {
        if (!logger) return;
        smsc_log_debug(logger, "DescrFile: files_count=%d, block_size=%d, file_size=%d, first_free_block=%d",
                       descrFile.files_count, descrFile.block_size, descrFile.file_size, descrFile.first_free_block);
    }


    void loadBackupData(bool ignoreCompleteFlag = false) {
        try {
            dataBlockBackup.clear();
            backupFile_f.Seek(0);
            uint8_t trx = backupFile_f.ReadByte();
            if (trx == TRX_COMPLETE && !ignoreCompleteFlag) {
                if (logger) smsc_log_debug(logger, "Last transaction is complete");
                return;
            }
            if (ignoreCompleteFlag) {
              if (logger) smsc_log_warn(logger, "Last transaction is complete. Rollback last transaction.");
            } else {
              if (logger) smsc_log_warn(logger, "Last transaction is incomplete. Load transaction data");
            }

            deserialize(&backupFile_f, descrFile);
            deserialize(&backupFile_f, backupHeader);

            for (int i = 0; i < backupHeader.blocksCount; ++i) {
                index_type nextBlock = backupFile_f.ReadNetInt64();
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
            if (logger) smsc_log_error(logger, "Error loading transaction data: '%s'", e.what());
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
            if (logger) smsc_log_warn(logger, "FSStorage: error open backup file: '%s'\n", e.what());
            if (File::Exists(name.c_str())) {
                if (logger) smsc_log_error(logger, "FSStorage: backup file - exists, but can't be opened correct: '%s'\n", e.what());
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
            deserialize(&descrFile_f, descrFile);

            if (descrFile.version != dfVersion_64_2) {
              if (logger) smsc_log_error(logger, "FSStorage: invalid version of data storage:0x%x, correct version:0x%x", descrFile.version, dfVersion_64_2);
              return CANNOT_OPEN_EXISTS_DESCR_FILE;
            }

            if (logger) smsc_log_info(logger, "OpenDescrFile: storage version:0x%x, files count:%d, block size:%d, file size:%d, first free block:%lld",
                          descrFile.version, descrFile.files_count, descrFile.block_size, descrFile.file_size, descrFile.first_free_block);
            effectiveBlockSize = descrFile.block_size - hdrSize;
            return 0;
        }
        catch(const std::exception& ex)
        {
            if (File::Exists(name.c_str())) {
                if (logger) smsc_log_error(logger, "FSStorage: idx_file - exists, but can't be opened : %s\n", ex.what());
                return OpenBackupFile();
            }
            if (logger) smsc_log_debug(logger, "FSStorage: error idx_file - %s\n", ex.what());
            return DESCR_FILE_OPEN_FAILED;
        }
    }


    int OpenDataFiles(void)
    {
        if (descrFile.files_count <= 0) {
            if (logger) smsc_log_error(logger, "Open data files error: files count=%d", descrFile.files_count);
            return CANNOT_OPEN_DATA_FILE;
        }
        char	buff[10];
        string path = dbPath + '/' + dbName;
        for(int i = 0; i < descrFile.files_count; i++)
        {
            snprintf(buff, 10, "-%.7d", i);
            string name = path + buff;
            dataFile_f.push_back(new File());
            try
            {
                if (logger) smsc_log_debug(logger, "Open data file: %s", name.c_str());
                dataFile_f[i]->RWOpen(name.c_str());
                dataFile_f[i]->SetUnbuffered();
            }
            catch(const std::exception& ex)
            {
                if (logger) smsc_log_error(logger, "Cannot open data file: %s", ex.what());
                return CANNOT_OPEN_DATA_FILE;
            }
        }
        return 0;
    }

    
    void restoreDataBlocks(const DescriptionFile& _descrFile, const char* data) {
        if (dataBlockBackup.size() <= 0) {
            if (logger) smsc_log_warn(logger, "data blocks backup is empty");
            return;
        }
        try {
            if (logger) smsc_log_warn(logger, "Restoring data storage...");
            size_t profileBlocksCount = (backupHeader.dataSize + effectiveBlockSize - 1) / effectiveBlockSize;
            index_type curBlockIndex = backupHeader.curBlockIndex;
            DataBlockHeader hdr;
            memset(&hdr, 0, sizeof(DataBlockHeader));
            hdr.block_used = BLOCK_USED;
            hdr.key = backupHeader.key;
            hdr.head = true;
            hdr.data_size = backupHeader.dataSize;
            hdr.total_blocks = profileBlocksCount;
            int dataSize = 0;
            for (int i = 0; i < profileBlocksCount; ++i) {
                const int file_number = getFileNumber(curBlockIndex);
                if ( file_number < 0 ) {
                    throw smsc::util::Exception("Invalid file number %d, max file number %d", file_number, descrFile.files_count-1);
                }
                const off_t offset = getOffset(curBlockIndex);
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
                const int file_number = getFileNumber(curBlockIndex);
                if ( file_number < 0 ) {
                    throw smsc::util::Exception("invalid fn: %d, max fn: %d", file_number, descrFile.files_count-1);
                }
                const off_t offset = getOffset(curBlockIndex);
                curBlockIndex = dataBlockBackup[i];
                File* f = dataFile_f[file_number];
                f->Seek(offset);
                f->WriteNetInt64(curBlockIndex);
                if (logger) smsc_log_debug(logger, "restore next_free_block=%d offset=%d", curBlockIndex, offset);
            }
            clearBackup();
            if (logger) smsc_log_warn(logger, "Restoring complite");
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "Error restore data storage: '%s'", e.what());
            ::abort();
        }
    }


    void writeBackup(File& f, const DescriptionFile& _descrFile, const char* backupData) {

        // serialization
        io::Serializer descrser(serDescrBuf_);
        _descrFile.serialize(descrser);
        io::Serializer backupser(serBackupBuf_);
        backupHeader.serialize(backupser);

        const size_t backupSize = sizeof(TRX_INCOMPLETE) + descrser.size() + 
            backupser.size() +
            backupHeader.blocksCount * sizeof(descrFile.first_free_block) +
            backupHeader.dataSize;
        char* buf = 0;
        if (backupSize > BACKUP_BUF_SIZE) {
          buf = new char[backupSize];
        } else {
          //memset(backupBuf, 0, BACKUP_BUF_SIZE);
          buf = backupBuf;
        }

        try {

            memcpy(buf, &TRX_INCOMPLETE, 1);
            size_t bufOffset = sizeof(TRX_INCOMPLETE);
            memcpy(buf + bufOffset, descrser.data(), descrser.size());
            bufOffset += descrser.size();
            memcpy(buf + bufOffset, backupser.data(), backupser.size());
            bufOffset += backupser.size();

            uint64_t ni = 0;
            for (int i = 0; i < backupHeader.blocksCount; ++i) { 
                ni = Uint64Converter::toNetworkOrder(dataBlockBackup[i]);
                memcpy(buf + bufOffset, &ni, sizeof(ni)); 
                bufOffset += sizeof(ni);
            }
            if (backupHeader.dataSize > 0) {
                memcpy(buf + bufOffset, backupData, backupHeader.dataSize);           
            }
            f.Seek(0);
            f.Write(buf, backupSize);
            if (backupSize > BACKUP_BUF_SIZE) {
              delete[] buf;
            }
        } catch (...) {
          if (backupSize > BACKUP_BUF_SIZE) {
            delete[] buf;
          }
          throw;
        }
    }


    void saveBackupToFile(const DescriptionFile& _descrFile, const char* backupData) {
        try {
            //if (logger) smsc_log_debug(logger, "Save transaction data to file");
            //printDescrFile();
            writeBackup(backupFile_f, _descrFile, backupData);
            return;
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "Error saving transaction data: '%s'", e.what());
        }
        try {
            string tmpName = backupFile_f.getFileName() + ".tmp";
            if (logger) smsc_log_error(logger, "Trying to save transaction data to file '%s'...", tmpName.c_str());
            File tmpFile;
            tmpFile.RWCreate(tmpName.c_str());
            tmpFile.SetUnbuffered();
            writeBackup(tmpFile, _descrFile, backupData);
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "%s", e.what());
        }
        ::abort();
    }


    void clearBackup() {
        try {
            if (logger) smsc_log_debug(logger, "clear backup");
            backupFile_f.Seek(0);
            backupFile_f.WriteByte(TRX_COMPLETE);
            dataBlockBackup.clear();
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "Error clear transaction data: '%s'", e.what());
        }
    }


    index_type readFreeBlocks(int blocksCount) {
        if (logger) smsc_log_debug(logger, "Read %d free blocks", blocksCount);
        index_type curBlockIndex = descrFile.first_free_block;
        for (int i = 0; i < blocksCount; ++i) {
            const int file_number = getFileNumber(curBlockIndex);
            if ( file_number < 0 ) {
                throw smsc::util::Exception("invalid fn: %d, max fn: %d", file_number, descrFile.files_count-1);
            }
            const off_t offset = getOffset(curBlockIndex);
            curBlockIndex = getFirstFreeBlock(file_number, offset);
            dataBlockBackup.push_back(curBlockIndex);
        }
        return curBlockIndex;
    }


    bool backUpProfile(const Key& key, index_type blockIndex, const BlocksHSBackupData& profile, int dataLength, bool saveToFile = false) 
    {
        if (logger) smsc_log_debug(logger, "backup profile key=%s block index=%d, data length=%d",
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
                /*index_type curBlockIndex =*/
                readFreeBlocks(blocksCount);
                if (saveToFile) {
                    saveBackupToFile(oldDescrFile, NULL ); // profile.getBackupData());
                }
                if (logger) smsc_log_debug(logger, "backup profile size=%d", dataBlockBackup.size());
                return true;
            }
            backupHeader.curBlockIndex = blockIndex;
            backupHeader.dataSize = profile.getBackupDataSize(); 
            dataBlockBackup = profile.getBackup();
            size_t backupSize = dataBlockBackup.size();
            backupHeader.blocksCount = blocksCount >= backupSize ? blocksCount : backupSize;
            if (backupSize == 0) {
                if (logger) smsc_log_warn(logger, "Error backup profile: key=%s backup is empty", key.toString().c_str() );
                return false;
            }
            if (backupSize < blocksCount) {
                dataBlockBackup[backupSize - 1] = oldDescrFile.first_free_block;
                readFreeBlocks(blocksCount - backupSize);
            }
            if (logger) smsc_log_debug(logger, "backup profile key=%s size=%d", key.toString().c_str(), dataBlockBackup.size());
            if (saveToFile) {
                saveBackupToFile(oldDescrFile, profile.getBackupData());
            }
            return true;
        } catch (const std::exception& e) {
            if (logger) smsc_log_warn(logger, "Error backup profile key=%s. std::exception: '%s'",
                          key.toString().c_str(), e.what());
            dataBlockBackup.clear();
            descrFile = oldDescrFile;
            changeDescriptionFile();
            ::abort();
        }
    }

    inline bool findFirstFreeBlock() {
      try {
        if (logger) smsc_log_warn(logger, "try to find first free block");
        index_type lastBlock = descrFile.files_count * descrFile.file_size - 1;
        int fileNumber = getFileNumber(lastBlock);
        if (fileNumber < 0) {
          smsc_log_error(logger, "can't find first free block: last block index is invalid %lld", lastBlock);
          return false;
        }
        off_t offset = getOffset(lastBlock);
        File* f = dataFile_f[fileNumber];
        f->Seek(offset, SEEK_SET);
        index_type ffb = f->ReadNetInt64();
        if (ffb != -1) {
          smsc_log_error(logger, "can't find first free block: last block header is invalid %lld, mast be -1", ffb);
          return false;
        }
        for (index_type curBlockIndex = lastBlock - 1; curBlockIndex >= 0; --curBlockIndex) {
          fileNumber = getFileNumber(curBlockIndex);
          if (fileNumber < 0) {
            smsc_log_error(logger, "can't find first free block: current block index is invalid %lld", curBlockIndex);
            return false;
          }
          offset = getOffset(curBlockIndex);
          File* f = dataFile_f[fileNumber];
          f->Seek(offset, SEEK_SET);
          index_type nextffb = f->ReadNetInt64();
          if (nextffb == BLOCK_USED) {
            smsc_log_error(logger, "first used block found: %lld, first free block will be: %lld", curBlockIndex, curBlockIndex + 1);
            descrFile.first_free_block = curBlockIndex + 1;
            changeDescriptionFile();
            if (logger) smsc_log_info(logger, "first free block fixed: %lld", descrFile.first_free_block);
            return true;
          }
        }
      } catch (const std::exception& ex) {
        if (logger) smsc_log_error(logger, "Error finding first free block: %s", ex.what());
      }
      smsc_log_error(logger, "first free block not found");
      return false;
    }

    inline bool checkFirstFreeBlock() {
      try {
        if (logger) smsc_log_info(logger, "check first free block: %lld", descrFile.first_free_block);
        index_type curBlockIndex = descrFile.first_free_block;
        if (curBlockIndex < 0) {
          smsc_log_error(logger, "first free block invalid: %lld, can't fix first free block", curBlockIndex);
          return false;
        }
        const int fileNumber = getFileNumber(curBlockIndex);
        if ( fileNumber < 0 ) {
            smsc_log_error(logger, "first free block invalid: %lld, can't fix first free block", curBlockIndex);
            return false;
        }
        const off_t offset = getOffset(curBlockIndex);
        File* f = dataFile_f[fileNumber];
        f->Seek(offset, SEEK_SET);
        index_type ffb = f->ReadNetInt64();
        index_type maxBlockIndex = descrFile.files_count * descrFile.file_size;
        if (ffb == -1 || (ffb >= 0 && ffb < maxBlockIndex)) {
          if (logger) smsc_log_info(logger, "first free block: %lld valid, next free block: %lld", curBlockIndex, ffb);
          return true;
        }

        if (ffb == BLOCK_USED) {
          if (logger) smsc_log_warn(logger, "first free block: %lld used, try to find correct first free block...", curBlockIndex);
        } else {
          if (logger) smsc_log_warn(logger, "first free block: %lld invalid, try to find correct first free block...", curBlockIndex);
        }
        ++curBlockIndex;
        while (curBlockIndex < maxBlockIndex) {
          const int fn = getFileNumber(curBlockIndex);
          if (fn < 0) {
            if (logger) smsc_log_error(logger, "can't fix first free block: %lld", curBlockIndex);
            return false;
          }
          const off_t offset = getOffset(curBlockIndex);
          File* f = dataFile_f[fileNumber];
          f->Seek(offset, SEEK_SET);
          index_type nextffb = f->ReadNetInt64();
          if (nextffb == -1 || (nextffb >= 0 && nextffb < maxBlockIndex)) {
            if (logger) smsc_log_info(logger, "correct first free block found: %lld, next free block: %lld", curBlockIndex, nextffb);
            descrFile.first_free_block = curBlockIndex;
            changeDescriptionFile();
            if (logger) smsc_log_info(logger, "first free block fixed %lld", descrFile.first_free_block);
            return true;
          }
          ++curBlockIndex;
        }
        if (logger) smsc_log_error(logger, "can't fix first free block: %lld", curBlockIndex);
      } catch (const std::exception& ex) {
        if (logger) smsc_log_error(logger, "Error checking first free block: %s", ex.what());
      }
      return false;
    }


    inline int getFileNumber(index_type index) const {
        int fn = index / descrFile.file_size;
        if (!checkfn(fn)) return -1;
        return fn;
    }

    inline off_t getOffset(index_type index) const {
        return (index % descrFile.file_size) * descrFile.block_size;
    }

    /// check file number
    inline bool checkfn( int fn ) const {
        if ( fn >= 0 && fn < descrFile.files_count ) return true;
        if (logger) smsc_log_error(logger, "Invalid file number %d, max file number %d", fn, descrFile.files_count-1 );
        return false;
    }

private:

  Logger* logger;
  io::GlossaryBase* glossary_;
  bool running;
  string dbName;
  string dbPath;
  DescriptionFile descrFile;		
  File descrFile_f;
  File backupFile_f;
  vector<File*> dataFile_f;
  // iterBlockIndex is not used anymore, see Iterator
  // index_type iterBlockIndex;
  int effectiveBlockSize;	

  vector<index_type> dataBlockBackup;
  char writeBuf[WRITE_BUF_SIZE];
  char backupBuf[BACKUP_BUF_SIZE];
  size_t hdrSize;
  SerialBuffer profileData;
  CompleteDataBlock completeDataBlock;
  BackupHeader backupHeader;

  std::vector< unsigned char > serDescrBuf_;
  std::vector< unsigned char > serBackupBuf_;
  std::vector< unsigned char > serHdrBuf_;
    unsigned char* deserBuf_;
    unsigned deserBufSize_;
    DataFileCreator dataFileCreator_;

};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE_H */
