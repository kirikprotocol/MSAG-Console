#include "HttpChunked.h"

namespace scag2 { namespace transport { namespace http
{

const char* ChunkInfo::crlf = "\r\n";

ChunkItem::ChunkItem(std::string s) : header(s) {
	dataSize = fromHexString<unsigned int>(s);
	sendSize = s.length() + strlen(ChunkInfo::crlf) + dataSize + strlen(ChunkInfo::crlf);
}
/*
unsigned int ChunkInfo::prepare(TmpBuf* buf, char* src) {
	char* tmp;
	unsigned int rc = 0;
	buf.SetPos(0);
	if ( chunks.size() ) {
		rc = chunks.front().dataSize;
		buf.Append(chunks.front().header, chunks.front().header.length());
		unparsed.Append(ChunkInfo::crlf, strlen(ChunkInfo::crlf));
		if ( rc ) {
			tmp = src;
			tmp += position;
			buf.Append(tmp, rc);
			buf.Append(ChunkInfo::crlf, strlen(ChunkInfo::crlf));
		}
		chunks.front().sendSize = buf.GetPos();
	}
	return rc;
}
*/
}}}
