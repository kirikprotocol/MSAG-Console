/*
	$Id$
*/

#if !defined  __Cxx_Header__smpp_stream_h__
#define  __Cxx_Header__smpp_stream_h__

#include "util/debug.h"
#include "smpp_memory.h"
#include "smpp_structures.h"
#include <algorighm>

namespace smsc{
namespace smpp{

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
};

class BadStreamException : public exception
{
public:
  const char* what() {return "bad stream" } const;
};

inline  __check_stream_invariant__ (SmppStream* stream)
{
#if defined SMPP_USE_BUFFER
  __require__ ( stream->buffer != NULL ); 
  __require__ ( stream->dataLength >= 0 ); 
  __require__ ( stream->dataOffset >= 0 ); 
  __require__ ( stream->bufferSize >= 0 ); 
  __require__ ( stream->bufferSize >= stream->dataLength ); 
  __require__ ( stream->dataOffset <= stream->dataLength );
#else
  __require__ ( stream->chanel != NULL ); 
  __require__ ( stream->dataLength >= 0 ); 
  __require__ ( stream->dataOffset >= 0 ); 
  __require__ ( stream->dataOffset <= stream->dataLength );
	__require__ ( stream->chanel >= 0 );
#endif /*SMPP_USE_BUFFER*/
}


template<class T>
inline T& __fetch_x__ (SmppStream* stream, T& data)
{
#if defined SMPP_USE_BUFFER 
  __check_smpp_stream_invariant__ ( stream );
  __require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
  data = *((T*)stream->buffer);
  stream->dataOffset+sizeof(T);
  return data;
#else
//#error "undefined rules of fetchX"
  __check_smpp_stream_invariant__ ( stream );
  __require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
  //data = *((T*)stream->buffer);
	int wasread = read(stream->chanel,data,sizeof(T));
	__throw_if_fail__(wasread==sizeof(T),BadStreamException);
  stream->dataOffset+sizeof(T);
  return data;
#endif
}

inline uint8_t& fetchX(SmppStream* s,uint8_t& d){return __fetch_x__(s,d);}
inline int8_t& fetchX(SmppStream* s,int8_t& d){return __fetch_x__(s,d);}
inline uint16_t& fetchX(SmppStream* s,uint16_t& d){__fetch_x__(s,d);d = ntohs(d); return d;}
inline int16_t& fetchX(SmppStream* s,int16_t& d){__fetch_x__(s,d);d = (int16_t)ntohs((uint16_t)d); return d;}
inline uint32_t& fetchX(SmppStream* s,uint32_t& d){__fetch_x__(s,d);d = ntohl(d); return d;}
inline int32_t& fetchX(SmppStream* s,int32_t& d){__fetch_x__(s,d);d = (int16_t)ntohl((uint16_t)d); return d;}


inline dropPdu(SmppStream* stream)
{
  __check_smpp_stream_invariant__ ( stream );
#if defined SMPP_USE_BUFFER 
  stream->dataOffset=stream->dataLength;
#else
	static char data[1024];
	while ( stream->dataOffset != strem->dataLength )
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
inline void assignStreamWith(SmppStream* stream,void* buffer,int bufferSize)
#else
inline void assignStreamWith(SmppStream* stream,int chanel)
#endif
{
  __require__ ( buffer != NULL );
  __require__ ( stream != NULL );
  __require__ ( bufferSize >= 16 );
#if defined SMPP_USE_BUFFER
  stream->buffer = buffer;
#else
  strem->chanel = chanel;
	__require__ ( chanel > 0 );
#endif  
  stream->dataOffset = 0;
  stream->bufferSize = bufferSize;
  stream->dataLength = 4;
  fetchX(stream,stream->header.commandLength);
  stream->dataLength = header.commandLength;
  fetchX(stream,stream->header.commandId);
  fetchX(stream,stream->header.commandStatus);
  fetchX(stream,stream->header.sequenceNumber);
}

inline void fetchSmppHeader(SmppStream* stream,SmppHeader& header)
{
  __check_smpp_stream_invariant__ ( stream );
  header = stream->header;
}

inline int32_t smppCommandId(SmppStream* stream)
{
  __check_smpp_stream_invariant__ ( stream );
  return stream->header.commandId;
}

};
};

#endif
