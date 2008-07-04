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
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "logger/Logger.h"

#include "core/buffers/File.hpp"

namespace scag {
namespace util {
namespace storage {

using namespace std;

template<class Key=long, class Value=long>
class RBTreeHSAllocator: public RBTreeAllocator<Key, Value>, public RBTreeChangesObserver<Key, Value>
{
protected:
  typedef typename RBTreeAllocator<Key,Value>::RBTreeNode RBTreeNode;
private:

	struct rbtFileHeader
	{
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
		rbtree_addr(0), rbtFileLen(0), growth(100000000), running(false)
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
		if(running)
		{
//			if(rbtree_addr) munmap(rbtree_addr, rbtFileLen);
			if(rbtree_addr) delete rbtree_addr;
			running = false;
		}
	}
	virtual RBTreeNode* allocateNode(void)
	{
		if(!running) return 0;
		RBTreeNode* newNode;
//        smsc_log_debug(logger, "allocatingNode rbtree_body = %p, header->first_free_cell = %d", rbtree_body, header->first_free_cell);
        if(!header->cells_free && ReallocRBTreeFile() != SUCCESS)
            abort();
		newNode = (RBTreeNode*)((long)rbtree_body + header->first_free_cell);
        smsc_log_debug(logger, "allocateNode rbtree_body = %p, header->first_free_cell = %d", rbtree_body, header->first_free_cell);
		header->first_free_cell = ((free_cell_list*)newNode)->next_free_cell;
		newNode->parent = newNode->left = newNode->right = 0;
		header->cells_used++;
		header->cells_free--;
		return newNode;
	}
	virtual void releaseNode(RBTreeNode* node)
	{
		if(!running) return;
		((free_cell_list*)node)->next_free_cell = header->first_free_cell;
		header->first_free_cell =(long)node - (long)rbtree_body;
		header->cells_used--;
		header->cells_free++;
	}
	virtual RBTreeNode* getRootNode(void)
	{
		if(!running) return 0;
		if(-1 == header->root_cell)	return 0;
		return (RBTreeNode*)(rbtree_body + (header->root_cell));
	}
	virtual void setRootNode(RBTreeNode* node)
	{
		if(!running) return;
		header->root_cell = (long)node - (long)rbtree_body;
		
	    smsc_log_debug(logger, "SetRoot (long)node = (%d)%p, header->root_cell=%d", (long)node - (long)rbtree_body, node, header->root_cell);
		//printf("SetRoot (long)node = %X", (long)node);
		//printf("(long)rbtree_body = %X", (long)rbtree_body);
		//printf("((long)node - (long)rbtree_body) = %d", ((long)node - (long)rbtree_body));
		//printf("header->root_cell = %d\n", header->root_cell);
	}
	virtual RBTreeNode* getNilNode(void)
	{
		if(!running) return 0;
		return (RBTreeNode*)(rbtree_body + header->nil_cell);
	}
	virtual long getSize(void) const
	{
		if(!running) return 0;
		return header->cells_used;
	}
	virtual long getOffset(void) const
	{
		if(!running) return 0;
		return (long)rbtree_body;
	}
	virtual void startChanges(RBTreeNode* node, int operation)
	{
            if ( changedNodes.size() > 0 ) completeChanges();
	    smsc_log_debug(logger, "startChanges. node = (%d)%p, operation = %d", (long)node - (long)rbtree_body, node, operation);
	    currentOperation = operation;
	    changedNodes.push_back(node);
	}
	virtual void nodeChanged(RBTreeNode* node)
	{
//	    smsc_log_debug(logger, "Node changed=%d(%p)", (long)node - (long)rbtree_body, node);
	    changedNodes.push_back(node);
	}
	virtual void completeChanges(void)
	{
//	    smsc_log_debug(logger, "completeChanges=%d", changedNodes.size());
	    changedNodes.sort();
	    changedNodes.unique();
            //printf("%d\n", changedNodes.size());
	    startTransaction();
	    writeChanges();
	    endTransaction();
	    changedNodes.erase(changedNodes.begin(), changedNodes.end());
	}

private:
	string			rbtree_file;
	string			trans_file;
	File			rbtree_f;
	File			trans_f;
	caddr_t			rbtree_addr;
	caddr_t			rbtree_body;
	off_t			growth;
	off_t			rbtFileLen;
	bool			running;
	rbtFileHeader*		header;
	
	list<RBTreeNode*>	changedNodes;
	int			currentOperation;
    
    smsc::logger::Logger* logger;
	
	bool isFileExists(void)
	{
		struct ::stat st;
		return ::stat(rbtree_file.c_str(),&st)==0;
	}
    
    int ReallocRBTreeFile(void)
    {
        uint32_t _growth = growth;
        if(!rbtree_addr)
        {
            rbtFileLen = sizeof(rbtFileHeader);
            _growth--;
        } else {
            // we have to make sure that no pending nodes are on the list
            completeChanges();
        }
        
        uint32_t newRbtFileLen = rbtFileLen + growth * sizeof(RBTreeNode);
        caddr_t newMem = new char[newRbtFileLen];
        if(!newMem)
        {
            smsc_log_error(logger, "Error reallocating memory for RBTree, reason: %s", strerror(errno));
            return BTREE_FILE_MAP_FAILED;
        }
        else {
            smsc_log_info(logger, "RBTree index realloc from %lld to %lld bytes",
                          static_cast<long long>(rbtFileLen),
                          static_cast<long long>(newRbtFileLen) );
            // smsc_log_debug(logger, "RBTree address range is [%p..%p)", newMem, newMem + newRbtFileLen );
        }

        if(rbtree_addr)
        {
            memcpy(newMem, rbtree_addr, rbtFileLen);
            delete rbtree_addr;
        } else {
            memset(newMem, 0, rbtFileLen);
        }
            
        header = (rbtFileHeader*)newMem;
        rbtree_body = newMem + sizeof(rbtFileHeader);        
        
        if(!rbtree_addr)
        {
            header->root_cell = 0;//-1;
            header->nil_cell = 0;
            header->growth = growth;
            header->cells_used = 1;
            header->cells_count = 1;
            header->version = version_64_1;
            memcpy(header->preamble, "RBTREE_FILE_STORAGE!", sizeof("RBTREE_FILE_STORAGE!"));            
        }
        
        rbtree_addr = newMem;
        memset(rbtree_addr + rbtFileLen, 0x00, growth * sizeof(RBTreeNode));
        
        free_cell_list* cell = (free_cell_list*)(rbtree_addr + sizeof(rbtFileHeader) + header->cells_count * sizeof(RBTreeNode));
        for( long i = header->cells_count + 1; i < header->cells_count + _growth; i++ )
        {
            // if ( growth < 100 )
            // smsc_log_debug( logger, "RBTree cell #%d has address [%x..%x)", i-1, (caddr_t)cell, (caddr_t)cell + sizeof(RBTreeNode) );
            cell->next_free_cell = i * sizeof(RBTreeNode);
            cell = (free_cell_list*)((caddr_t)cell + sizeof(RBTreeNode));
        }

        header->first_free_cell = header->cells_count * sizeof(RBTreeNode);
        header->cells_count += _growth;
        header->cells_free = _growth;
        rbtree_f.Seek(rbtFileLen, SEEK_SET);
        rbtree_f.Write(rbtree_addr + rbtFileLen, newRbtFileLen - rbtFileLen);
        rbtree_f.Flush();
        // should be in transactional manner
        // rbtree_f.Seek(0, SEEK_SET);
        // rbtree_f.Write(rbtree_addr, sizeof(rbtFileHeader));
        rbtFileLen = newRbtFileLen;
        startChanges( getRootNode(), OPER_CHANGE );
        completeChanges();
        smsc_log_debug(logger, "ReallocRBTree: cells_used %ld, cells_free %ld, cells_count %ld, first_free_cell %ld, root_cell %ld, nil_cell %ld, rbtFileLen %lld",
                       long(header->cells_used), long(header->cells_free),
                       long(header->cells_count), long(header->first_free_cell),
                       long(header->root_cell), long(header->nil_cell),
                       static_cast<long long>(rbtFileLen) );
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

        rbtree_addr = NULL;
        
        uint32_t i;
        if((i = ReallocRBTreeFile()) != SUCCESS)
            return i;

		try
		{
		    trans_f.RWCreate(trans_file.c_str());
		    trans_f.SetUnbuffered();
		}
		catch(FileException ex)
		{
		    smsc_log_error(logger, "FSStorage: error idx_file - %s\n", ex.what());
		    rbtree_f.Close();
		    return CANNOT_CREATE_TRANS_FILE;
		}
        
		int status = STAT_OK;
		trans_f.Write((char*)&status, sizeof(int));
		return SUCCESS;
	}
    
	int OpenRBTreeFile(void)
	{
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
                        smsc_log_debug(logger, "OpenRBTree: transf status is trx" );
			trans_f.Seek(0, SEEK_SET);
			status = STAT_OK;
			trans_f.Write((char*)&status, sizeof(int));
			ret = SUCCESS_PREV_OPER_FAILED;
		}
		else if(status == STAT_WRITE_RBT)
		{
                        smsc_log_debug(logger, "OpenRBTree: transf status is rbt" );
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
        rbtFileLen = len;
		smsc_log_debug(logger, "OpenRBTree: cells_used %d, cells_free %d, cells_count %d, first_free_cell %d, root_cell %d, nil_cell %d, rbtFileLen %d", header->cells_used, header->cells_free, header->cells_count, header->first_free_cell, header->root_cell, header->nil_cell, rbtFileLen);
		return ret;
	}
    
	int startTransaction(void)
	{
		transFileHeader		hdr;
		typename list<RBTreeNode*>::iterator It;
		long			nodeAddr;
            FileFlushGuard fg( trans_f );
		
		hdr.status = STAT_WRITE_TRX;
		hdr.version = TRX_VER_1;
		hdr.operation = currentOperation;
		hdr.nodes_count = changedNodes.size();
	    smsc_log_debug( logger, "Start transaction: nodes changed=%d", changedNodes.size() );
		//printf("header->root_cell = %d (%d)\n", header->root_cell, sizeof(header->root_cell));
		trans_f.Seek(0, SEEK_SET);
		trans_f.Write((char*)&hdr, sizeof(transFileHeader));
		trans_f.Write((char*)header, sizeof(rbtFileHeader));
		
		for(It = changedNodes.begin(); It != changedNodes.end(); It++)
		{
			//printf("0x%p\n", *It);
			nodeAddr = (long)*It - (long)rbtree_addr;
			//printf("nodeAddr = 0x%X", nodeAddr);
			trans_f.Write(&nodeAddr, sizeof(long));
			trans_f.Write((char*)*It, sizeof(RBTreeNode));
		}
	
	return 0;
	}
	int writeChanges(void)
	{
	    smsc_log_debug( logger, "Write Changes: nodes changed=%d", changedNodes.size());
	    int stat = STAT_WRITE_RBT;
	    trans_f.Seek(0, SEEK_SET);
	    trans_f.Write((char*)&stat, sizeof(int));
            trans_f.Flush();

            FileFlushGuard fg(rbtree_f);
	    typename list<RBTreeNode*>::iterator It;
	    rbtree_f.Seek(0, SEEK_SET);
	    rbtree_f.Write((char*)header, sizeof(rbtFileHeader));

	    for(It = changedNodes.begin(); It != changedNodes.end(); It++)
	    {
    		rbtree_f.Seek((long)*It - (long)rbtree_addr , SEEK_SET);
	    	rbtree_f.Write((char*)*It, sizeof(RBTreeNode));
	    }
	    //smsc_log_debug(logger, "Write Changes: finish");        
	    return 0;
	}
	int endTransaction()
	{
            smsc_log_debug(logger, "endTransaction");
		int stat = STAT_OK;
		trans_f.Seek(0, SEEK_SET);
		trans_f.Write((char*)&stat, sizeof(int));
            trans_f.Flush();
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

                smsc_log_debug(logger, "RepairRBTree: cells_used %d, cells_free %d, cells_count %d, first_free_cell %d, root_cell %d, nil_cell %d, rbtFileLen %d", rbtHdr.cells_used, rbtHdr.cells_free, rbtHdr.cells_count, rbtHdr.first_free_cell, rbtHdr.root_cell, rbtHdr.nil_cell, rbtFileLen);
		smsc_log_info(logger, "repairRBTreeFile transHdr.nodes_count = %d, transHdr.status=%d", transHdr.nodes_count, transHdr.status);
		
		for(int i = 0; i < transHdr.nodes_count; i++)
		{
			trans_f.Read((char*)&nodeAddr, sizeof(long));
			trans_f.Read((char*)&curNode, sizeof(RBTreeNode));
			rbtree_f.Seek(nodeAddr, SEEK_SET);
			rbtree_f.Write((char*)&curNode, sizeof(RBTreeNode));
		}
            rbtree_f.Flush();
		return 0;
	}
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_RBTREEHSALLOCATOR_H */
