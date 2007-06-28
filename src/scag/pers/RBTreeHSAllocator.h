//------------------------------------
//  RBTreeHSAllocator.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ RBTreeHSAllocator.
//

#include "RBTreeAllocator.h"
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/mman.h"

#include "core/buffers/File.hpp"

extern int errno;

#ifndef ___RBTREE_HS_ALLOCATOR_H
#define ___RBTREE_HS_ALLOCATOR_H

using namespace std;

template<class Key=long, class Value=long>
class RBTreeHSAllocator: public RBTreeAllocator<Key, Value>, public RBTreeChangesObserver<Key, Value>
{
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
		rbtree_addr(0), rbtree_fd(-1), growth(100000000), running(false)
	{
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
			if(0 != (ret = CreateRBTreeFile()))
				return ret;
		}
		else
		{
			int ret;
			if(0 != (ret = OpenRBTreeFile()))
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
			if(rbtree_addr) free((void*)rbtree_addr);
			if(-1 != rbtree_fd) close(rbtree_fd);
			running = false;
		}
	}
	virtual RBTreeNode* allocateNode(void)
	{
		if(!running) return 0;
		RBTreeNode* newNode;
		newNode = (RBTreeNode*)((long)rbtree_body + header->first_free_cell);
//		printf("allocateNode 1111 rbtree_body = %p, header->first_free_cell = %X newNode = %p\n", rbtree_body, header->first_free_cell, newNode);
		header->first_free_cell = ((free_cell_list*)newNode)->next_free_cell - (long)rbtree_body;
		newNode->parent = newNode->left = newNode->right = (RBTreeNode*)header->nil_cell;

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
		
		//printf("SetRoot (long)node = %X", (long)node);
		//printf("(long)rbtree_body = %X", (long)rbtree_body);
		//printf("((long)node - (long)rbtree_body) = %d", ((long)node - (long)rbtree_body));
		//printf("header->root_cell = %d\n", header->root_cell);
	}
	virtual RBTreeNode* getNilNode(void)
	{
		if(!running) return 0;
		return (RBTreeNode*)(rbtree_body + (header->nil_cell));
	}
	virtual long getSize(void)
	{
		if(!running) return 0;
		return header->cells_used;
	}
	virtual long getOffset(void)
	{
		if(!running) return 0;
		return (long)rbtree_body;
	}
	virtual void startChanges(RBTreeNode* node, int operation)
	{
	    //printf("startChanges. node = 0x%p, operation = %d\n", node, operation);
	    currentOperation = operation;
	    changedNodes.erase(changedNodes.begin(), changedNodes.end());
	    changedNodes.push_back(node);
	}
	virtual void nodeChanged(RBTreeNode* node)
	{
	    //printf("nodeChanged 0x%p\n", node);
	    changedNodes.push_back(node);
	}
	virtual void completeChanges(void)
	{
	    //printf("completeChanges %d\n", changedNodes.size());
	    changedNodes.sort();
	    changedNodes.unique();
            //printf("%d\n", changedNodes.size());
	    startTransaction();
	    writeChanges();
	    endTransaction();
	}

private:
	string			rbtree_file;
	string			trans_file;
	int			rbtree_fd;
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
	
	bool isFileExists(void)
	{
		struct ::stat st;
		return ::stat(rbtree_file.c_str(),&st)==0;
	}
	int CreateRBTreeFile(void)
	{
		rbtFileLen = growth*sizeof(RBTreeNode) + sizeof(rbtFileHeader) + sizeof(RBTreeNode);

		try
		{
		    rbtree_f.RWCreate(rbtree_file.c_str());
		    rbtree_f.SetUnbuffered();
		}
		catch(FileException ex)
		{
//		    smsc_log_debug(logger, "FSStorage: error idx_file - %s\n", ex.what());
		    return CANNOT_CREATE_RBTREE_FILE;
		}

		if(!(rbtree_addr = (caddr_t)malloc(rbtFileLen)))
		{
			//printf("malloc failed\n");
			return BTREE_FILE_MAP_FAILED;
		}

		memset((void*)rbtree_addr, 0x00, rbtFileLen);
		
		//create nil cell
		caddr_t nil_cell_addr = rbtree_body = rbtree_addr + sizeof(rbtFileHeader);
		RBTreeNode*	nilNode = (RBTreeNode*)nil_cell_addr;
		nilNode->parent = 0;
		nilNode->left = 0;
		nilNode->right = 0;
		nilNode->color = BLACK;

		//create list of free cells
		free_cell_list* cell = (free_cell_list*)(nil_cell_addr + sizeof(RBTreeNode));
		for(long i = 2; i < growth; i++)
		{
			cell->next_free_cell = (long)nil_cell_addr + i*sizeof(RBTreeNode);
			cell = (free_cell_list*)(cell->next_free_cell);
		}

		// fill header by initialization info
		header = (rbtFileHeader*)rbtree_addr;
//		memcpy((void*)&(header->preamble), sPREAMBLE, sizeof(header->preamble));
		memcpy((void*)&(header->preamble), "RBTREE_FILE_STORAGE!", sizeof("RBTREE_FILE_STORAGE!"));
		header->version = version_64_1;
		header->cells_count = growth;
		header->cells_used = 1;
		header->cells_free = growth - 1;
		header->root_cell = (long)nil_cell_addr - (long)rbtree_body;//-1;
		header->first_free_cell = (long)nil_cell_addr + sizeof(RBTreeNode) - (long)rbtree_body;
		header->nil_cell = (long)nil_cell_addr - (long)rbtree_body;
		header->growth = growth;
	
//		write(rbtree_fd, rbtree_addr, rbtFileLen);
		rbtree_f.Write(rbtree_addr, rbtFileLen);

		try
		{
		    trans_f.RWCreate(trans_file.c_str());
		    trans_f.SetUnbuffered();
		}
		catch(FileException ex)
		{
//		    smsc_log_debug(logger, "FSStorage: error idx_file - %s\n", ex.what());
		    rbtree_f.Close();
		    return CANNOT_CREATE_TRANS_FILE;
		}
		int status = STAT_OK;
		trans_f.Write((char*)&status, sizeof(int));

		//printf("Create file return \n");
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
			
		if(!(rbtree_addr = (caddr_t)malloc(len)))
		{
			//printf("malloc failed\n");
			return BTREE_FILE_MAP_FAILED;
		}

		rbtree_f.Read(rbtree_addr, len);
		
		// set header pointer to header area of RBT File
		header = (rbtFileHeader*)rbtree_addr;
		rbtree_body = rbtree_addr + sizeof(rbtFileHeader);
		//printf("header->cells_used %d\n", header->cells_used);
		return ret;
	}
	int startTransaction(void)
	{
	    transFileHeader		hdr;
	    list<RBTreeNode*>::iterator It;
	    long			nodeAddr;

	    hdr.status = STAT_WRITE_TRX;
	    hdr.version = TRX_VER_1;
	    hdr.operation = currentOperation;
	    hdr.nodes_count = changedNodes.size();
	
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
	    //printf("Write Changes\n");
	    getchar();
	    int stat = STAT_WRITE_RBT;
	    trans_f.Seek(0, SEEK_SET);
	    trans_f.Write((char*)&stat, sizeof(int));

	    list<RBTreeNode*>::iterator It;
	    rbtree_f.Seek(0, SEEK_SET);
	    rbtree_f.Write((char*)header, sizeof(rbtFileHeader));

	    for(It = changedNodes.begin(); It != changedNodes.end(); It++)
	    {
		//printf("0x%p\n", *It);
		//printf("0x%x\n", (long)*It - (long)rbtree_body);
		rbtree_f.Seek((long)*It - (long)rbtree_addr , SEEK_SET);
		rbtree_f.Write((char*)*It, sizeof(RBTreeNode));
		
	    }
	    return 0;
	}
	int endTransaction()
	{
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
		
		//printf("repairRBTreeFile transHdr.nodes_count = \n", transHdr.nodes_count);
		
		for(int i = 0; i < transHdr.nodes_count; i++)
		{
			trans_f.Read((char*)&nodeAddr, sizeof(long));
			trans_f.Read((char*)&curNode, sizeof(RBTreeNode));
			rbtree_f.Seek(nodeAddr, SEEK_SET);
			rbtree_f.Write((char*)&curNode, sizeof(RBTreeNode));
		}
		return 0;
	}
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

