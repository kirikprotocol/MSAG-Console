//------------------------------------
//  RBTreeHSAllocator.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ RBTreeHSAllocator.
//

#include <string>
#include <cerrno>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "sys/mman.h"
#include "logger/Logger.h"
#include "RBTreeAllocator.h"

#include "core/buffers/File.hpp"

#ifndef ___RBTREE_HS_ALLOCATOR_H
#define ___RBTREE_HS_ALLOCATOR_H

using namespace std;

template<class Key=long, class Value=long>
class RBTreeHSAllocator: public RBTreeAllocator<Key, Value>, public RBTreeChangesObserver<Key, Value>
{
protected:
  typedef typename RBTreeAllocator<Key,Value>::RBTreeNode RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type             nodeptr_type;

private:

	struct rbtFileHeader
	{
            rbtFileHeader() :
            cells_count(0), cells_used(0), cells_free(0), root_cell(0),
            first_free_cell(0), nil_cell(0), growth(0) {}

		char preamble[20];
		int version;

		int cells_count;
		int cells_used;
		int cells_free;
		long root_cell;
		long first_free_cell;
		long nil_cell;
		off_t growth;
		char reserved[168 - sizeof(off_t)];
	};
	
	static	const int STAT_OK = 0;
	static	const int STAT_WRITE_TRX = 1;
	static	const int STAT_WRITE_RBT = 2;
	static	const int TRX_VER_1 = 1;
	
	struct transFileHeader
	{
		int status;
		int version;
		int operation;
		int nodes_count;
	};

	struct free_cell_list
	{
		long next_free_cell;
	};

public:
	static const int version_32_1 = 0x01;
	static const int version_64_1 = 0x80000001;

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

	RBTreeHSAllocator():
        growth(1000000), running(false), currentOperation(0)
	{
        logger = smsc::logger::Logger::getInstance("RBTAlloc");
	}

	void Create(void){}
	void Open(void){}
	void Close(void){}

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
			int ret;
			if(0 > (ret = CreateRBTreeFile()))
				return ret;
		}
		else
		{
			int ret;
			if(0 > (ret = OpenRBTreeFile()))
				return ret;
		}
		running = true;
		return SUCCESS;
	}

	virtual ~RBTreeHSAllocator()
	{
            if (running) running = false;
            for ( std::vector< caddr_t >::iterator i = chunks_.begin();
                  i != chunks_.end(); ++i ) {
                delete *i;
            }
	}
	virtual nodeptr_type allocateNode(void)
	{
		if(!running) return 0;
//        smsc_log_debug(logger, "allocatingNode rbtree_body = %p, header_.first_free_cell = %d", rbtree_body, header_.first_free_cell);
        if(!header_.cells_free && ReallocRBTreeFile() != SUCCESS)
            abort();
            nodeptr_type newNode = (nodeptr_type)header_.first_free_cell;
            RBTreeNode* node = addr2node(newNode);
        smsc_log_debug(logger, "allocateNode node=%ld/%p", (long)newNode, node);
		header_.first_free_cell = ((free_cell_list*)node)->next_free_cell;
		node->parent = node->left = node->right = (nodeptr_type)header_.nil_cell;
		header_.cells_used++;
		header_.cells_free--;
		return newNode;
	}
	virtual void releaseNode(nodeptr_type node)
	{
		if(!running) return;
                RBTreeNode* theNode = addr2node(node);
		((free_cell_list*)theNode)->next_free_cell = header_.first_free_cell;
                header_.first_free_cell = (long)node;
		header_.cells_used--;
		header_.cells_free++;
	}
	virtual nodeptr_type getRootNode(void)
	{
                if(!running) return (nodeptr_type)header_.nil_cell;
		if(-1 == header_.root_cell) return (nodeptr_type)header_.nil_cell;
		return (nodeptr_type)header_.root_cell;
	}
	virtual void setRootNode(nodeptr_type node)
	{
		if(!running) return;
		header_.root_cell = (long)node;
		
	    smsc_log_debug(logger, "SetRoot node = %ld", (long)node);
		//printf("SetRoot (long)node = %X", (long)node);
		//printf("(long)rbtree_body = %X", (long)rbtree_body);
		//printf("((long)node - (long)rbtree_body) = %d", ((long)node - (long)rbtree_body));
		//printf("header_.root_cell = %d\n", header_.root_cell);
	}
	virtual nodeptr_type getNilNode(void)
	{
            return (nodeptr_type)header_.nil_cell;
	}
	virtual long getSize(void) const
	{
		if(!running) return 0;
		return header_.cells_used;
	}
    
    virtual RBTreeNode* realAddr(nodeptr_type n) const
    {
        return addr2node(n);
    }

	virtual void startChanges(nodeptr_type node, int operation)
	{
            if ( changedNodes.size() > 0 ) completeChanges();
//	    smsc_log_debug(logger, "startChanges. node = (%d)%p, operation = %d", (long)node - (long)rbtree_body, node, operation);        
        //TODO: if changedNodes not empty call compliteChanges

            currentOperation = operation;
	    changedNodes.push_back(node);
	}
	virtual void nodeChanged(nodeptr_type node)
	{
//	    smsc_log_debug(logger, "Node changed=%d(%p)", (long)node - (long)rbtree_body, node);
	    changedNodes.push_back(node);
	}
	virtual void completeChanges(void)
	{
            if ( changedNodes.empty() ) return;
//	    smsc_log_debug(logger, "completeChanges=%d", changedNodes.size());
            //printf("%d\n", changedNodes.size());
	    startTransaction();
	    writeChanges();
	    endTransaction();
            changedNodes.erase(changedNodes.begin(), changedNodes.end());
	}

private:
    inline RBTreeNode* addr2node( nodeptr_type offset ) const
    {
        register long idx = ((long)offset)/cellsize();
        register unsigned chunkidx = idx/growth;
        assert( chunkidx < chunks_.size() );
        return reinterpret_cast< RBTreeNode* >(int64_t(chunks_[chunkidx]) + cellsize()*int64_t(idx%growth) );
    }

    inline nodeptr_type idx2addr(long idx) const { return (nodeptr_type)(idx*cellsize()); }

    inline int32_t cellsize() const { return sizeof(RBTreeNode); }
    
	bool isFileExists(void)
	{
		struct ::stat st;
		return ::stat(rbtree_file.c_str(),&st)==0;
	}
    
    int ReallocRBTreeFile(void)
    {
        if ( header_.cells_free >= growth ) return SUCCESS;

        const bool creation = ( chunks_.size() == 0 );
        if ( changedNodes.size() > 0 ) {
            // we have to make sure that no pending nodes are on the list
            completeChanges();
        }

        smsc_log_info(logger,"RBtree realloc started");

        int32_t realgrowth = growth - header_.cells_count % growth;
        if ( realgrowth == growth ) {
            // new chunk is needed
            const int64_t newChunkSize = int64_t(cellsize()*realgrowth);
            std::auto_ptr<char> newMem(new char[newChunkSize]);
            if(!newMem.get())
            {
                smsc_log_error(logger, "Error reallocating memory for RBTree, reason: %s", strerror(errno));
                return BTREE_FILE_MAP_FAILED;
            }

            // smsc_log_debug(logger, "RBTree address range is [%p..%p)", newMem, newMem + newRbtFileLen );
            chunks_.push_back( newMem.release() );
        } else {
            // we don't need a new chunk, the last one is quite enough
        }
        memset( chunks_.back()+(growth-realgrowth)*cellsize(), 0, realgrowth*cellsize() );
        smsc_log_info(logger, "RBTree index realloc from %lld to %lld cells",
                      int64_t(header_.cells_count),
                      int64_t(header_.cells_count+realgrowth) );

        nodeptr_type freecell = idx2addr(header_.cells_count);
        if ( header_.cells_free > 0 ) {
            // we have to find the last free cell
            nodeptr_type lastfreecell = (nodeptr_type)header_.first_free_cell;
            for ( unsigned i = header_.cells_free; i > 1; --i ) {
                lastfreecell = (nodeptr_type)((free_cell_list*)addr2node(lastfreecell))->next_free_cell;
            }
            ((free_cell_list*)addr2node(lastfreecell))->next_free_cell = (long)freecell;
            nodeChanged(lastfreecell);
        } else {
            header_.first_free_cell = (long)freecell;
        }

        // filling free cells links
        for ( long i = 0; i < realgrowth-1; ++i ) {
            RBTreeNode* cell = addr2node(freecell);
            ((free_cell_list*)cell)->next_free_cell = (long)++freecell;
        }

        long startcell = header_.cells_count;
        header_.cells_count += realgrowth;
        header_.cells_free += realgrowth;
        if (creation) {
    		header_.cells_used = 1;
                header_.first_free_cell = cellsize(); // points to a 1st cell
                --header_.cells_free;
    		header_.root_cell = 0;
	    	header_.nil_cell = 0;
                header_.growth = growth;
    		header_.version = version_64_1;
                memset(addr2node(0),0,cellsize()); // clearing a nil node
    		memcpy(header_.preamble, "RBTREE_FILE_STORAGE!", sizeof("RBTREE_FILE_STORAGE!"));
        }
        
        {
            // writing new cells to disk
            // NOTE: those cells must be all in one chunk!
            assert(startcell/growth == (startcell+realgrowth-1)/growth);
            rbtree_f.Seek( sizeof(rbtFileHeader) + cellsize()*startcell, SEEK_SET );
            rbtree_f.Write(addr2node(idx2addr(startcell)),cellsize()*realgrowth);
        }
        
        if ( creation ) {
            // writing file header to disk
            rbtree_f.Seek(0, SEEK_SET);
            rbtree_f.Write( &header_, sizeof(rbtFileHeader));
            rbtree_f.Flush();
        } else {
            rbtree_f.Flush();
            completeChanges();
        }

        smsc_log_info( logger, "ReallocRBTree: cells_used %ld, cells_free %ld, cells_count %ld, first_free_cell %ld, root_cell %ld, nil_cell %ld, perscellsize %d",
                        long(header_.cells_used), long(header_.cells_free),
                        long(header_.cells_count), long(header_.first_free_cell),
                        long(header_.root_cell), long(header_.nil_cell), cellsize() );
        return SUCCESS;
    }
    
	int CreateRBTreeFile(void)
	{
		try
		{
		    rbtree_f.RWCreate(rbtree_file.c_str());
		    rbtree_f.SetUnbuffered();
		}
		catch(FileException ex)
		{
		    smsc_log_error(logger, "FSStorage: error idx_file: %s, reason: %s\n", rbtree_file.c_str(), ex.what());
		    return CANNOT_CREATE_RBTREE_FILE;
		}

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
            /*

		//printf("OpenRBTreeFile\n");
		int ret = SUCCESS;
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
		
		int status;
		try
		{
			trans_f.Seek(0, SEEK_SET);
			trans_f.Read((char*)&status, sizeof(int));
		}
		catch(FileException ex)
		{
			//printf("CANNOT_READ_TRANS_FILE\n");
			rbtree_f.Close();
			trans_f.Close();
			return CANNOT_READ_TRANS_FILE;
		}
		
		if(status == STAT_WRITE_TRX)
		{
			//printf("status == STAT_WRITE_TRX\n");
			trans_f.Seek(0, SEEK_SET);
			status = STAT_OK;
			trans_f.Write((char*)&status, sizeof(int));
			ret = SUCCESS_PREV_OPER_FAILED;
		}
		else if(status == STAT_WRITE_RBT)
		{
			//printf("status == STAT_WRITE_RBT\n");
			repairRBTreeFile();
		}
		
		rbtree_f.Seek(0, SEEK_END);
		off_t len = rbtree_f.Pos();
		rbtree_f.Seek(0, SEEK_SET);
			
		if(!(rbtree_addr = new char[len]))
			return BTREE_FILE_MAP_FAILED;

		rbtree_f.Read(rbtree_addr, len);
		header = (rbtFileHeader*)rbtree_addr;
		rbtree_body = rbtree_addr + sizeof(rbtFileHeader);
        //TODO: calc rbtFileLen 
        rbtFileLen = len;
		smsc_log_debug(logger, "OpenRBTree: cells_used %d, cells_free %d, cells_count %d, first_free_cell %d, root_cell %d, nil_cell %d, rbtFileLen %d", header_.cells_used, header_.cells_free, header_.cells_count, header_.first_free_cell, header_.root_cell, header_.nil_cell, rbtFileLen);
		return ret;
             */

        try {
            // determining the length of the file
            rbtree_f.Seek(0, SEEK_END);
            const off_t len = rbtree_f.Pos();
            rbtree_f.Seek(0, SEEK_SET);

            const off_t headerLen = sizeof(rbtFileHeader);
            if ( len < headerLen ) {
                return BTREE_FILE_MAP_FAILED;
            }

            // reading the header
            rbtree_f.Read(&header_,headerLen);
            const off_t expectedLen = off_t(headerLen + header_.cells_count*cellsize());

            long maxcells = header_.cells_count;
            if ( len < expectedLen ) {
                maxcells = ( len - headerLen ) / cellsize();
            }

            // create the necessary number of chunks
            unsigned needchunks = (maxcells-1) / growth + 1;
            chunks_.reserve( needchunks );
            for ( unsigned i = 0; i < needchunks; ++i ) {
                std::auto_ptr<char> mem( new char[growth*cellsize()] );
                if ( ! mem.get() ) return BTREE_FILE_MAP_FAILED;
                chunks_.push_back( mem.release() );
            }

            // reading cells
            rbtree_f.Seek( headerLen );
            for ( unsigned i = 0; i < needchunks; ++i ) {
                long chunksize = std::min(growth,maxcells-i*growth) * cellsize();
                rbtree_f.Read(addr2node(idx2addr(i*growth)),chunksize);
            }
            
            if ( len < expectedLen ) {
                // FIXME: we should fix it oneday
                smsc_log_warn( logger, "OpenRBTree: file size is smaller than what expected: headersize=%d expected_cells_count=%lld actual_cells_count=%lld cellsize=%d expectedlen=%lld len=%lld, I'll try to recover...",
                               int(headerLen),
                               int64_t(header_.cells_count),
                               int64_t(maxcells),
                               int(cellsize()),
                               expectedLen,
                               int64_t(len) );
                /*
                // trying to recover the file
                RBTreeChecker< Key, Value > checker( *this,
                                                     maxcells,
                                                     logger,
                                                     10 );
                if ( ! checker.check(header_.root_cell, header_.nil_cell) ) {
                    if ( logger ) smsc_log_error( logger,
                                                  "cannot recover the tree: %s at depth=%u in node %ld path=%s",
                                                  checker.failedMsg(),
                                                  checker.failedDepth(),
                                                  long(checker.failedNode()),
                                                  checker.failedPath() );
                    return CANNOT_OPEN_RBTREE_FILE;
                }

                // fix header.
                // NOTE: we don't truncate the file here.
                header_.cells_count = header_.cells_used = checker.maxUsedCell() + 1;
                header_.cells_free = 0;
                header_.first_free_cell = 0;
                if (logger) smsc_log_debug( logger, "Hoorah! It seems that we could recover, the cells_count=cells_used=%d actual_used_cells=%d",
                                            int(header_.cells_count),
                                            int(checker.usedCells()) );
                 */
                return BTREE_FILE_MAP_FAILED;
            } else if ( len != expectedLen ) {
                return CANNOT_OPEN_RBTREE_FILE;
            }
            /*
                    smsc_log_warn( logger, "RBTree file size differ: expectedlen=%lld len=%lld",
                                   int64_t(expectedLen), int64_t(len) );

                    RBTreeChecker< Key, Value > checker( *this,
                                                         header_.cells_count,
                                                         logger,
                                                         10 );
                
                    if ( ! checker.check(header_.root_cell, header_.nil_cell) ) {
                        smsc_log_error( logger, "RBTree file is corrupted: %s at depth=%u in node %ld path=%s",
                                        checker.failedMsg(),
                                        checker.failedDepth(),
                                        long(checker.failedNode()),
                                        checker.failedPath() );
                        return CANNOT_OPEN_RBTREE_FILE;
                    }
                }

            }
             */

        } catch (std::exception& e) {
            smsc_log_error(logger, "OpenRBTree: exception %s", e.what() );
            return CANNOT_OPEN_RBTREE_FILE;
        } catch (...) {
            smsc_log_error(logger, "OpenRBTree: unknown exception");
            return CANNOT_OPEN_RBTREE_FILE;
        }

        smsc_log_info( logger, "OpenRBTree: version=%d cells_count=%d cells_used=%d cells_free=%d root_cell=%d first_free_cell=%d pers_cell_size=%d",
                       header_.version,
                       header_.cells_count,
                       header_.cells_used,
                       header_.cells_free,
                       int32_t(header_.root_cell),
                       int32_t(header_.first_free_cell),
                       int(cellsize()) );
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

        smsc_log_debug( logger, "Start transaction: nodes changed=%d", int(transactionNodes_.size()) );
            
        transFileHeader		hdr;
		
        hdr.status = STAT_WRITE_TRX;
        hdr.version = TRX_VER_1;
        hdr.operation = currentOperation;
        hdr.nodes_count = transactionNodes_.size();
	    //smsc_log_debug(logger, "Start transaction: nodes changed=%d", changedNodes.size());
		//printf("header_.root_cell = %d (%d)\n", header_.root_cell, sizeof(header_.root_cell));
        trans_f.Seek(0, SEEK_SET);
        trans_f.Write(&hdr, sizeof(transFileHeader));
        trans_f.Write(&header_, sizeof(rbtFileHeader));
		
        for ( typename std::vector< nodeptr_type >::const_iterator i = transactionNodes_.begin();
              i != transactionNodes_.end();
              ++i ) {
            long nodeAddr = (long)*i;
            trans_f.Write(&nodeAddr, sizeof(long));
            trans_f.Write(addr2node(*i), sizeof(RBTreeNode));
        }
	
	return 0;
	}
	int writeChanges(void)
	{
	    //smsc_log_debug(logger, "Write Changes: nodes changed=%d", changedNodes.size());
	    int stat = STAT_WRITE_RBT;
	    trans_f.Seek(0, SEEK_SET);
	    trans_f.Write((char*)&stat, sizeof(int));
	    rbtree_f.Seek(0, SEEK_SET);
	    rbtree_f.Write(&header_, sizeof(rbtFileHeader));

            for ( typename std::vector< nodeptr_type >::const_iterator i = transactionNodes_.begin();
                  i != transactionNodes_.end();
                  ++i ) {
                nodeptr_type addr = *i;
                rbtree_f.Seek(sizeof(rbtFileHeader)+(long)addr, SEEK_SET);
                rbtree_f.Write(addr2node(addr), sizeof(RBTreeNode));
	    }
	    //smsc_log_debug(logger, "Write Changes: finish");        
	    return 0;
	}
	int endTransaction()
	{
		//smsc_log_debug(logger, "endTransaction");
		int stat = STAT_OK;
		trans_f.Seek(0, SEEK_SET);
                trans_f.Write((char*)&stat, sizeof(int));
		return 0;
	}
	int repairRBTreeFile(void)
	{
		transFileHeader	transHdr;
		rbtFileHeader	rbtHdr;
		long		nodeAddr;
		RBTreeNode	curNode;
		
		trans_f.Seek(0, SEEK_SET);
		trans_f.Read((char*)&transHdr, sizeof(transFileHeader));
		trans_f.Read((char*)&rbtHdr, sizeof(rbtFileHeader));
		rbtree_f.Seek(0, SEEK_SET);
		rbtree_f.Write((char*)&rbtHdr, sizeof(rbtFileHeader));

		smsc_log_debug(logger, "RepairRBTree: cells_used %d, cells_free %d, cells_count %d, first_free_cell %d, root_cell %d, nil_cell %d, rbtFileLen %d", rbtHdr.cells_used, rbtHdr.cells_free, rbtHdr.cells_count, rbtHdr.first_free_cell, rbtHdr.root_cell, rbtHdr.nil_cell,
                               sizeof(rbtFileHeader)+idx2addr(rbtHdr.cells_count));
		smsc_log_info(logger, "repairRBTreeFile transHdr.nodes_count = %d, transHdr.status=%d", transHdr.nodes_count, transHdr.status);
		
		for(int i = 0; i < transHdr.nodes_count; i++)
		{
			trans_f.Read((char*)&nodeAddr, sizeof(long));
			trans_f.Read((char*)&curNode, sizeof(RBTreeNode));
			rbtree_f.Seek(sizeof(rbtFileHeader)+nodeAddr, SEEK_SET);
			rbtree_f.Write((char*)&curNode, sizeof(RBTreeNode));
		}
		return 0;
	}

private:
	string			rbtree_file;
	string			trans_file;
	File			rbtree_f;
	File			trans_f;
	off_t			growth;
	bool			running;
	rbtFileHeader		header_;
        std::vector< caddr_t >  chunks_;
	list<nodeptr_type>	changedNodes;
	int			currentOperation;
        smsc::logger::Logger* logger;
    
    std::vector< nodeptr_type > transactionNodes_;
};

#endif

//		printf("sizeof(RBTreeNode) = %d\n", sizeof(RBTreeNode));
		//printf("sizeof(n.parent) = %d\n", sizeof(n.parent));
		//printf("sizeof(n.left) = %d\n", sizeof(n.left));
		//printf("sizeof(n.right) = %d\n", sizeof(n.right));
		//printf("sizeof(n.color) = %d\n", sizeof(n.color));
		//printf("sizeof(n.key) = %d\n", sizeof(n.key));
		//printf("sizeof(n.value) = %d\n", sizeof(n.value));
		//printf("sizeof(Abnt1) = %d\n", sizeof(Abnt1));
		//printf("sizeof(n) = %d\n", sizeof(n));
//		if(-1 == (rbtree_fd = open(rbtree_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode)))
//			return CANNOT_CREATE_RBTREE_FILE;

//		printf("%d\n", rbtFileLen);

//	    if(MAP_FAILED == (rbtree_addr = mmap((caddr_t)0, rbtFileLen, PROT_READ | PROT_WRITE, MAP_SHARED, rbtree_fd, 0)))
//		{
////			perror("mmap: ");
//			return BTREE_FILE_MAP_FAILED;
//		}
