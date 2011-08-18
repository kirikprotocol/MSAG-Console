#ifndef SCAG_TRANSPORT_HTTP_CHUNKED
#define SCAG_TRANSPORT_HTTP_CHUNKED

#include <queue>
#include <sstream>
#include "core/buffers/TmpBuf.hpp"

namespace scag2 { namespace transport { namespace http
{
using smsc::core::buffers::TmpBuf;

#define DFLT_BUF_SIZE 32
//typedef TmpBuf<char,size_t> mybuf;

template<class T>
static T fromHexString(const std::string& s)
{
     std::stringstream stream;
     T t;
     stream << std::hex << s;
     stream >> t;
     return t;
}

struct ChunkItem {
	std::string header;
	unsigned int dataSize;
	unsigned int sendSize;
	ChunkItem() : header(""), dataSize(0), sendSize(0) {};
	ChunkItem(std::string s);
};

class ChunkInfo {
public:
	ChunkInfo() {};
    ~ChunkInfo() {};

    unsigned int add(std::string value) {
    	chunks.push(ChunkItem(value));
    	return chunks.back().dataSize;
    }
    void remove(void) {
    	if (chunks.size())
    		chunks.pop();
    }
    unsigned int size(void) {
    	return chunks.size();
    }

    unsigned int getData(void) {
    	return chunks.size() ? chunks.front().dataSize : 0;
    }
    unsigned int getSend(void) {
    	return chunks.size() ? chunks.front().sendSize : 0;
    }
    void setSend(unsigned int value) {
    	if (chunks.size())
    		chunks.front().sendSize = value;
    }
    std::string getHeader(void) {
    	return chunks.size() ? chunks.front().header : std::string("");
    }
    unsigned int getLastData(void) {
    	return chunks.size() ? chunks.back().dataSize : 0;
    }
//    unsigned int prepare(TmpBuf* buf, char* src);

public:
	static const char* crlf;

protected:
    std::queue<ChunkItem> chunks;
/*
    template<class T>
    static T fromHexString(const std::string& s)
    {
         std::stringstream stream;
         T t;
         stream << std::hex << s;
         stream >> t;
         return t;
    }
*/
};

}}}

#endif // SCAG_TRANSPORT_HTTP_CHUNKED
