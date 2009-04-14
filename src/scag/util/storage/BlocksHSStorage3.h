//------------------------------------
//  BlocksHSStorage.h
//  Odarchenko Vitaly, 2008
//------------------------------------
//

#ifndef _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE3_H
#define _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE3_H

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
#include "DataBlockBackup2.h"
#include "util/Exception.hpp"
#include "DataFileCreator.h"
#include "FixupLogger.h"
#include "EndianConverter.h"
#include "BlocksHSStorage.h"
#include "HSPacker.h"
#include "scag/util/Drndm.h"

namespace scag {
namespace util {
namespace storage {

using std::vector;
using std::string;
using smsc::logger::Logger;

struct BHDescription3
{
    typedef HSPacker::index_type  index_type;
    typedef HSPacker::offset_type offset_type;

    BHDescription3( unsigned v = 0x80000002 ) :
    version(v), block_size(1024), file_size(100), files_count(0), first_free_block(0) {}

    void serialize( Serializer& ser ) const
    {
        ser << uint32_t(version);
        ser << uint32_t(block_size);
        ser << uint32_t(file_size);
        ser << uint32_t(files_count);
        ser << uint64_t(first_free_block);
    }

    void deserialize( Deserializer& dsr ) {
        uint32_t v;
        dsr >> v;
        version = v;
        dsr >> v;
        block_size = v;
        dsr >> v;
        file_size = v;
        dsr >> v;
        files_count = v;
        dsr >> first_free_block;
    }

public:
    unsigned   version;
    unsigned   block_size;
    unsigned   file_size;          // in blocks
    unsigned   files_count;
    index_type first_free_block;   // index
};


template<class Key, class Profile>
class BlocksHSStorage3
{
public:
    typedef HSPacker::index_type        index_type;
    typedef HSPacker::offset_type       offset_type;
    typedef DataBlockBackup2< Profile > data_type;

private:
    typedef BHDescription3         DescriptionFile;
    typedef BlockNavigation        DataBlockHeader;
    typedef HSPacker::buffer_type  buffer_type;

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

    BlocksHSStorage3( DataFileManager& manager, GlossaryBase* g = NULL,
                      smsc::logger::Logger* thelog = 0) :
    logger(thelog),
    glossary_(g),
    running(false),
    profileData_(0),
    dataFileCreator_(manager, thelog),
    packer_(defaultBlockSize,0,thelog),
    fileSizeBytes_(0)
    {

        rnd_.setSeed(time(0));
        /*
        if (!glossary_) {
          throw std::runtime_error("BlocksHSStorage: glossary should be provided!");
        }
         */
    }


    virtual ~BlocksHSStorage3()
    {
        Close();
    }


    /// return the invalid index (for external clients).
    static inline index_type invalidIndex() {
        return HSPacker::invalidIndex();
    }


    int Create(const string& _dbName, const string& _dbPath, uint32_t _fileSize, uint32_t _blockSize)
    {
        dbName = _dbName;
        dbPath = _dbPath;
        if ( _blockSize > WRITE_BUF_SIZE ) {
            if (logger) { smsc_log_error(logger, "block size %d too large. max block size = %d", _blockSize, WRITE_BUF_SIZE); }
            return CANNOT_CREATE_STORAGE;
        }
        if ( _blockSize < navSize() + MIN_BLOCK_SIZE ) {
            if (logger) { smsc_log_error(logger, "block size %d too small. min block size = %d", _blockSize, navSize() + MIN_BLOCK_SIZE); }
            return CANNOT_CREATE_STORAGE;
        }
		
        int ret;
        if(0 != (ret = CreateDescriptionFile(_blockSize, _fileSize)))
            return ret;
        try {
            dataFileCreator_.create( descrFile.first_free_block, descrFile.files_count, true);
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

        dataFileCreator_.init(descrFile.file_size, descrFile.block_size, descrFile.files_count, dbPath, dbName, true);
        dataFileCreator_.openPreallocatedFile(descrFile.first_free_block);

        running = true;
        return 0;
    }
	

    void Close(void)
    {
        if(running)
        {
            for( size_t i = 0; i < dataFile_f.size(); i++ )
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


    bool Add( data_type& prof, const Key& key, index_type& blockIndex) {
        Profile& profile = *prof.value;
        serializeProfile(key,profile);
        if (logger) smsc_log_debug(logger, "Add data block key='%s' length=%u",
                                   key.toString().c_str(),
                                   unsigned(profileData_->size()));
        if ( profileData_->size() <= idxSize() + navSize()) {
            blockIndex = invalidIndex();
            return false;
        }

        DescriptionFile oldDescrFile = descrFile;
        prof.backup->clear(); // reset backup
        if (!backUpProfile(key, notUsed(), prof.backup, profileData_, true)) {
            blockIndex = invalidIndex();
            if (logger) smsc_log_error(logger, "Backup profile error");
            return false;
        }
        // backup contains iFFFFFF

        // extract link information from backup and add it to the new data
        const offset_type newffb = linkBlocks( *prof.backup,
                                               *profileData_,
                                               idx2pos(descrFile.first_free_block) );

        try {
            const offset_type dataStart = writeBlocks(*profileData_,0);
            descrFile.first_free_block = pos2idx(newffb);
            changeDescriptionFile();
            clearBackup();
            blockIndex = pos2idx(dataStart);
            attachBackup(prof.backup,profileData_);
            if (logger) smsc_log_debug(logger, "add(%s) finished with prof.backup=%u",
                                       key.toString().c_str(),
                                       unsigned(prof.backup->size()));
            return true;
        } catch (const std::exception& e) {
            blockIndex = invalidIndex();
            writeBlocks(*prof.backup,0);
            descrFile = oldDescrFile;
            changeDescriptionFile();
            throw;
        }
    }

    
    bool Change( data_type& prof, const Key& key, index_type& blockIndex) {
        if (blockIndex == invalidIndex()) {
            return Add(prof, key, blockIndex);
        }
        Profile& profile = *prof.value;
        serializeProfile(key,profile);
        if (profileData_->size() <= idxSize()+navSize()) {
            // we have to remove it
            Remove(key,blockIndex,prof);
            return true;
        }
        const offset_type blkIndex = idx2pos(blockIndex);
        const size_t newDataSize = profileData_->size(); // remember to remove free chain
        if (logger) 
            smsc_log_debug(logger, "Change data block index=%llx key='%s' oldsize=%u newsize=%u",
                           blkIndex, key.toString().c_str(),
                           unsigned(prof.backup->size()),
                           unsigned(profileData_->size()));
        DescriptionFile oldDescrFile = descrFile;
        offset_type ffb = idx2pos(descrFile.first_free_block);

        // old size is remembered to remove the tail at the end
        const size_t oldBackupSize = prof.backup->size();
        if (!backUpProfile( key, blkIndex, prof.backup, profileData_, true )) {
            if (logger) smsc_log_error(logger, "Backup profile error");
            return false;
        }
        // backup may contain:
        // iNDNDNDiFFFF or iNDNDND

        // link blocks
        const size_t blocksCount = countBlocks(newDataSize);
        const offset_type newffb = linkBlocks(*prof.backup,
                                              *profileData_,
                                              ffb );

        try {
            writeBlocks(*profileData_,0);
            descrFile.first_free_block = pos2idx(newffb);

            //optimization
            if (oldDescrFile.first_free_block != descrFile.first_free_block || oldDescrFile.files_count != descrFile.first_free_block) {
                changeDescriptionFile();
            }
            if (blocksCount == 0) {
                // no new data -- profile has been removed
                blockIndex = invalidIndex(); 
            } else {
                // otherwise, the profile has not been moved.
            }

            profileData_->resize(newDataSize); // remove the tail containing free blocks
            attachBackup( prof.backup, profileData_ );
            if (logger) smsc_log_debug(logger,"change(%s) finished with prof.backup=%u",
                                       key.toString().c_str(),
                                       unsigned(prof.backup->size()));
            return true;
        } catch (const std::exception& e) {
            writeBlocks(*prof.backup,0);
            descrFile = oldDescrFile;
            changeDescriptionFile();
            prof.backup->resize(oldBackupSize);
            Key tmpkey;
            deserializeProfile(tmpkey,*prof.value,*prof.backup);
            throw;
        }

    }
    

    /// load datablocks only
    bool Get( index_type blkIndex, buffer_type& data )
    {
        if (logger) smsc_log_debug(logger, "Get data block index=%llx", blkIndex);
        if (blkIndex == invalidIndex()) return false;
        const offset_type blockIndex = idx2pos(blkIndex);
        const size_t oldSize = data.size();
        {
            // store initial index
            Serializer ser(data);
            ser << blockIndex;
        }
        size_t curpos = data.size();
        size_t dataSize = navSize();
        data.resize(curpos+dataSize);
        bool head = true;
        bool rv = false;
        offset_type curBlockIndex = blockIndex;
        offset_type prevBlockIndex = curBlockIndex;
        do {

            File* f = getFile(curBlockIndex);
            if ( !f ) { break; }
            const off_t offset = getOffset(curBlockIndex);
            f->Seek(offset, SEEK_SET);
            size_t toread = std::min( dataSize, blockSize() );
            if ( toread < navSize() ) { break; }
            f->Read(&((*profileData_)[curpos]),toread);

            // decode navigation
            BlockNavigation bn;
            {
                Deserializer dsr(*profileData_);
                dsr.setrpos(curpos);
                bn.load(dsr);
            }
            curpos += toread;

            if ( head ) {
                // shall be head
                if (!bn.isHead()) {break;}
                if ( bn.dataSize() <= navSize() ) {break;}
                dataSize = bn.dataSize() - toread;
                data.resize(data.size()+dataSize);
                toread = std::min(dataSize,blockSize()-navSize());
                f->Read(&((*profileData_)[curpos]),toread);
                curpos += toread;
            } else {
                // not a head
                if (bn.isFree()) {break;}
                if (bn.refBlock() != prevBlockIndex) {break;}
            }
            dataSize -= toread;

            if ( dataSize == 0 ) {
                // last block has been just read
                if ( bn.nextBlock() != notUsed() ) {break;} // too much data
                rv = true;
                break;
            } else {
                // next block
                prevBlockIndex = curBlockIndex;
                curBlockIndex = bn.nextBlock();
                if ( curBlockIndex == notUsed()) {break;} // too few data
            }

        } while (true);
        if (!rv) { data.resize(oldSize); }
        return rv;
    }


    void recoverFromBackup( data_type& prof )
    {
        Key key;
        deserializeProfile(key,*prof.value,*prof.backup);
    }


    bool Get( index_type blockIndex, data_type& prof )
    {
        if (blockIndex == invalidIndex()) return false;
        if (!profileData_) profileData_ = new buffer_type;
        profileData_->clear();
        if (!Get(blockIndex,*profileData_)) return false;
        Key key;
        deserializeProfile(key,*prof.value,*profileData_);
        attachBackup(prof.backup,profileData_);
        if ( logger ) smsc_log_debug(logger,"get(%llx) finished with prof.backup=%u",
                                     blockIndex,
                                     unsigned(prof.backup->size()));
        return true;
    }


    void Remove( const Key& key, index_type blkIndex, const data_type& prof ) {
        Profile& profile = *prof.value;
        // BlocksHSBackupData& bkp = *prof.backup;
        offset_type blockIndex = idx2pos(blkIndex);
        if (logger) smsc_log_debug(logger, "Remove data block index=%llx key=%s",
                                   blockIndex, key.toString().c_str() );
        if (blockIndex == invalidIndex()) {
            if (logger) smsc_log_warn(logger, "Can't remove data block index=%llx", blockIndex);
            return;
        }
        DescriptionFile oldDescrFile = descrFile;
        offset_type ffb = idx2pos(descrFile.first_free_block);
        if (!backUpProfile(key,blockIndex,prof.backup,0,true)) {
            return;
        }
        // backup contains iNDNDND
        
        buffer_type tmpbuf;
        const offset_type newffb = linkBlocks( *prof.backup,
                                               tmpbuf,
                                               ffb );
        
        try {
            writeBlocks(tmpbuf,0);
            descrFile.first_free_block = pos2idx(newffb);
            changeDescriptionFile();
            clearBackup();
        } catch (const std::exception& e) {
            writeBlocks(*prof.backup,0);
            descrFile = oldDescrFile;
            changeDescriptionFile();
            if (logger) smsc_log_error(logger, "error remove data block index:%d : %s", blockIndex, e.what());
            ::abort();
        }
    }

private:

    /// method counts a number of necessary datablocks for packed buffer.
    /// idx + nav+data [+nav+data ... ]
    inline size_t countBlocks( size_t packedSize ) const {
        return packer_.countBlocks(packedSize);
    }
    /// checks if absolute offset is last in the chain.
    inline offset_type notUsed() const {
        return packer_.notUsed();
    }
    inline size_t navSize() const {
        return packer_.navSize();
    }
    inline size_t idxSize() const {
        return packer_.idxSize();
    }
    inline size_t blockSize() const {
        return packer_.blockSize();
    }
    inline size_t extraSize() const {
        return 32;
    }
    inline index_type pos2idx( offset_type i ) const {
        return packer_.pos2idx(i);
    }
    inline offset_type idx2pos( index_type i ) const {
        return packer_.idx2pos(i);
    }


    /// method process oldprofile ([iNDNDND][iFFFF])
    /// and fill newprofile with header data => ([iNDNDND][iFFFF])
    offset_type linkBlocks( buffer_type& oldprofile,
                            buffer_type& newprofile,
                            offset_type ffb )
    {
        std::vector< offset_type > blocks;
        blocks.reserve( (countBlocks(newprofile.size())+2)*2 );
        // offset_type newffb = ffb;
        offset_type newffb = packer_.extractBlocks(oldprofile,blocks,ffb);
        if ( newffb == notUsed() ) {
            // if newffb is notUsed(), it means that we reached end-of-free-chain
            // let's create a new file.
            newffb = descrFile.files_count * fileSizeBytes_;
            CreateDataFile();
        }

        std::vector< offset_type > offsets;
        packer_.extractOffsets(blocks,offsets);

        const size_t needBlocks = countBlocks(newprofile.size());
        if ( needBlocks > 0 ) {
            // headers should be filled
            buffer_type headers;
            packer_.makeHeaders( headers, offsets, newprofile.size()-idxSize() );
            packer_.mergeHeaders(newprofile,headers);
        }

        if ( offsets.size() > needBlocks ) {
            // free blocks should be chained also
            newffb = packer_.makeFreeChain(newprofile,offsets,needBlocks,newffb);
        }
        return newffb;
    }


    inline File* getFile(offset_type absoluteOffset) const {
        offset_type fn = absoluteOffset / fileSizeBytes_;
        if ( fn < size_t(descrFile.files_count) ) {
            if ( absoluteOffset % blockSize() != 0 ) {
                if (logger) smsc_log_error(logger,"wrong offset passed, did you forgot idx2pos()?");
                ::abort();
            }
            return dataFile_f[fn];
        }
        if (logger) smsc_log_error( logger, "Invalid file number %lld, max file number %u",
                                    fn, unsigned(descrFile.files_count-1) );
        return 0;
    }


    inline off_t getOffset(offset_type absoluteOffset) const {
        return absoluteOffset % fileSizeBytes_;
    }


    inline void attachBackup( buffer_type*& oldbackup, buffer_type*& newbackup ) const {
        // FIXME: add consumption check, i.e. if new backup capacity is too big
        // then resize it.
        std::swap(oldbackup,newbackup);
    }


    /// recovery procedures
    class Recovery 
    {
    public:
        Recovery( BlocksHSStorage3& bhs ) : bhs_(bhs), df_(bhs_.descrFile) {
            if ( df_.files_count <= 0 ) {
                if (bhs_.logger) smsc_log_error(bhs_.logger,"cannot recover as files_count = 0");
                throw smsc::util::Exception("cannot recover as files_count = 0");
            }
            fixup_.reset(new FixupLogger(bhs_.dbPath + '/' + bhs_.dbName + ".fixup"));
            std::vector< unsigned char > buf;
            Serializer ser(buf);
            df_.serialize(ser);
            fixup_->makeComment(2,ser.size(),ser.data());
            lastFreeBlock_ = df_.first_free_block = bhs_.invalidIndex();
            // df_.blocks_free = 0;
            // df_.blocks_used = df_.files_count * df_.file_size;
        }

        ~Recovery() {
            // we have to write down a new description file
            bhs_.changeDescriptionFile();
        }

        /// the methods frees a chain of used blocks
        void freeBlocks( offset_type blockIndex, DataBlockHeader& hdr ) {
            /*
             * FIXME: restore
            const offset_type totalBlocks = df_.files_count * df_.file_size;
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
             */
        }

        void chainFreeBlock( offset_type blockIndex, DataBlockHeader& hdr ) {
            /*
            freeBlock( blockIndex, hdr.next_free_block );
             */
        }

    private:
        void freeBlock( offset_type blockIndex, offset_type saved_next_free_block )
        {
            /*
            EndianConverter cvt;
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
            fixup_->save(offset,8,cvt.set(uint64_t(saved_next_free_block)));
            File* f = bhs_.dataFile_f[fn];
            f->Seek(offset);
            f->WriteNetInt64(bhs_.invalidIndex());
            lastFreeBlock_ = blockIndex;
            ++df_.blocks_free;
            --df_.blocks_used;
         */
        }

    private:
        BlocksHSStorage3&           bhs_;
        DescriptionFile&           df_;
        std::auto_ptr<FixupLogger> fixup_;
        offset_type                 lastFreeBlock_;
    };

public:
    /// used to iterate over BlocksHSStorage.
    /// NOTE: modification operation may invalidate the iterator!
    class Iterator {
        friend class BlocksHSStorage3;
    private:
        Iterator( BlocksHSStorage3& bhs, Recovery* rec = 0 ) :
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
            const index_type cnt = bhs_->descrFile.files_count * bhs_->descrFile.file_size;
            while ( iterBlockIndex_ < cnt ) {
                curBlockIndex_ = iterBlockIndex_++;
                if ( bhs_->Get(curBlockIndex_,blockData_) ) {
                    // FIXME: deser key
                    return true; 
                }
            }
            return false;
        }
        
        inline index_type blockIndex() const { return curBlockIndex_; }
        inline const DataBlock& blockData() const { return blockData_; }
        inline const Key& key() const { return key_; }
        
    private:
        BlocksHSStorage3*                bhs_;
        index_type                       iterBlockIndex_;
        std::auto_ptr<Recovery>          recovery_;

        // data which are set as a result of a positive next() call.
        index_type                       curBlockIndex_;
        buffer_type                      blockData_;
        Key                              key_;
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

    /*
  template<class T> void deserialize(File* f, T& hdr) {
      unsigned needsize = T::persistentSize();
      if ( deserBufSize_ < needsize ) {
          delete [] deserBuf_;
          deserBuf_ = new unsigned char[deserBufSize_ = needsize];
      }
      f->Read( (void*)deserBuf_, needsize );
      Deserializer ds( deserBuf_, needsize );
      hdr.deserialize(ds);
  }
     */

    void printHdr( const DataBlockHeader& hdr ) {
        if (!logger) return;
        smsc_log_debug( logger, "block header %s next=%llx ref=%llx",
                        hdr.isFree() ? "free" : ( hdr.isHead() ? "head" : "used" ),
                        hdr.nextBlock(), hdr.refBlock() );
    }


    void writeDataBlock( int fileNumber, off_t offset,
                         const void* data, size_t dataSize )
    {
        if (logger)
            smsc_log_debug(logger, "write data block fn=%d, offset=%d, blockSize=%d", fileNumber, offset, dataSize);
        /*
         * NOTE: fn and offset must be already checked
        if ( !checkfn(fileNumber) ) {
            throw smsc::util::Exception("Invalid file number %d, max file number %d", fileNumber, descrFile.files_count-1);
        }
         */
        /*
        size_t bufSize = hdrSize + curBlockSize;
        // memset(&(serHdrBuf_[0]), 0, DataBlockHeader::persistentSize());
        Serializer ser(serHdrBuf_);
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
         */
        try {
            File* f = dataFile_f[fileNumber];
            f->Seek(offset,SEEK_SET);
            f->Write(data,dataSize);
        } catch ( std::exception& e ) {
            if (logger)
                smsc_log_warn(logger, "Error writing block to data file %u, std::exception: '%s'",
                              fileNumber, e.what());
            throw;
        }
    }


    /// 
    bool backUpProfile( const Key& key,
                        offset_type blockIndex,
                        buffer_type* oldprofile,
                        const buffer_type* newprofile,
                        bool saveToFile = false )
    {
        if (logger && logger->isDebugEnabled()) {
            HexDump hd;
            std::string ohex;
            std::string nhex;
            if ( oldprofile && oldprofile->size() > 0 ) {
                hd.hexdump(ohex,&((*oldprofile)[0]), oldprofile->size());
            }
            if ( newprofile && newprofile->size() > 0 ) {
                hd.hexdump(nhex,&((*newprofile)[0]), newprofile->size());
            }
            smsc_log_debug( logger, "backup profile key=%s index=%llx, old=%s new=%s",
                            key.toString().c_str(), blockIndex, ohex.c_str(), nhex.c_str() );
        }

        size_t blocksCount = newprofile ? countBlocks(newprofile->size()) : 0;
        if ( blockIndex == notUsed() && blocksCount == 0 ) {
            return true; 
        }

        DescriptionFile oldDescrFile = descrFile;
        printDescrFile();
        const size_t oldProfileSize = oldprofile ? oldprofile->size() : 0;
        try {
            if ( oldProfileSize == 0 ) {
                assert( blockIndex == notUsed() );
                buffer_type backupbuf;
                if (!oldprofile) oldprofile = &backupbuf;
                // backupHeader.blocksCount = blocksCount;
                // backupHeader.dataSize = 0;
                // backupHeader.curBlockIndex = oldDescrFile.first_free_block;
                readFreeBlocks(*oldprofile,blocksCount);
                if (saveToFile) {
                    saveBackupToFile(oldDescrFile,*oldprofile);
                }
                if (logger) smsc_log_debug( logger, "backup profile blocks=%u size=%u", unsigned(blocksCount), unsigned(newprofile->size()));
                return true;
            }
            // backupHeader.curBlockIndex = blockIndex;
            if (!oldprofile) {
                if (logger) smsc_log_warn(logger, "Error backup profile: key=%s backup is empty", key.toString().c_str() );
                return false;
            }
            // backupHeader.dataSize = oldprofile->size() - idxSize();
            // dataBlockBackup = profile.getBackup();
            const size_t backupSize = countBlocks(oldprofile->size());
            // backupHeader.blocksCount = blocksCount >= backupSize ? blocksCount : backupSize;
            if (backupSize < blocksCount) {
                // dataBlockBackup[backupSize - 1] = oldDescrFile.first_free_block;
                // reading several more free blocks
                readFreeBlocks(*oldprofile,blocksCount-backupSize);
            }
            if (logger) smsc_log_debug(logger, "backup profile key=%s blocks=%u",
                                       key.toString().c_str(), unsigned(blocksCount));
            if (saveToFile) {
                saveBackupToFile(oldDescrFile,*oldprofile);
            }
        } catch (const std::exception& e) {
            if (logger) smsc_log_warn(logger, "Error backup profile key=%s. std::exception: '%s'",
                                      key.toString().c_str(), e.what());
            // dataBlockBackup.clear();
            descrFile = oldDescrFile;
            changeDescriptionFile();
            if (oldprofile) oldprofile->resize(oldProfileSize);
            ::abort();
            // FIXME: if abort will be replaced eventually with return false
            // you'll have to uncomment the following:
            // if (oldprofile) oldprofile->resize(oldProfileSize);
        }
        return true;
    }


    /// write chained blocks to the disk.
    ///  @param key              -- the key of the profile;
    ///  @param newdata          -- new data chain
    ///  @param ffb              -- is necessary to link free chain at the end
    ///  @return the index of the first block in the chain.
    offset_type writeBlocks( const buffer_type& buffer, size_t position )
    {
        if ( buffer.size() <= position ) return notUsed();

        std::vector< offset_type > affectedBlocks;
        const size_t needBlocks = countBlocks(buffer.size()-position);
        assert(needBlocks > 0);
        affectedBlocks.reserve(needBlocks+20);
        offset_type rv = notUsed();
        packer_.extractBlocks(buffer,affectedBlocks,notUsed(),position);

        size_t pos = position;
        for ( std::vector<offset_type>::const_iterator i = affectedBlocks.begin();
              i != affectedBlocks.end();
              ++i ) {

            const offset_type idx = *i;
            const size_t dataSize = *++i;
            if (idx == notUsed() || dataSize == 0) {
                // skip dummy and go on
                pos += idxSize();
                continue;
            } else if ( rv == notUsed() ) {
                rv = idx;
            }
            
            File* f = getFile(idx);
            if ( !f ) {
                if (logger) smsc_log_error(logger,"addblocks failed, idx=%llx", idx);
                ::abort();
            }
            f->Seek( getOffset(idx) );
            f->Write( &buffer[pos], dataSize );
            pos += dataSize;

        }
        return rv;
    }


    /// NOTE: this method is buggy, threshould may be reached several times!
    /*
    offset_type getFirstFreeBlock(int fileNumber, off_t offset)
    {
        File* f = dataFile_f[fileNumber];
        f->Seek(offset, SEEK_SET);
        offset_type ffb = f->ReadNetInt64();
        dataFileCreator_.create(ffb, descrFile.files_count);
        if (ffb == -1) {
            ffb = descrFile.files_count * descrFile.file_size;
            CreateDataFile();
        }
        if (logger) smsc_log_debug(logger, "First free block %d", ffb);
        return ffb;
    }
     */


    /*
    offset_type removeBlocks(offset_type ffb, offset_type blockIndex, size_t backupIndex) {
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
     */


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
        // recreate packer
        packer_ = HSPacker(_blockSize,0,logger);
        fileSizeBytes_ = _blockSize*_fileSize;

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
        // effectiveBlockSize = descrFile.block_size - hdrSize;
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
        // descrFile.blocks_free = descrFile.file_size;
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
            buffer_type buf;
            Serializer ser(buf);
            ser.setwpos(4);
            descrFile.serialize(ser);
            ser.setwpos(0);
            ser << uint32_t(ser.size());
            writeBuffer(descrFile_f,buf);
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
            buffer_type buf;
            Serializer ser(buf);
            ser.setwpos(4);
            descrFile.serialize(ser);
            ser.setwpos(0);
            ser << uint32_t(ser.size());
            writeBuffer(tmpFile,buf);
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


    void loadBackupData( bool ignoreCompleteFlag = false ) {
        try {
            // dataBlockBackup.clear();
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

            buffer_type hdrbuf;

            hdrbuf.reserve(512);
            hdrbuf.resize(5);
            hdrbuf[0] = trx;
            backupFile_f.Read(&hdrbuf[1],4);
            Deserializer dsr(hdrbuf);
            dsr.setrpos(1);
            uint32_t wsize;
            dsr >> wsize;
            if (wsize <= 5) throw smsc::util::Exception("too few data in backup: %u", wsize);
            hdrbuf.resize(wsize);
            backupFile_f.Read(&hdrbuf[5],wsize-5);
            uint64_t csum1, csum2;
            dsr >> csum1;
            deserializeDescr(dsr,descrFile);
            // deserializeBackup(dsr,backupHeader);
            dsr >> wsize;
            const size_t dataStart = dsr.rpos();
            dsr.setrpos(dataStart+wsize);
            dsr >> csum2;
            if ( csum1 != csum2 ) {
                throw smsc::util::Exception("control sum mismatch %llx != %llx", csum1, csum2 );
            }

            // erase control sum
            hdrbuf.erase(hdrbuf.begin()+dataStart+wsize,hdrbuf.end());

            // deserialize(&backupFile_f, descrFile);
            // deserialize(&backupFile_f, backupHeader);
            // buffer_type tmpbuf;
            // tmpbuf.resize(backupHeader.backupSize);

            /*
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
             */
            writeBlocks(hdrbuf,dataStart);

            // FIXME: restore (write back) descrFile
            // FIXME: should it be done with changeDescriptionFile(); ?
            return;
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "Error loading transaction data: '%s'", e.what());
            // dataBlockBackup.clear();
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
            {
                buffer_type tmpbuf;
                Deserializer dsr(tmpbuf);
                dsr.setrpos( readBuffer(descrFile_f,tmpbuf) );
                deserializeDescr(dsr,descrFile);
            }

            if (logger) smsc_log_info(logger,"opening descr: block_size=%u file_size=%u files_count=%u ffb=%llx",
                                      unsigned(descrFile.block_size),
                                      unsigned(descrFile.file_size),
                                      unsigned(descrFile.files_count),
                                      index_type(descrFile.first_free_block) );

            packer_ = HSPacker(descrFile.block_size,0,logger);
            fileSizeBytes_ = descrFile.block_size * descrFile.file_size;

            if (descrFile.version != dfVersion_64_2) {
              if (logger) smsc_log_error(logger, "FSStorage: invalid version of data storage:0x%x, correct version:0x%x", descrFile.version, dfVersion_64_2);
              return CANNOT_OPEN_EXISTS_DESCR_FILE;
            }

            if (logger) smsc_log_info(logger, "OpenDescrFile: storage version:0x%x, files count:%d, block size:%d, file size:%d, first free block:%lld",
                          descrFile.version, descrFile.files_count, descrFile.block_size, descrFile.file_size, descrFile.first_free_block);
            // effectiveBlockSize = descrFile.block_size - hdrSize;
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


    void writeBuffer( File&f, buffer_type& buf )
    {
        assert(buf.size() > 4);
        f.Write(&buf[0],buf.size());
    }


    /// return the position in buffer, where to start deserialization
    size_t readBuffer( File& f, buffer_type& buf, size_t hintSize = 0 )
    {
        size_t rpos = buf.size();
        hintSize += 4;
        buf.resize(rpos+hintSize);
        buf.resize(rpos+f.Read(&buf[rpos],hintSize));
        uint32_t wsize;
        {
            Deserializer dsr(buf);
            dsr.setrpos(rpos);
            dsr >> wsize;
        }
        const size_t prevpos = rpos+4;
        const size_t needsize = rpos+wsize;
        while ( needsize > buf.size() ) {
            const size_t curpos = buf.size();
            buf.resize(needsize);
            buf.resize(curpos + f.Read(&buf[curpos],needsize-curpos));
        }
        return prevpos;
    }


    int OpenDataFiles(void)
    {
        if (descrFile.files_count <= 0) {
            if (logger) smsc_log_error(logger, "Open data files error: files count=%d", descrFile.files_count);
            return CANNOT_OPEN_DATA_FILE;
        }
        char buff[10];
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

    /*
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
     */


    void writeBackup( File& f, const DescriptionFile& _descrFile, const buffer_type& backupData )
    {
        // serialization
        buffer_type tmpbuf;
        {
            tmpbuf.reserve(backupData.size()+256);
            Serializer ser(tmpbuf);
            ser << uint8_t(TRX_INCOMPLETE);
            ser << uint32_t(0);
            const uint64_t csum = rnd_.getNextNumber();
            ser << csum;
            serializeDescr(ser,_descrFile);
            // serializeBackup(ser,backupHeader);
            ser << uint32_t(tmpbuf.size());
            tmpbuf.insert(tmpbuf.end(),backupData.begin(),backupData.end());
            ser << csum;
            ser.setwpos(0);
            ser << uint32_t(tmpbuf.size());
        }

        /*
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
         */
        f.Seek(0);
        f.Write(&tmpbuf[0], tmpbuf.size());
    }


    void saveBackupToFile( const DescriptionFile& oldDescr, const buffer_type& backupData )
    {
        try {
            //if (logger) smsc_log_debug(logger, "Save transaction data to file");
            //printDescrFile();
            writeBackup(backupFile_f, oldDescr, backupData);
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
            writeBackup(tmpFile, oldDescr, backupData);
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "%s", e.what());
        }
        ::abort();
    }


    void clearBackup() 
    {
        try {
            if (logger) smsc_log_debug(logger, "clear backup");
            backupFile_f.Seek(0);
            backupFile_f.WriteByte(TRX_COMPLETE);
            // dataBlockBackup.clear();
        } catch (const std::exception& e) {
            if (logger) smsc_log_error(logger, "Error clear transaction data: '%s'", e.what());
        }
    }


    bool deserializeProfile( Key& key, Profile& profile, buffer_type& buffer )
    {
        if (!profileData_) return false;
        // unpack buffer
        buffer_type headers;
        packer_.unpackBuffer(buffer,&headers);
        bool rv = false;
        try {
            Deserializer dsr(*profileData_,glossary_);
            dsr.setrpos(idxSize()+navSize()+extraSize());
            dsr >> key;
            dsr >> profile;
            rv = true;
        } catch (std::exception& e) {
            if (logger) smsc_log_warn(logger,"profile cannot be deserialized: %s", e.what());
        }
        packer_.packBuffer(buffer,&headers);
        return rv;
    }


    unsigned serializeProfile( const Key& key, const Profile& profile )
    {
        if (!profileData_) profileData_ = new buffer_type;
        profileData_->clear();
        profileData_->reserve(blockSize());
        Serializer ser(*profileData_,glossary_);
        ser.setwpos(idxSize()+navSize()+extraSize());
        ser << key;
        const size_t wpos = ser.wpos();
        ser << profile;
        if ( wpos+4 >= ser.wpos() ) {
            // profile has not been serialized -- no data
            profileData_->clear();
        } else {
            packer_.packBuffer(*profileData_,0);
        }
        return profileData_->size();
    }


    void deserializeDescr( Deserializer& dsr, DescriptionFile& descr ) const
    {
        descr.deserialize(dsr);
    }


    void serializeDescr( Serializer& ser, const DescriptionFile& descr ) const
    {
        descr.serialize(ser);
    }


    void readFreeBlocks( buffer_type& buffer, size_t blocksCount )
    {
        if (logger) smsc_log_debug(logger, "Read %u free blocks", unsigned(blocksCount));
        offset_type curBlockIndex = idx2pos(descrFile.first_free_block);
        const size_t oldSize = buffer.size();
        buffer.resize(buffer.size()+blocksCount*navSize()+idxSize());
        Deserializer dsr(buffer);
        dsr.setrpos(oldSize+idxSize());
        offset_type prevBlockIndex = idx2pos(invalidIndex());
        for ( size_t i = 0; i < blocksCount; ++i ) {

            if ( curBlockIndex == notUsed() ) {
                // need a new file
                curBlockIndex = descrFile.files_count * fileSizeBytes_;
                CreateDataFile();
            } else {
                dataFileCreator_.create( pos2idx(curBlockIndex), descrFile.files_count, true );
            }

            if ( i == 0 ) {
                // write initial block
                Serializer ser(buffer);
                ser.setwpos(oldSize);
                ser << curBlockIndex;
            }

            File* f = getFile(curBlockIndex);
            if ( !f ) {
                throw smsc::util::Exception( "invalid blk: %llx, max: %llx", 
                                             curBlockIndex, fileSizeBytes_*descrFile.files_count );
            }
            // const int file_number = getFileNumber(curBlockIndex);
            const off_t offset = getOffset(curBlockIndex);
            f->Seek(offset);
            f->Read(&buffer[dsr.rpos()],navSize());
            
            // reading block navigation
            BlockNavigation bn;
            bn.load(dsr);
            if (!bn.isFree()) {
                if (logger) smsc_log_error(logger,
                                           "non-free block found, storage corruption: prev:%llx, this:%llx",
                                           prevBlockIndex,
                                           curBlockIndex );
                ::abort();
            }
            // FIXME: add a check for chain consistency
            curBlockIndex = bn.nextBlock();
        }
    }


    bool findFirstFreeBlock() 
    {
        try {
            if (logger) smsc_log_warn(logger, "try to find first free block");
            offset_type lastBlock = descrFile.files_count * fileSizeBytes_ - 1;
            File* f = getFile(lastBlock);
            if (!f) {
                smsc_log_error(logger, "can't find first free block: last block index is invalid %llx", lastBlock);
                return false;
            }
            off_t offset = getOffset(lastBlock);
            f->Seek(offset, SEEK_SET);
            BlockNavigation bn;
            bn.load( *f );
            if ( !bn.isFree() ) {
                smsc_log_error(logger, "can't find first free block: last block header is not free");
                return false;
            }
            for ( offset_type curBlockIndex = lastBlock; curBlockIndex >= 0; curBlockIndex -= blockSize() ) {
                f = getFile(curBlockIndex);
                if (!f) {
                    smsc_log_error(logger, "can't find first free block: current block index is invalid %llx", curBlockIndex);
                    return false;
                }
                offset = getOffset(curBlockIndex);
                f->Seek(offset, SEEK_SET);
                bn.load(*f);
                if ( !bn.isFree() ) {
                    smsc_log_error(logger, "first used block found: %llx, first free block will be: %llx", curBlockIndex, lastBlock );
                    descrFile.first_free_block = lastBlock;
                    changeDescriptionFile();
                    if (logger) smsc_log_info(logger, "first free block fixed: %llx", descrFile.first_free_block );
                    return true;
                }
            }
        } catch (const std::exception& ex) {
            if (logger) smsc_log_error(logger, "Error finding first free block: %s", ex.what());
        }
        smsc_log_error(logger, "first free block not found");
        return false;
    }


    bool checkFirstFreeBlock() 
    {
        try {
            offset_type curBlockIndex = idx2pos(descrFile.first_free_block);
            if (logger) smsc_log_info(logger, "check first free block: %llx", curBlockIndex);
            if ( notUsed() == curBlockIndex ) {
                // FIXME: it should be quite ok
                smsc_log_error(logger, "first free block invalid: %llx, can't fix first free block", curBlockIndex);
                return false;
            }
            File* f = getFile(curBlockIndex);
            if ( !f ) {
                smsc_log_error(logger, "first free block invalid: %llx, can't fix first free block", curBlockIndex);
                return false;
            }
            const off_t offset = getOffset(curBlockIndex);
            f->Seek(offset, SEEK_SET);
            BlockNavigation bn;
            bn.load(*f);
            const offset_type maxBlockIndex = descrFile.files_count * fileSizeBytes_;

            if ( bn.isFree() ) {
                if ( (notUsed() == bn.nextBlock()) || (bn.nextBlock() < maxBlockIndex) ) {
                    if (logger) smsc_log_info(logger, "first free block: %llx valid, next free block: %llx", curBlockIndex, bn.nextBlock());
                    return true;
                }
            
                if (logger)  smsc_log_info(logger, "first free block: %llx invalid, try to find correct ffb", curBlockIndex);
            } else {
                // used
                if (logger) smsc_log_warn(logger, "first free block: %llx used, try to find correct first free block...", curBlockIndex);
            }

            ++curBlockIndex;
            while (curBlockIndex < maxBlockIndex) {
                File* f = getFile(curBlockIndex);
                if (!f) {
                    if (logger) smsc_log_error(logger, "can't fix first free block: %llx", curBlockIndex);
                    return false;
                }
                const off_t offset = getOffset(curBlockIndex);
                f->Seek(offset, SEEK_SET);
                BlockNavigation bn;
                bn.load(*f);

                if ( bn.isFree() ) {
                    if ( (notUsed() == bn.nextBlock()) || (bn.nextBlock() < maxBlockIndex) ) {
                        if (logger) smsc_log_info(logger, "correct first free block found: %llx, next free block: %llx", curBlockIndex, bn.nextBlock());
                        descrFile.first_free_block = pos2idx(curBlockIndex);
                        changeDescriptionFile();
                        if (logger) smsc_log_info(logger, "first free block fixed: %llx", descrFile.first_free_block );
                        return true;
                    }
                }
                ++curBlockIndex;
            } // while
            if (logger) smsc_log_error(logger, "can't fix first free block: %llx", curBlockIndex);
        } catch (const std::exception& ex) {
            if (logger) smsc_log_error(logger, "Error checking first free block: %s", ex.what());
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
    // int effectiveBlockSize;

    // vector<index_type> dataBlockBackup;
    // char writeBuf[WRITE_BUF_SIZE];
    // char backupBuf[BACKUP_BUF_SIZE];
    // size_t hdrSize;
    // SerialBuffer profileData;

    /// NOTE: a pointer is used to allow fast switch w/ profile backup.
    buffer_type* profileData_;

    // CompleteDataBlock completeDataBlock;
    // BackupHeader backupHeader;

    // std::vector< unsigned char > serDescrBuf_;
    // std::vector< unsigned char > serBackupBuf_;
    // std::vector< unsigned char > serHdrBuf_;
    // unsigned char* deserBuf_;
    // unsigned deserBufSize_;
    DataFileCreator dataFileCreator_;

    HSPacker packer_;
    size_t   fileSizeBytes_;
    Drndm    rnd_;
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE3_H */
