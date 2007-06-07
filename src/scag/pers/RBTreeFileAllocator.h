//------------------------------------
//  RBTreeFileAllocator.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	Файл содержит описание класса RBTreeFileAllocator.
//

#include "RBTreeAllocator.h"
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/mman.h"

extern int errno;

#ifndef ___RBTREE_FILE_Allocator_H
#define ___RBTREE_FILE_Allocator_H

using namespace std;

const string sPREAMBLE = "RBTREE_FILE_STORAGE!";
const int version_32_1 = 0x01;
const int version_64_1 = 0x80000001;

const int mode = 0600;
const off_t	default_growth = 1000000;
//const off_t	default_growth = 1000;

const int SUCCESS					= 0;
const int RBTREE_FILE_NOT_SPECIFIED	= -1;
const int CANNOT_CREATE_RBTREE_FILE	= -2;
const int CANNOT_OPEN_RBTREE_FILE	= -3;
const int BTREE_FILE_MAP_FAILED		= -4;
const int RBTREE_FILE_ERROR			= -10;
const int ATTEMPT_INIT_RUNNING_PROC	= -20;

template<class Key=long, class Value=long>
class RBTreeFileAllocator: public RBTreeAllocator<Key, Value>
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

	struct free_cell_list
	{
		long next_free_cell;
	};

public:
	RBTreeFileAllocator(const string& _rbtree_file="", off_t _growth = default_growth, bool clearFile = false):
		rbtree_addr(0), rbtree_fd(-1), growth(_growth), running(false)
	{
		if(_rbtree_file.length() > 0)
			Init(_rbtree_file, _growth, clearFile);
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
		growth = _growth;
		
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

	virtual ~RBTreeFileAllocator()
	{
		if(running)
		{
			if(rbtree_addr) munmap(rbtree_addr, rbtFileLen);
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
private:
	string			rbtree_file;
	int				rbtree_fd;
	caddr_t			rbtree_addr;
	caddr_t			rbtree_body;
	off_t			growth;
	off_t			rbtFileLen;
	bool			running;
	rbtFileHeader*	header;
	
	
	bool isFileExists(void)
	{
		struct ::stat st;
		return ::stat(rbtree_file.c_str(),&st)==0;
	}
	int CreateRBTreeFile(void)
	{
		rbtFileLen = growth*sizeof(RBTreeNode) + sizeof(rbtFileHeader) + sizeof(RBTreeNode);
//		printf("sizeof(RBTreeNode) = %d\n", sizeof(RBTreeNode));
		//printf("sizeof(n.parent) = %d\n", sizeof(n.parent));
		//printf("sizeof(n.left) = %d\n", sizeof(n.left));
		//printf("sizeof(n.right) = %d\n", sizeof(n.right));
		//printf("sizeof(n.color) = %d\n", sizeof(n.color));
		//printf("sizeof(n.key) = %d\n", sizeof(n.key));
		//printf("sizeof(n.value) = %d\n", sizeof(n.value));
		//printf("sizeof(Abnt1) = %d\n", sizeof(Abnt1));
		//printf("sizeof(n) = %d\n", sizeof(n));
		if(-1 == (rbtree_fd = open(rbtree_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode)))
			return CANNOT_CREATE_RBTREE_FILE;
		lseek(rbtree_fd, rbtFileLen-1, SEEK_SET);
		write(rbtree_fd, " ", 1);
		lseek(rbtree_fd, 0, SEEK_SET);
//		printf("%d\n", rbtFileLen);
	    if(MAP_FAILED == (rbtree_addr = mmap((caddr_t)0, rbtFileLen, PROT_READ | PROT_WRITE, MAP_SHARED, rbtree_fd, 0)))
		{
//			perror("mmap: ");
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
		memcpy((void*)&(header->preamble), sPREAMBLE.c_str(), sizeof(header->preamble));
		header->version = version_32_1;
		header->cells_count = growth;
		header->cells_used = 1;
		header->cells_free = growth - 1;
		header->root_cell = (long)nil_cell_addr - (long)rbtree_body;//-1;
		header->first_free_cell = (long)nil_cell_addr + sizeof(RBTreeNode) - (long)rbtree_body;
		header->nil_cell = (long)nil_cell_addr - (long)rbtree_body;
		header->growth = growth;
		
		printf("Create file return \n");
		return SUCCESS;
	}
	int OpenRBTreeFile(void)
	{
		if(-1 == (rbtree_fd = open(rbtree_file.c_str(), O_RDWR)))
			return CANNOT_OPEN_RBTREE_FILE;
		
		off_t len = lseek(rbtree_fd, 0, SEEK_END);
		lseek(rbtree_fd, 0, SEEK_SET);
	    
		if(MAP_FAILED == (rbtree_addr = mmap((caddr_t)0, len, PROT_READ | PROT_WRITE, MAP_SHARED, rbtree_fd, 0)))
			return BTREE_FILE_MAP_FAILED;

		// set header pointer to header area of RBT File
		header = (rbtFileHeader*)rbtree_addr;
		rbtree_body = rbtree_addr + sizeof(rbtFileHeader);
		return SUCCESS;
	}
};

#endif
