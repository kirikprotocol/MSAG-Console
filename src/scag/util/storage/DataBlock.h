//------------------------------------
//  DataBlock.h
//  Routman Michael, 2007
//------------------------------------
//
//	Файл содержит описание класса DataBlock.
//

#ifndef _SCAG_UTIL_STORAGE_DATABLOCK_H
#define _SCAG_UTIL_STORAGE_DATABLOCK_H

#include "string.h"
#include <cstdio>

namespace scag {
namespace util {
namespace storage {

//template<int buff_size>
class DataBlock
{
private:
  static const int start_buff_len = 1024;

public:
	DataBlock(const char* _block, int _block_len):block(0), block_len(_block_len), buff_len(start_buff_len), pos(0)
	{
		if(block_len >= buff_len) buff_len = 2 * block_len;
		block = new char[buff_len];
		if(_block && block_len > 0)
		{
			memcpy((void*)block, (void*)_block, block_len);
			pos += block_len;
		}
	}
	DataBlock(int _buff_len = start_buff_len):block(0), block_len(0), buff_len(_buff_len), pos(0)
	{
		if(buff_len > 0)
			block = new char[buff_len];
	}
	DataBlock(const DataBlock& _block):block(0), block_len(_block.block_len), buff_len(start_buff_len), pos(_block.pos)
	{
		if(block_len >= buff_len) buff_len = 2 * block_len;
		block = new char[buff_len];
		if(_block.block)
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

	inline const char* c_ptr(void) const
	{
		return block;
	}
        inline const char* c_curPtr(void) const
	{
		return block+pos;
	}
	inline int length(void) const
	{
		return block_len;
	}
        inline int getPos(void) const
	{
		return pos;
	}

    void reserve( int sz )
    {
        if ( sz < buff_len ) return;
        char* temp = block;
        buff_len = sz;
        block = new char[buff_len];
        if ( temp ) {
            if ( block_len ) memcpy( (void*)block, temp, block_len );
            delete[] temp;
        }
    }

	int setPos(int _pos)
	{
		if(_pos < block_len)
			return pos = _pos;
		if(_pos >= buff_len)
		{
                    reserve(_pos*2);
		}
		return block_len = pos = _pos;
	}
	

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// НЕ СТОИТ использовать  функцию ptr() без особой надобности - она нарушает инкапсуляцию класса. 
// В случае использования, внимательно посмотри как устроен класс и приведи все его переменные в порядок
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
	char*	block;
	int		block_len;
	int		buff_len;
	int		pos;
	
	void print(const char* str)
	{
            std::printf("%s pos = %d ", str, pos);
            std::printf("buff_len = %d ", buff_len);
            std::printf("block_len = %d\n", block_len);
	}
};

} // namespace storage
} // namespace util
} // namespace scag
    
#endif /* ! _SCAG_UTIL_STORAGE_DATABLOCK_H */
