/*
	$Id$
*/

#if !defined  __Cxx_Header__smpp_stream_h__
#define  __Cxx_Header__smpp_stream_h__

#include "util/debug.h"
#include "smpp_memory.h"
#include "smpp_structures.h"
#include "smpp_strings.h"
#include <algorithm>
#include <exception>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace smsc{
namespace smpp{

//using std::min;
//using std::max;

struct SmppStream
{
  SmppHeader header;
  unsigned int dataLength;
  unsigned int dataOffset;
#if defined SMPP_USE_BUFFER
  unsigned int bufferSize;
  unsigned char* buffer;
#else
  int chanel;
#endif
	bool readable;
};

class BadStreamException {}	;
/*: public std::exception
{
public:
  const char* what() {return "bad stream"; } const
};*/

inline  void __check_smpp_stream_invariant__ (SmppStream* stream)
{
#if defined SMPP_USE_BUFFER
  __require__ ( stream->buffer != NULL ); 
  __require__ ( stream->dataLength >= 0 ); 
  __require__ ( stream->dataOffset >= 0 ); 
  __require__ ( stream->bufferSize >= 0 ); 
  __require__ ( stream->bufferSize >= stream->dataLength ); 
  __require__ ( stream->dataOffset <= stream->dataLength );
#else
  //__require__ ( stream->chanel != 0 ); 
  __require__ ( stream->dataLength >= 0 ); 
  __require__ ( stream->dataOffset >= 0 ); 
  __require__ ( stream->dataOffset <= stream->dataLength );
	__require__ ( stream->chanel > 0 );
#endif /*SMPP_USE_BUFFER*/
}

inline  void __check_smpp_stream_is_readable__ (SmppStream* stream)
{
	__require__ ( stream != NULL );
	__require__ ( stream->readable );
}

inline  void __check_smpp_stream_is_writable__ (SmppStream* stream)
{
	__require__ ( stream != NULL );
	__require__ ( !stream->readable );
}


template<class T>
inline T& __fetch_x__ (SmppStream* stream, T& data)
{
#if defined SMPP_USE_BUFFER 
  __check_smpp_stream_invariant__ ( stream );
	__check_smpp_stream_is_readable__(stream);
  __require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
  data = *((T*)stream->buffer);
  stream->dataOffset+sizeof(T);
  return data;
#else
//#error "undefined rules of fetchX"
  __check_smpp_stream_invariant__ ( stream );
	__check_smpp_stream_is_readable__(stream);
  __require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
  //data = *((T*)stream->buffer);
	int wasread = read(stream->chanel,data,sizeof(T));
	__throw_if_fail__(wasread==sizeof(T),BadStreamException);
  stream->dataOffset+sizeof(T);
  return data;
#endif
}

template<class T>
inline void __fill_x__ (SmppStream* stream, T& data)
{
#if defined SMPP_USE_BUFFER 
  __check_smpp_stream_invariant__ ( stream );
	__check_smpp_stream_is_writable__(stream);
  __require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
  *((T*)stream->buffer) = data;
  stream->dataOffset+sizeof(T);
#else
//#error "undefined rules of fetchX"
  __check_smpp_stream_invariant__ ( stream );
	__check_smpp_stream_is_writable__(stream);
  __require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
  //data = *((T*)stream->buffer);
	int writen = write(stream->chanel,data,sizeof(T));
	__throw_if_fail__(writen==sizeof(T),BadStreamException);
  stream->dataOffset+sizeof(T);
  return data;
#endif
}

inline uint8_t& fetchX(SmppStream* s, uint8_t& d){return __fetch_x__(s,d);}
inline int8_t& fetchX(SmppStream* s, int8_t& d){return __fetch_x__(s,d);}
inline uint16_t& fetchX(SmppStream* s,uint16_t& d){__fetch_x__(s,d);d = ntohs(d); return d;}
inline int16_t& fetchX(SmppStream* s,int16_t& d){__fetch_x__(s,d);d = (int16_t)ntohs((uint16_t)d); return d;}
inline uint32_t& fetchX(SmppStream* s,uint32_t& d){__fetch_x__(s,d);d = ntohl(d); return d;}
inline int32_t& fetchX(SmppStream* s,int32_t& d){__fetch_x__(s,d);d = (int16_t)ntohl((uint16_t)d); return d;}

inline void fillX(SmppStream* s,const uint8_t& d){__fill_x__(s,d);}
inline void fillX(SmppStream* s,const int8_t& d){__fill_x__(s,d);}
inline void fillX(SmppStream* s,const uint16_t& d){uint16_t x = htons(d); __fill_x__(s,x);}
inline void fillX(SmppStream* s,const int16_t& d){ int16_t x = (int16_t)htons((uint64_t)d);__fill_x__(s,x);}
inline void fillX(SmppStream* s,const uint32_t& d){ uint32_t x = htonl(d); __fill_x__(s,x);}
inline void fillX(SmppStream* s,const int32_t& d){ int32_t x = (int32_t)htonl((uint32_t)d); __fill_x__(s,x);}


inline void dropPdu(SmppStream* stream)
{
  __check_smpp_stream_invariant__ ( stream );
#if defined SMPP_USE_BUFFER 
  stream->dataOffset=stream->dataLength;
#else
	static char data[1024];
	while ( stream->dataOffset != stream->dataLength )
	{
		int wasread = read(stream->chanel,data,
											 std::min( sizeof(data), stream->dataLength-stream->dataOffset) );
		__throw_if_fail__(wasread>0,BadStreamException);
		stream->dataOffset+=wasread;
	}
//#error "undefined rules dropPdu"
#endif
}

#if defined SMPP_USE_BUFFER
inline void assignStreamWith(SmppStream* stream,void* buffer,int bufferSize,bool readable)
#else
inline void assignStreamWith(SmppStream* stream,int chanel, bool readable)
#endif
{
  __require__ ( stream != NULL );
#if defined SMPP_USE_BUFFER
  __require__ ( bufferSize >= 16 );
  __require__ ( buffer != NULL );
  stream->buffer = buffer;
  stream->bufferSize = bufferSize;
#else
  __require__( chanel > 0 );
	stream->chanel = chanel;
#endif  
  stream->dataOffset = 0;
  if ( readable )
	{
		stream->dataLength = 4;
		fetchX(stream,stream->header.commandLength);
		stream->dataLength = stream->header.commandLength;
		fetchX(stream,stream->header.commandId);
		fetchX(stream,stream->header.commandStatus);
		fetchX(stream,stream->header.sequenceNumber);
	}
	else
		stream->dataLength = 0;
	stream->readable = readable;
}

inline void fetchSmppHeader(SmppStream* stream,SmppHeader& header)
{
  __check_smpp_stream_invariant__ ( stream );
	__check_smpp_stream_is_readable__(stream);
  header = stream->header;
}

inline void fillSmppHeader(SmppStream* stream,SmppHeader& header)
{
  __check_smpp_stream_invariant__ ( stream );
	__check_smpp_stream_is_writable__(stream);
  stream->header = header;
  stream->dataOffset = 0;
  stream->dataLength = header.commandLength;
	fillX(stream,header.commandLength);
	fillX(stream,header.commandId);
	fillX(stream,header.commandStatus);
	fillX(stream,header.sequenceNumber);
  __check_smpp_stream_invariant__ ( stream );
}

inline int32_t smppCommandId(SmppStream* stream)
{
  __check_smpp_stream_invariant__ ( stream );
  return stream->header.commandId;
}

inline void fillCOctetStr(SmppStream* stream,COStr& costr)
{
  __check_smpp_stream_invariant__ ( stream );
	__require__ ( costr.cstr() != NULL );
	const char* text = costr.cstr();
	const char* endText = text+strlen(text)+1;
	__require__ ( *(endText-1) == 0 );
	while ( text != endText ) fillX(stream,*(uint8_t*)(text++));
  __check_smpp_stream_invariant__ ( stream );
}
/**
  Вытаскиваем из потока строку
  @param stream  поток
  @param costr   строка
  В случае возникновения ошибки бросает исключение
*/
inline void fetchCOctetStr(SmppStream* stream,COStr& costr,int cOctMax)
{
  int length = 0;
  int maxLength;
  __check_smpp_stream_invariant__ ( stream );
  
  maxLength = std::min ( stream->dataLength-stream->dataOffset, (uint32_t)cOctMax );

#if defined SMPP_SHARE_BUFFER_MEMORY
//{
  char* tmp = stream->buffer+stream->dataOffset;
  for ( ; (length < maxLength) && (*tmp != 0) ; ++length,++tmp );
  __throw_if_fail__ ( *tmp == 0 , BadStreamException );
  costr.text = stream->buffer;
  if ( length )
    stream->dataOffset+=length+1;
  else costr.text = NULL;
//}
#else // use memory manager & stream access
//{
  int8_t oct;
  __require__( costr.text == NULL );
  fetchX(stream,oct);
  if ( oct )
  {
    length = 1;
    costr.text = (char*)smartMalloc(maxLength);
    costr.text[0] = oct;
    for ( ;length < maxLength; ++length)
    {
      if ( fetchX(stream,costr.text[length]) == 0 ) goto success;
    }
    throw BadStreamException();
  }
  else
  {
    costr.text = NULL;
  }
  success: ;
//}
#endif
  __check_smpp_stream_invariant__ ( stream );
}

inline void fillOctetStr(SmppStream* stream,OStr& ostr)
{
  __check_smpp_stream_invariant__ ( stream );
	__require__ ( ostr.cstr() != NULL );
	const char* text = ostr.cstr();
	const char* endText = text+ostr.size();
	while ( text != endText ) fillX(stream,*(uint8_t*)(text++));
  __check_smpp_stream_invariant__ ( stream );
}

/**
  Вытаскиваем из потока строку
  @param stream  поток
  @param ostr   строка
  В случае возникновения ошибки бросает исключение
*/
inline void fetchOctetStr(SmppStream* stream,OStr& ostr,uint32_t octets)
{
  __check_smpp_stream_invariant__ ( stream );
  __require__ ( octets >= 0 && octets <= (stream->dataLength-stream->dataOffset) );
  if ( octets )
	{
#if defined SMPP_SHARE_BUFFER_MEMORY
		ostr.text = stream->buffer;
		stream->dataOffset += octets;
#else // use memory manager & stream access
		uint16_t length = 0;
		__require__( ostr.text == NULL );
		ostr.text = (char*)smartMalloc(octets);
		for ( ;length < octets; ++length)
		{
			fetchX(stream,ostr.text[length]);
		}
#endif
	}
  else
    ostr.text = NULL;
  ostr.length = octets;
  __check_smpp_stream_invariant__ ( stream );
}

};
};

#endif
