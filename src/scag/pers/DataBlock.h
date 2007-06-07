//------------------------------------
//  DataBlock.h
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ DataBlock.
//

#ifndef ___DATA_BLOCK_H
#define ___DATA_BLOCK_H

#include "string.h"

class DataBlock
{
public:
	DataBlock(const char* _block=0, int _block_len = 0):block(0), block_len(0), buff_len(0), pos(0)
	{
		if(_block && _block_len > 0)
		{
			buff_len = block_len = _block_len;
			block = new char[buff_len];
			memcpy((void*)block, (void*)_block, block_len);
			pos += block_len;
		}
	}
	DataBlock(int _buff_len):block(0), block_len(0), buff_len(_buff_len), pos(0)
	{
		if(buff_len > 0)
			block = new char[buff_len];
	}
	DataBlock(const DataBlock& _block):block(0), block_len(0), buff_len(0), pos(0)
	{
		if(!_block.block)
			return;
		pos = block_len = buff_len = _block.block_len;
		block = new char[buff_len];
		memcpy((void*)block, (void*)(_block.block), block_len);
	}
	~DataBlock(void)
	{
		if(block)
			delete[] block;
	}

	void setBuffLength(int _buff_len)
	{
		if(buff_len < _buff_len)
		{
			if(block) delete[] block;
			buff_len = _buff_len;
			block = new char[buff_len];
			pos = block_len = 0;
		}
	}

	DataBlock& operator=(const DataBlock& _block)
	{
		if(this != &_block)
		{
			if(!_block.block) // || (_block.block_len == 0) || (_block.buff_len == 0) )
			{
				pos = block_len = 0;
				return *this;
			}
			if(buff_len < _block.block_len)
			{
				if(block) delete[] block;
				buff_len = _block.block_len;
				block = new char[buff_len];
			}
			block_len = _block.block_len;
			memcpy((void*)block, (void*)&(_block.block), block_len);
			pos = _block.pos;
		}
		return *this;
	}
	DataBlock& blkset(char ch, int len)
	{
		if(buff_len < len)
		{
			if(block) delete[] block;
			buff_len = len;
			block = new char[buff_len];
		}
		pos = block_len = len;
		memset((void*)block, ch, block_len);
		return *this;
	}
	DataBlock& blkcpy(const char* buff, int len)
	{
		if(buff_len < len)
		{
			if(block) delete[] block;
			buff_len = len;
			block = new char[buff_len];
		}
		pos = block_len = len;
		memcpy((void*)block, buff, block_len);
		return *this;
	}
	DataBlock& blkadd(const char* buff, int len)
	{
		if(buff_len <  block_len + len)
		{
			char* temp = block;
			buff_len = block_len + len;
			block = new char[buff_len];
			if(temp)
			{
				memcpy((void*)block, temp, block_len);
				delete[] temp;
			}
		}
		memcpy((void*)(block+block_len), buff, len);
		block_len += len;
		return *this;
	}

	int blkwrite(const char* buff, int len, int from_pos = -1)
	{
		if(from_pos == -1) from_pos = pos;
		if(buff_len <  from_pos + len)
		{
			char* temp = block;
			buff_len = (from_pos + len)*2;
			block = new char[buff_len];
			if(temp)
			{
				memcpy((void*)block, temp, from_pos);
				delete[] temp;
			}
		}
		memcpy((void*)(block+from_pos), buff, len);
		pos = from_pos + len;
		if(pos > block_len) block_len = pos;
//		print("DataBlock::blkwrite():");
		return len;
	}
	int blkread(const char* buff, int len, int from_pos = -1)
	{
		if(from_pos >= block_len)
		{
			pos = block_len;
			return 0;
		}
		if(from_pos == -1) from_pos = pos;
		if(len > block_len) len = block_len;
		memcpy((void*)buff, (void*)(block + from_pos), len);
		pos = from_pos + len;
		return len;
	}

	const char* c_ptr(void) const
	{
		return block;
	}
	const char* c_curPtr(void) const
	{
		return block+pos;
	}
	int length(void) const
	{
		return block_len;
	}
	int getPos(void) const
	{
		return pos;
	}
	int setPos(int _pos)
	{
		if(_pos < block_len)
			return pos = _pos;
		if(_pos >= buff_len)
		{
			char* temp = block;
			buff_len = (_pos)*2;
			block = new char[buff_len];
			if(temp)
			{
				memcpy((void*)block, temp, block_len);
				delete[] temp;
			}
		}
		return block_len = pos = _pos;
	}
	

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// �� ����� ������������  ������� ptr() ��� ������ ���������� - ��� �������� ������������ ������. 
// � ������ �������������, ����������� �������� ��� ������� ����� � ������� ��� ��� ���������� � �������
//
	char* ptr(void)
	{
		return block;
	}
	void setLength(int len)
	{
		block_len = len;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	int		block_len;
	int		buff_len;
	char*	block;
	
	int		pos;
	
	void print(const char* str)
	{
		printf("%s pos = %d ", str, pos);
		printf("buff_len = %d ", buff_len);
		printf("block_len = %d\n", block_len);
	}
};

#endif
