//------------------------------------
//  RBTreeHSAllocator.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ RBTreeHSAllocator.
//

#ifndef _SCAG_UTIL_STORAGE_RBTREEHSALLOCATOR_H
#define _SCAG_UTIL_STORAGE_RBTREEHSALLOCATOR_H

#include "RBTreeAllocator.h"
#include <string>
#include <list>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "logger/Logger.h"
#include "core/buffers/File.hpp"
#include "scag/util/storage/Serializer.h"

namespace scag {
namespace util {
namespace storage {

using namespace std;

// NOTE: we have a problem with a serialization of a RBTreeNode here.
// The problem is that it is a template class and so it could have different Key and Value types.
// And we need to serialize it differently for different types and versions.
// But typically Value is an integral type.
template < typename Key >
    class RBTreeSerializer
{
public:
    inline void serialize( Serializer& s, const Key& k ) {
        s << k;
    }
    inline void deserialize( Deserializer& d, Key& k ) {
        d >> k;
    }
};

// partial specialization for long
template <> void RBTreeSerializer< long >::serialize( Serializer& s, const long& k ) {
    int64_t x = k; s << x;
}
template <> void RBTreeSerializer< long >::deserialize( Deserializer& d, long& k ) {
    int64_t x; d >> x; k = static_cast<long>(x);
}


template< class Key = long, class Value = long, class KS = RBTreeSerializer<Key>, class VS = RBTreeSerializer<Value> >
    class RBTreeHSAllocator: public RBTreeAllocator<Key, Value>, public RBTreeChangesObserver<Key, Value>
{
protected:
    typedef typename RBTreeAllocator<Key,Value>::RBTreeNode RBTreeNode;

public:
    static const int32_t version_32_1 = 0x01;             // prehistoric version
    static const int32_t version_64_1 = 0x80000001;       // prehistoric version
    static const int32_t version_current = 2;

private:
    struct RbtFileHeader   // transient representation
    {
        RbtFileHeader() :
        version(version_current),
        cells_count(0), cells_used(0),
        cells_free(0), root_cell(0), first_free_cell(0), nil_cell(0), growth(0) {}
    public:
        // char preamble[20];                   // 20
        int32_t version;                     // 4
        int32_t cells_count;                 // 4
        int32_t cells_used;                  // 4
        int32_t cells_free;                  // 4
        int64_t root_cell;                   // 8
        int64_t first_free_cell;             // 8 an offset (transient) to a first free cell
        int64_t nil_cell;                    // 8
        int64_t growth;                      // 8
        // char reserved[160];               // 160

        int32_t persistentCellSize;          // this is not a persistent field
    };

    static const int32_t STAT_OK = 0;
    static const int32_t STAT_WRITE_TRX = 1;
    static const int32_t STAT_WRITE_RBT = 2;
    static const int32_t TRX_VER_1 = 1;

    struct TransFileHeader  // transient representation
    {
        /// NOTE: serialization does not include the status of the trans file header
        // int32_t status;
        // int32_t version;
        int32_t operation;
        int32_t nodes_count;
    };

public:
    static const int mode = 0600;
    static const off_t	default_growth = 1000;

    static const int SUCCESS			= 0;
    static const int SUCCESS_PREV_OPER_FAILED	= 1;
	
    static const int RBTREE_FILE_NOT_SPECIFIED	= -1;
    static const int CANNOT_CREATE_RBTREE_FILE	= -2;
    static const int CANNOT_OPEN_RBTREE_FILE	= -3;
    static const int BTREE_FILE_MAP_FAILED		= -4;
    static const int CANNOT_CREATE_TRANS_FILE	= -5;
    static const int CANNOT_OPEN_TRANS_FILE		= -6;
    static const int CANNOT_READ_TRANS_FILE		= -7;
    static const int RBTREE_FILE_ERROR		= -10;
    static const int ATTEMPT_INIT_RUNNING_PROC	= -20;

    RBTreeHSAllocator( smsc::logger::Logger* thelog ):
    rbtree_body_(0),
    growth(100000000),
    running(false),
    currentOperation(0),
    logger(thelog),
    rbtFileHeaderDump_(rbtFileHeaderBuf_)
    {
    }

    // void Create(void){}
    // void Open(void){}
    // void Close(void){}

    int Init(const string& _rbtree_file="", off_t _growth = default_growth, bool clearFile = false)
    {
        if(running) return ATTEMPT_INIT_RUNNING_PROC;
        if(_rbtree_file.length()==0)
            return RBTREE_FILE_NOT_SPECIFIED;
		
        rbtree_file = _rbtree_file;
        trans_file = rbtree_file + ".trx";
        if(_growth > 0) growth = _growth;
		
        if(!isFileExists() || clearFile)
        {
            const int ret = CreateRBTreeFile();
            if ( ret < 0 ) return ret;
        }
        else
        {
            const int ret = OpenRBTreeFile();
            if (ret < 0 ) return ret;
        }
        running = true;
        return SUCCESS;
    }

    virtual ~RBTreeHSAllocator()
    {
        if (rbtree_body_) delete rbtree_body_;
        if(running) {
            running = false;
        }
    }

    virtual RBTreeNode* allocateNode(void)
    {
        if(!running) return 0;
        RBTreeNode* newNode;
        if (!header_.cells_free && ReallocRBTreeFile() != SUCCESS)
            abort();
        newNode = addr2node( header_.first_free_cell );
        if (logger) smsc_log_debug(logger, "allocateNode rbtree_body_ = %p, header_.first_free_cell = %lld, node=%p", rbtree_body_, header_.first_free_cell, newNode );
        header_.first_free_cell = (int64_t)newNode->parent;
        newNode->parent = newNode->left = newNode->right = 0;
        header_.cells_used++;
        header_.cells_free--;
        return newNode;
    }

    virtual void releaseNode(RBTreeNode* node)
    {
        if(!running) return;
        node->parent = (RBTreeNode*)header_.first_free_cell;
        header_.first_free_cell = node2addr(node);
        header_.cells_used--;
        header_.cells_free++;
    }

    virtual RBTreeNode* getRootNode(void)
    {
        if (!running) return 0;
        if (-1 == header_.root_cell)	return 0;
        return addr2node(header_.root_cell);
    }

    virtual void setRootNode(RBTreeNode* node)
    {
        if (!running) return;
        header_.root_cell = node2addr(node);
		
        if (logger) smsc_log_debug(logger, "SetRoot node = %p, header_.root_cell=%lld", node, header_.root_cell);
        //printf("SetRoot (long)node = %X", (long)node);
        //printf("(long)rbtree_body_ = %X", (long)rbtree_body_);
        //printf("((long)node - (long)rbtree_body_) = %d", ((long)node - (long)rbtree_body_));
        //printf("header_.root_cell = %d\n", header_.root_cell);
    }

    virtual RBTreeNode* getNilNode()
    {
        if (!running) return 0;
        return addr2node(header_.nil_cell);
    }

    virtual long getSize(void) const
    {
        if (!running) return 0;
        return header_.cells_used;
    }

    virtual long getOffset(void) const
    {
        if (!running) return 0;
        return long(rbtree_body_);
    }

    virtual void startChanges(RBTreeNode* node, int operation)
    {
        if ( changedNodes.size() > 0 ) completeChanges();
        if (logger) smsc_log_debug(logger, "startChanges. node = (%lld)%p, operation = %d", node2addr(node), node, operation);
        currentOperation = operation;
        changedNodes.push_back(node);
    }

    virtual void nodeChanged(RBTreeNode* node)
    {
        //	    smsc_log_debug(logger, "Node changed=%d(%p)", node2addr(node), node);
        changedNodes.push_back(node);
    }

    virtual void completeChanges(void)
    {
        //	    smsc_log_debug(logger, "completeChanges=%d", changedNodes.size());
        // changedNodes.sort();
        // changedNodes.unique();
        //printf("%d\n", changedNodes.size());
        startTransaction();
        writeChanges();
        endTransaction();
        changedNodes.erase(changedNodes.begin(), changedNodes.end());
    }

    /// a temporary method to check free cells integrity
    virtual std::vector< RBTreeNode* > freenodes()
    {
        // check header integrity
        if ( header_.cells_count != header_.cells_used + header_.cells_free ) {
            throw smsc::util::Exception("RBTreeAlloc: cells_count != cells_used + cells_free");
        }
        /*
        if ( rbtFileLen != sizeof(RbtFileHeader) + header_.cells_count * sizeof(RBTreeNode) ) {
            throw smsc::util::Exception("RBTreeAlloc: rbtFileLen mismatch (!= cells_count * sizeof(RBTreeNode))");
        }
         */

        // check free cells integrity
        int64_t rbtree_len = index2addr(header_.cells_count);
        int64_t celladdr = header_.first_free_cell;
        std::vector< RBTreeNode* > fnl;
        fnl.reserve( header_.cells_free );
        for ( int32_t cf = 0; cf < header_.cells_free; ++cf ) {
            // if ( cf < 20 )
            // smsc_log_debug( logger, "free cell #%d has address %ld", cf, celladdr );
            if ( celladdr >= rbtree_len ) {
                if (logger) smsc_log_error( logger, "free cell %d address is too big=%lld filelen=%lld", cf, celladdr, rbtree_len );
                // fprintf( stderr, "free cell #d address is too big=%ld filelen=%lld\n", cf, celladdr, (long long)rbtFileLen );
                throw smsc::util::Exception( "RBTreeAlloc: free cell %d address is too big %lld filelen=%lld", cf, celladdr, rbtree_len );
                break;
            }
            RBTreeNode* cell = addr2node(celladdr);
            fnl.push_back(cell);
            celladdr = int64_t(cell->parent);
        }
        std::sort( fnl.begin(), fnl.end() );
        const size_t fnc = fnl.size();
        fnl.erase( std::unique( fnl.begin(), fnl.end() ), fnl.end() );
        if ( fnc != fnl.size() ) {
            if (logger) smsc_log_error( logger, "non-unique nodes have been found!" );
            // fprintf( stderr, "non-unique nodes have been found!\n" );
            throw smsc::util::Exception( "RBTreeAlloc: %d non-unique nodes have been found!", fnc - fnl.size() );
        }
        return fnl;
    }


private:
    inline RBTreeNode* addr2node( int64_t offset ) const
    {
        return reinterpret_cast< RBTreeNode* >( int64_t(rbtree_body_) + offset );
    }
    inline int64_t node2addr( const RBTreeNode* node ) const
    {
        return int64_t(node) - int64_t(rbtree_body_);
    }
    inline int32_t addr2index( int64_t a ) const {
        return a / sizeof(RBTreeNode);
    }
    inline int64_t index2addr( int32_t i ) const {
        return int64_t(i) * sizeof(RBTreeNode);
    }

    bool isFileExists(void)
    {
        struct ::stat st;
        return ::stat(rbtree_file.c_str(),&st)==0;
    }
    
    int ReallocRBTreeFile(void)
    {
        const bool creation = ! rbtree_body_;
        if ( changedNodes.size() > 0 ) {
            // we have to make sure that no pending nodes are on the list
            completeChanges();
        }

        const uint32_t newChunkSize = index2addr(growth + header_.cells_count);
        std::auto_ptr<char> newMem(new char[newChunkSize]);
        if(!newMem.get())
        {
            if (logger) smsc_log_error(logger, "Error reallocating memory for RBTree, reason: %s", strerror(errno));
            return BTREE_FILE_MAP_FAILED;
        }

        if (logger) smsc_log_info(logger, "RBTree index realloc from %lld to %lld cells",
                                  int64_t(header_.cells_count),
                                  int64_t(header_.cells_count+growth) );
        // smsc_log_debug(logger, "RBTree address range is [%p..%p)", newMem, newMem + newRbtFileLen );

        if (rbtree_body_)
        {
            memcpy( newMem.get(), rbtree_body_, index2addr(header_.cells_count) );
            delete rbtree_body_;
        }
        memset( newMem.get() + index2addr(header_.cells_count), 0, index2addr(growth) );
        rbtree_body_ = newMem.release();

        int32_t oldcellcount = header_.cells_count;
        int32_t freecell = header_.cells_count;
        header_.cells_count += growth;
        header_.cells_free += growth;
        if (creation) {
            header_.version = version_current;
            header_.cells_used = 1;
            --header_.cells_free;
            freecell = 1;
            header_.root_cell = 0; //-1;
            header_.nil_cell = 0;
            header_.growth = growth;
            persistentCellSize( header_ );
            // memcpy(header_.preamble, "RBTREE_FILE_STORAGE!", sizeof("RBTREE_FILE_STORAGE!"));            
        }
        // bool newfile = (!rbtree_addr);
        
        // rbtree_addr = newMem;
        // memset(rbtree_addr + rbtFileLen, 0x00, size_t(growth * sizeof(RBTreeNode)));
        
        RBTreeNode* cell = addr2node(index2addr(freecell));
        for ( unsigned long i = freecell + 1;
              i < header_.cells_count;
              ++i ) {
            // if ( growth < 100 )
            // smsc_log_debug( logger, "RBTree cell #%d has address [%x..%x)", i-1, (caddr_t)cell, (caddr_t)cell + sizeof(RBTreeNode) );
            cell->parent = (RBTreeNode*)index2addr(i);
            ++cell;
        }
        header_.first_free_cell = index2addr(freecell);

        serializeRbtHeader(rbtFileHeaderDump_, header_);
        rbtree_f.Seek( rbtFileHeaderDump_.size() + header_.persistentCellSize * oldcellcount, SEEK_SET);
        std::vector< unsigned char > buf;
        Serializer sr(buf);
        sr.setVersion( header_.version );
        for ( int32_t i = oldcellcount; i < header_.cells_count; ++i ) {
            sr.reset();
            serializeCell( sr, addr2node(index2addr(i)) );
            rbtree_f.Write(sr.data(), sr.size());
        }

        // should be in transactional manner
        // rbtFileLen = newRbtFileLen;
        if ( creation ) {
            rbtree_f.Seek(0, SEEK_SET);
            rbtree_f.Write( rbtFileHeaderDump_.data(), rbtFileHeaderDump_.size());
            rbtree_f.Flush();
        } else {
            rbtree_f.Flush();
            startChanges( getRootNode(), RBTreeChangesObserver<Key,Value>::OPER_CHANGE );
            completeChanges();
        }
        if (logger) smsc_log_debug( logger, "ReallocRBTree: cells_used %ld, cells_free %ld, cells_count %ld, first_free_cell %ld, root_cell %ld, nil_cell %ld, perscellsize %d",
                                    long(header_.cells_used), long(header_.cells_free),
                                    long(header_.cells_count), long(header_.first_free_cell),
                                    long(header_.root_cell), long(header_.nil_cell),
                                    int(header_.persistentCellSize));
        // check nodes
        freenodes();
        return SUCCESS;
    }


    int CreateRBTreeFile()
    {
        try
        {
            rbtree_f.RWCreate(rbtree_file.c_str());
            rbtree_f.SetUnbuffered();
        }
        catch(FileException ex)
        {
            if (logger) smsc_log_error(logger, "FSStorage: error idx_file: %s, reason: %s\n", rbtree_file.c_str(), ex.what());
            return CANNOT_CREATE_RBTREE_FILE;
        }

        rbtree_body_ = NULL;

        try
        {
            trans_f.RWCreate(trans_file.c_str());
            trans_f.SetUnbuffered();
        }
        catch(FileException ex)
        {
            if (logger) smsc_log_error(logger, "FSStorage: error idx_file - %s\n", ex.what());
            rbtree_f.Close();
            return CANNOT_CREATE_TRANS_FILE;
        }
        
        trans_f.WriteNetInt32(STAT_OK);
        int ret = ReallocRBTreeFile();
        if ( ret != SUCCESS ) return ret;
        return SUCCESS;
    }
    
    int OpenRBTreeFile(void)
    {
        //printf("OpenRBTreeFile\n");
        try
        {
            rbtree_f.RWOpen(rbtree_file.c_str());
            rbtree_f.SetUnbuffered();
        }
        catch(FileException ex)
        {
            return CANNOT_OPEN_RBTREE_FILE;
        }
        try
        {
            trans_f.RWOpen(trans_file.c_str());
            trans_f.SetUnbuffered();
        }
        catch(FileException ex)
        {
            rbtree_f.Close();
            return CANNOT_OPEN_TRANS_FILE;
        }
		
        int32_t status;
        try
        {
            trans_f.Seek(0, SEEK_SET);
            status = trans_f.ReadNetInt32();
        }
        catch(FileException ex)
        {
            //printf("CANNOT_READ_TRANS_FILE\n");
            rbtree_f.Close();
            trans_f.Close();
            return CANNOT_READ_TRANS_FILE;
        }
		
        int ret = SUCCESS;
        if (status == STAT_WRITE_TRX)
        {
            //printf("status == STAT_WRITE_TRX\n");
            if (logger) smsc_log_debug(logger, "OpenRBTree: transf status is trx" );
            trans_f.Seek(0, SEEK_SET);
            status = STAT_OK;
            trans_f.WriteNetInt32(status);
            trans_f.Flush();
            ret = SUCCESS_PREV_OPER_FAILED;
        } else if (status == STAT_WRITE_RBT) {
            if (logger) smsc_log_debug(logger, "OpenRBTree: transf status is rbt" );
            //printf("status == STAT_WRITE_RBT\n");
            repairRBTreeFile();
        }
		
        {
            rbtree_f.Seek(0, SEEK_END);
            const off_t len = rbtree_f.Pos();
            rbtree_f.Seek(0, SEEK_SET);
            off_t buflen = len;
            if ( buflen > sizeof(RbtFileHeader)*2+1024 ) {
                // taking sufficient chunk from the head of the file
                buflen = sizeof(RbtFileHeader)*2+1024;
            }
            std::auto_ptr<unsigned char> p(new unsigned char[buflen]);
            rbtree_f.Read(p.get(),buflen);
            Deserializer ds(p.get(),buflen);
            deserializeRbtHeader(ds,header_);
            rbtFileHeaderBuf_.clear();
            serializeRbtHeader(rbtFileHeaderDump_,header_);
            assert( ds.rpos() == rbtFileHeaderDump_.size() );
            uint64_t expectedLen = rbtFileHeaderDump_.size() + header_.cells_count*header_.persistentCellSize;
            if ( len != expectedLen ) {
                if (logger) smsc_log_warn(logger, "OpenRBTree: file size differ from what expected: headersize=%d cells_count=%lld cellsize=%d expectedlen=%lld len=%lld",
                                          int(rbtFileHeaderDump_.size()),
                                          int64_t(header_.cells_count),
                                          int(header_.persistentCellSize),
                                          expectedLen,
                                          int64_t(len) );
                // FIXME: we should return some failure status here
                // throw std::runtime_error("rbtree len differs");
                return CANNOT_OPEN_RBTREE_FILE;
            }
        }

        rbtree_body_ = new char[index2addr(header_.cells_count)];
        if ( rbtree_body_ == 0 ) return BTREE_FILE_MAP_FAILED;

        // reading cells
        rbtree_f.Seek( rbtFileHeaderDump_.size() );
        unsigned char buf[1024];
        const bool neednew = header_.persistentCellSize > 1024;
        std::auto_ptr<unsigned char> ppbuf;
        unsigned char* pbuf = buf;
        if (neednew) {
            ppbuf.reset( new unsigned char[header_.persistentCellSize] );
            pbuf = ppbuf.get();
        }
        Deserializer ds(pbuf,header_.persistentCellSize);
        ds.setVersion( header_.version );
        for ( int32_t i = 0; i < header_.cells_count; ++i ) {
            // FIXME: read cell
            rbtree_f.Read(pbuf,header_.persistentCellSize);
            ds.setrpos(0);
            deserializeCell( ds, addr2node(index2addr(i)) );
        }
        return ret;
    }
    

    int startTransaction(void)
    {
        // 1. making sure all nodes are unique
        transactionNodes_.clear();
        transactionNodes_.reserve(changedNodes.size());
        std::copy( changedNodes.begin(), changedNodes.end(), std::back_inserter(transactionNodes_) );
        std::sort( transactionNodes_.begin(), transactionNodes_.end() );
        transactionNodes_.erase( std::unique(transactionNodes_.begin(), transactionNodes_.end()),
                                 transactionNodes_.end() );

        if (logger) smsc_log_debug( logger, "Start transaction: nodes changed=%d", transactionNodes_.size() );
        //printf("header_.root_cell = %d (%d)\n", header_.root_cell, sizeof(header_.root_cell));

        FileFlushGuard fg( trans_f );

        TransFileHeader hdr;
        // hdr.version = TRX_VER_1;
        hdr.operation = currentOperation;
        hdr.nodes_count = transactionNodes_.size();
        std::vector<unsigned char> buf;
        Serializer sr(buf);
        sr.setVersion(header_.version);
        serializeTransHeader(sr,hdr);
        
        trans_f.Seek(0, SEEK_SET);
        trans_f.WriteNetInt32(STAT_WRITE_TRX);
        trans_f.Write(sr.data(),sr.size());
        
        serializeRbtHeader(rbtFileHeaderDump_,header_);
        trans_f.Write( rbtFileHeaderDump_.data(), rbtFileHeaderDump_.size());

        transactionBuf_.clear();
        Serializer ser(transactionBuf_);
        ser.setVersion(header_.version);
        transactionBuf_.reserve( transactionNodes_.size()*header_.persistentCellSize );
        for ( typename std::vector< RBTreeNode* >::const_iterator i = transactionNodes_.begin();
              i != transactionNodes_.end();
              ++i ) {
            //printf("0x%p\n", *It);
            const int32_t idx = addr2index(node2addr(*i));
            // long nodeAddr = (long)*It - (long)rbtree_addr;
            //printf("nodeAddr = 0x%X", nodeAddr);
            trans_f.WriteNetInt32( idx );
            const size_t pos = ser.size();
            serializeCell( ser, *i ); // node serialization
            trans_f.Write( &(ser.data()[pos]), header_.persistentCellSize );
        }
	return 0;
    }


    int writeChanges(void)
    {
        if (logger) smsc_log_debug( logger, "Write Changes: nodes changed=%d", transactionNodes_.size());
        trans_f.Seek(0, SEEK_SET);
        trans_f.WriteNetInt32(STAT_WRITE_RBT);
        trans_f.Flush();

        FileFlushGuard fg(rbtree_f);
        rbtree_f.Seek(0, SEEK_SET);
        rbtree_f.Write(rbtFileHeaderDump_.data(), rbtFileHeaderDump_.size());

        uint32_t pos = 0;
        for ( typename std::vector< RBTreeNode* >::const_iterator i = transactionNodes_.begin();
              i != transactionNodes_.end();
              ++i ) {
            rbtree_f.Seek( addr2index(node2addr(*i)) * header_.persistentCellSize + rbtFileHeaderDump_.size() );
            rbtree_f.Write( &transactionBuf_.front() + pos, header_.persistentCellSize );
            pos += header_.persistentCellSize;
            // rbtree_f.Seek((long)*It - (long)rbtree_addr , SEEK_SET);
            // rbtree_f.Write((char*)*It, sizeof(RBTreeNode));
        }
        //smsc_log_debug(logger, "Write Changes: finish");        
        return 0;
    }

    int endTransaction()
    {
        if (logger) smsc_log_debug(logger, "endTransaction");
        trans_f.Seek(0, SEEK_SET);
        trans_f.WriteNetInt32(STAT_OK);
        trans_f.Flush();
        return 0;
    }

    int repairRBTreeFile(void)
    {
        trans_f.Seek(0, SEEK_END);
        off_t len = trans_f.Pos();
        std::auto_ptr<unsigned char> p(new unsigned char[len-4]);
        trans_f.Seek(0, SEEK_SET);
        int status = trans_f.ReadNetInt32();
        trans_f.Read(p.get(),len-4);
        Deserializer ds(p.get(),len);

        TransFileHeader	transHdr;
        deserializeTransHeader(ds,transHdr);
        RbtFileHeader	rbtHdr;
        int32_t rbtPos = ds.rpos();
        deserializeRbtHeader(ds,rbtHdr);
        int32_t rbtLen = ds.rpos() - rbtPos;
        // trans_f.Read((char*)&transHdr, sizeof(TransFileHeader));
        // trans_f.Read((char*)&rbtHdr, sizeof(RbtFileHeader));
        rbtree_f.Seek(0, SEEK_SET);
        ds.setrpos(rbtPos);
        rbtree_f.Write( ds.curposc(), rbtLen );
        ds.setrpos(rbtPos+rbtLen);

        if (logger) smsc_log_debug(logger, "RepairRBTree: cells_used %d, cells_free %d, cells_count %d, first_free_cell %d, root_cell %d, nil_cell %d",
                                   rbtHdr.cells_used, rbtHdr.cells_free, rbtHdr.cells_count, rbtHdr.first_free_cell, rbtHdr.root_cell, rbtHdr.nil_cell);
        if (logger) smsc_log_info(logger, "repairRBTreeFile transHdr.nodes_count = %d, transHdr.status=%d", transHdr.nodes_count, status);
		
        for ( int32_t i = 0; i < transHdr.nodes_count; i++ )
        {
            uint32_t idx; // index of a cell
            ds >> idx;
            rbtree_f.Seek( rbtLen + idx * rbtHdr.persistentCellSize, SEEK_SET );
            rbtree_f.Write( ds.curposc(), rbtHdr.persistentCellSize );
            ds.setrpos( ds.rpos() + rbtHdr.persistentCellSize );
        }
        rbtree_f.Flush();
        return 0;
    }

    void serializeCell( Serializer& s, const RBTreeNode* node ) const {
        // for version 1
        if ( s.version() != 1 && s.version() != 2 ) {
            if (logger) smsc_log_warn( logger, "version %d is not implemented in rbtree, using version #1", s.version() );
            throw smsc::util::Exception( "version %d is not implemented in rbtree", s.version() );
        }
        int32_t align = 0;
        s <<
            uint64_t(addr2index(int64_t(node->parent))*header_.persistentCellSize) <<
            uint64_t(addr2index(int64_t(node->left))*header_.persistentCellSize) <<
            uint64_t(addr2index(int64_t(node->right))*header_.persistentCellSize) <<
            uint32_t(node->color) << align;
        KS ks; ks.serialize(s,node->key);
        VS vs; vs.serialize(s,node->value);
    }
    void deserializeCell( Deserializer& d, RBTreeNode* node ) const {
        if ( d.version() != 1 && d.version() != 2 ) {
            if (logger) smsc_log_warn( logger, "version %d is not implemented in rbtree", d.version() );
            throw smsc::util::Exception( "version %d is not implemented in rbtree", d.version() );
        }
        const char* fail = 0;
        uint32_t shift = 0;
        uint64_t i;
        do {
            d >> i;
            shift = uint32_t(i % header_.persistentCellSize);
            if ( shift ) { fail = "parent"; break; }
            node->parent = (RBTreeNode*)index2addr(i/header_.persistentCellSize);
            d >> i;
            shift = uint32_t(i % header_.persistentCellSize);
            if ( shift ) { fail = "left"; break; }
            node->left = (RBTreeNode*)index2addr(i/header_.persistentCellSize);
            d >> i;
            shift = uint32_t(i % header_.persistentCellSize);
            if ( shift ) { fail = "right"; break; }
            node->right = (RBTreeNode*)index2addr(i/header_.persistentCellSize);
        } while ( false );
        if ( fail ) {
            if (logger) smsc_log_warn( logger, "rbtree: reading %d node: %s field (%lld) is shifted via %d bytes",
                                       addr2index(node2addr(node)), fail, i, shift );
            throw smsc::util::Exception( "rbtree: reading %d node: %s field (%lld) is shifted via %d bytes",
                                         addr2index(node2addr(node)), fail, i, shift );
        }
        uint32_t j;
        d >> j;
        node->color = j;
        d >> j; // alignment
        KS ks; ks.deserialize(d,node->key);
        VS vs; vs.deserialize(d,node->value);
    }

    void serializeRbtHeader( Serializer& s, const RbtFileHeader& hdr ) {
        const bool first = ( s.size() == 0 );
        if ( first ) s.writeAsIs( 20, "RBTREE_FILE_STORAGE!" );
        else s.setwpos(20);
        int32_t alignment = 0;
        s << hdr.version << hdr.cells_count << hdr.cells_used << hdr.cells_free <<
            alignment <<
            int64_t(addr2index(hdr.root_cell) * hdr.persistentCellSize) <<
            int64_t(addr2index(hdr.first_free_cell) * hdr.persistentCellSize) <<
            int64_t(addr2index(hdr.nil_cell) * hdr.persistentCellSize) <<
            int64_t(hdr.growth);
        if ( first ) 
            s.writeAsIs( 160,
                         "This is a dummy message which serves as a placeholder "
                         "for future extension of RBTreeHSAllocator and should be discarded "
                         "at deserialization stage. " 
                         "ABCDEFGHIKLMNOPQRSTUVWXYZ"
                         "abcdefghiklmnopqrstuvwxyz" );
    }
    void deserializeRbtHeader( Deserializer& d, RbtFileHeader& hdr ) {
        const char* p = d.readAsIs(20);
        if ( strncmp(p, "RBTREE_FILE_STORAGE!", 20) ) {
            // strings differ
            if (logger) smsc_log_error( logger, "Wrong rbtree index prefix" );
            throw DeserializerException::stringMismatch();
        }
        d >> hdr.version >> hdr.cells_count >> hdr.cells_used >> hdr.cells_free;
        int32_t align;
        d >> align; // alignment
        int64_t i;
        d >> i; hdr.root_cell = i;
        d >> i; hdr.first_free_cell = i;
        d >> i; hdr.nil_cell = i;
        d >> i; hdr.growth = i;
        persistentCellSize( hdr );
        // post processing
        const char* fail = 0;
        int32_t shift;
        do {
            shift = int32_t( hdr.root_cell % hdr.persistentCellSize );
            if ( shift ) { fail = "root_cell"; break; }
            hdr.root_cell = index2addr(hdr.root_cell / hdr.persistentCellSize);
            shift = int32_t( hdr.first_free_cell % hdr.persistentCellSize );
            if ( shift ) { fail = "first_free_cell"; break; }
            hdr.first_free_cell = index2addr(hdr.first_free_cell / hdr.persistentCellSize);
            shift = int32_t( hdr.nil_cell % hdr.persistentCellSize );
            if ( shift ) { fail = "nil_cell"; break; }
            hdr.nil_cell = index2addr(hdr.nil_cell / hdr.persistentCellSize);
        } while ( false );
        if ( fail ) {
            if ( logger ) smsc_log_error( logger, "rbtree header field %s is shifted %d bytes, values are: root_cell=%lld, first_free_cell=%lld, nil_cell=%lld, cellsize=%d",
                                          fail, shift, hdr.root_cell, hdr.first_free_cell, hdr.nil_cell, int32_t(hdr.persistentCellSize) );
            throw smsc::util::Exception( "rbtree header field %s is shifted %d bytes, values are: root_cell=%lld, first_free_cell=%lld, nil_cell=%lld, cellsize=%d",
                                         fail, shift, hdr.root_cell, hdr.first_free_cell, hdr.nil_cell, int32_t(hdr.persistentCellSize) );
        }
        d.readAsIs(160); // skip 160 bytes
    }

    void serializeTransHeader( Serializer& s, const TransFileHeader& hdr ) {
        s << s.version() << hdr.operation << hdr.nodes_count;
    }
    void deserializeTransHeader( Deserializer& d, TransFileHeader& hdr ) {
        int32_t v;
        d >> v; // reading version
        d >> hdr.operation >> hdr.nodes_count;
    }

    // derived data: 
    // a size of one node on disk for this version
    void persistentCellSize( RbtFileHeader& header ) const
    {
        assert( header.version );
        RBTreeNode n;
        memset( (void*)&n, 0, sizeof(RBTreeNode) );
        std::vector< unsigned char > buf;
        buf.reserve(sizeof(RBTreeNode));
        Serializer s(buf);
        s.setVersion( header.version );
        serializeCell( s, &n );
        header.persistentCellSize = s.size();
    }

private:
    string			rbtree_file;
    string			trans_file;
    File			rbtree_f;
    File			trans_f;

    caddr_t			rbtree_body_;    // a pointer to a memory chunk
    off_t			growth;
    bool			running;
    RbtFileHeader		header_;          // transient header

    list<RBTreeNode*>	        changedNodes;
    int			        currentOperation;
    smsc::logger::Logger*       logger;

    // a cached serialized RbtFileHeader
    std::vector< unsigned char > rbtFileHeaderBuf_;
    Serializer                   rbtFileHeaderDump_;
    // a buffer where serialized cells are kept during transaction
    std::vector< RBTreeNode* >   transactionNodes_;
    std::vector< unsigned char > transactionBuf_;
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_RBTREEHSALLOCATOR_H */
