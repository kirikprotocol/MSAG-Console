/*
	$Id$
*/

#include "util/debug.h"
#include <inttypes.h>
#include <algorithm>

#define SMPP_USE_BUFFER
#define SMPP_SHARE_BUFFER_MEMORY

#define smartMalloc(x) malloc(x)
#define smartFree(x)   free(x)

#if defined SMPP_USE_BUFFER
#define __check_stream_invariant__ ( stream) \
	__require__ ( stream->buffer != NULL ); \
	__require__ ( stream->dataLength >= 0 ); \
	__require__ ( stream->dataOffset >= 0 ); \
	__require__ ( stream->bufferSize >= 0 ); \
	__require__ ( stream->bufferSize >= stream->dataLength ); \
	__require__ ( stream->dataOffset <= stream->dataLength );
#else
#define __check_stream_invariant__ ( stream) \
	__require__ ( stream->chanel != NULL ); \
	__require__ ( stream->dataLength >= 0 ); \
	__require__ ( stream->dataOffset >= 0 ); \
	__require__ ( stream->dataOffset <= stream->dataLength );
#endif

namespace smsc {
namespace smpp {

struct SmppStream
{
	SmppHeader header;
	unsigned int dataLength;
	unsigned int dataOffset;
#if defined SMPP_USE_BUFFER
	unsigned int bufferSize;
	unsigned char* buffer;
#else
	??? chanel;
#endif
};

template < class T >
inline T& fetchX(SmppStream* stream,T& data)
{
#if defined SMPP_USE_BUFFER	
	__check_smpp_stream_invariant__ ( stream );
	__require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
	data = *((T*)stream->buffer);
	stream->dataOffset+sizeof(T);
	return data;
#else
#error "undefined rules of fetchX"
#endif
}

/**
	C Octet String
*/
struct COStr
{
	// pointer on text
	char* text;
	COStr() : text(0) {};
	~COStr()
	{
#if defined (SMPP_SHARE_BUFFER_MEMORY)
	// nothing
#else
	if ( text )
		smartFree(text);
#endif
	}
	operator const char*(){__require(text!=NULL)__; return text;}
	const char* cstr(){__require(text!=NULL)__; return text;}
	operator void* new(size_t size) { return smartMalloc(size); }
	operator void delete(void* mem) { smartFree(mem); }
};

struct TVL
{
	int16_t tag;
	int16_t length;
	int8_t* value;
	TVL() : value(0),tag(0),length(0) {};
	~TVL()
	{
#if defined (SMPP_SHARE_BUFFER_MEMORY)
	// nothing
#else
	if ( value )
		smartFree(value);
#endif
	}
	operator void* new(size_t size) { return smartMalloc(size); }
	operator void delete(void* mem) { smartFree(mem); }
};

struct SmppHeader
{
	int32_t command_length;
	int32_t command_id;
	int32_t command_status;
	int32_t sequence_number;
};

inline void fetchSmppHeader(SmppStream* stream,SmppHeader& header)
{
	__check_smpp_stream_invariant__ ( stream );
	header = stream->header;
}

/**
	Вытаскиваем из потока строку
	@param stream  поток
	@param costr   строка
	В случае возникновения ошибки бросает исключение
*/
inline void fetchSmppCOctStr(SmppStream* stream,COStr& costr,int cOctMax)
{
	int length = 0;
	int maxLength;
	__check_smpp_stream_invariant__ ( stream );
	
	maxLength = std::min ( stream->dataLength-stream->dataOffset, cOctMax );

#if defined SMPP_SHARE_BUFFER_MEMORY
{
	char* tmp = stream->buffer+stream->dataOffset;
	for ( ; (length < maxLength) && (*tmp != 0) ; ++length,++tmp );
	if (*tmp) throw BadStreamException();
	costr.text = buffer;
}
#else // use memory manager & stream access
{
	costr.text = smartMalloc(maxLength);
	for ( ;length < maxLength; ++length)
	{
		if ( fetchX(stream,costr.text[length]) == 0 ) goto success;
	}
	throw BadStreamException();
success: ;
}
#endif
	
	stream->dataOffset += length;
	__check_smpp_stream_invariant__ ( stream );
}

/**
	Достаем вариантное поле
*/
inline void fetchSmppTVL(SmppStream* stream,TVL& tvl)
{
	__check_smpp_stream_invariant__ ( stream );
	fetchX(stream,tvl.tag);
	fetchX(stream,tvl.length);
	__require__ ( tv.length >= 1 );
#if defined SMPP_SHARE_BUFFER_MEMORY
{
	tvl.value=stream->buffer+stream->dataOffset;
	stream->buffer+=tvl.length;
	stream->dataOffset+=tvl.length;
}
#else
{
	int i=0;
	tvl.value = smartMalloc(tvl.length);
	__require__ ( tvl.value != NULL );
	for (;i<tvl.length; ++i)
	{
		fetchX(stream,tvl.value[i]);
	}
}
#endif
	__check_smpp_stream_invariant__ ( stream );
}

struct PduBindTransmitter
{
	SmppHeader header;
	COStr system_id;
	COStr password;
	COStr system_type;
	int8_t interface_version;
	int8_t addr_ton;
	int8_t addr_npi;
	COStr address_range;
};

struct PduBindReceiver
{
	SmppHeader header;
	COStr system_id;
	COStr password;
	COStr system_type;
	int8_t interface_version;
	int8_t addr_ton;
	int8_t addr_npi;
	COStr address_range;
};

struct PduBindTranceiver
{
	SmppHeader header;
	COStr system_id;
	COStr password;
	COStr system_type;
	int8_t interface_version;
	int8_t addr_ton;
	int8_t addr_npi;
	COStr address_range;
};

struct PduBindReceiverResp
{
	SmppHeader header;
	COStr system_id;
	TVL sc_interface_version;
};

struct PduBindTranceiverResp
{
	SmppHeader header;
	COStr system_id;
	TVL sc_interface_version;
};

struct PduOutBind
{
	SmppHeader header;
	COStr system_id;
	COStr password;
};

struct PduUnbind
{
	SmppHeader header;
};

struct PduUnbindResp
{
	SmppHeader header;
};

struct PduGenericNack
{
	SmppHeader header;
};

struct PduSubmitSm
{
	SmppHeader header;
	COStr service_type;
	int8_t source_addr_ton;
	int8_t source_addr_npi;
	COStr source_addr;
	int8_t dest_addr_ton;
	int8_t dest_addr_npi;
	COStr destination_addr;
	int8_t esm_class;
	int8_t protocol_id;
	int8_t priority_flag;
	COStr schedule_delivery_time;
	COStr validity_p
};

}; // smpp namespace
}; // smsc namespace
