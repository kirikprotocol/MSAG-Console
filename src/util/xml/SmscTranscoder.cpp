#include <util/xml/SmscTranscoder.h>
#include <util/xml/utilFunctions.h>
#include <util/Exception.hpp>
#include <errno.h>

namespace smsc {
namespace util {
namespace xml {

using namespace smsc::util;

///////////////////////////////////////////////////////////////////////////////
/// SmscTranscoder
///////////////////////////////////////////////////////////////////////////////
SmscTranscoder::SmscTranscoder(const XMLCh *const encodingName, const unsigned int blockSize, MemoryManager *const manager)
  :XMLTranscoder(encodingName, blockSize, manager)
{
  std::auto_ptr<const char> enc(XMLString::transcode(encodingName));
  const char * const ucs2("UCS-2LE");
  iconvHandlerFrom = iconv_open(ucs2, enc.get());
  if (iconvHandlerFrom == (iconv_t)-1)
    throw Exception("Could not open iconv for transcode from \"%s\" to \"%s\", errno:%d", encodingName, ucs2, errno);
  iconvHandlerTo = iconv_open(enc.get(), ucs2);
  if (iconvHandlerTo == (iconv_t)-1)
    throw Exception("Could not open iconv for transcode from \"%s\" to \"%s\", errno:%d", ucs2, encodingName, errno);
}

SmscTranscoder::~SmscTranscoder()
{
  iconv_close(iconvHandlerFrom);
  iconv_close(iconvHandlerTo);
}

unsigned int SmscTranscoder::transcodeFrom(const XMLByte *const srcData,
                           const unsigned int srcCount,
                           XMLCh *const toFill,
                           const unsigned int maxChars,
                           unsigned int &bytesEaten,
                           unsigned char *const charSizes)
{
  size_t inbytesleft = srcCount;
  size_t outbytesleft = maxChars*sizeof(XMLCh);
  
  const char *inbuf = (const char*)srcData;
  char * outbuf = (char*)toFill;
  unsigned char * charSizesPtr = (unsigned char *)charSizes;
  size_t lastInLeft = inbytesleft;
  while (inbytesleft > 0 && outbytesleft > 0) {
    size_t tmp = sizeof(XMLCh);
    size_t ret = iconv(iconvHandlerFrom, &inbuf, &inbytesleft, &outbuf, &tmp);
    outbytesleft -= (sizeof(XMLCh)-tmp);
    if (ret != (size_t)-1 || errno == E2BIG) {
      *(charSizesPtr++)= lastInLeft - inbytesleft;
      lastInLeft = inbytesleft;
    } else {
      //error
      break;
    }
  }
  if (((XMLCh*)outbuf) < toFill + maxChars)
    *(XMLCh*)outbuf = 0;
  bytesEaten = srcCount - inbytesleft;
  size_t result = maxChars - outbytesleft/sizeof(XMLCh);
  return result;
}
 
unsigned int SmscTranscoder::transcodeTo(const XMLCh *const srcData,
                         const unsigned int srcCount,
                         XMLByte *const toFill, 
                         const unsigned int maxBytes, 
                         unsigned int &charsEaten, 
                         const UnRepOpts options)
{
  size_t inbytesleft = srcCount*sizeof(XMLCh);
  size_t outbytesleft = maxBytes;
  const char *inbuf = (const char * const)srcData;
  char * outbuf = (char *)toFill;
  size_t ret = iconv(iconvHandlerTo, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  if (ret != (size_t)-1) {
    if ((XMLByte*)outbuf < toFill + maxBytes)
      *outbuf = 0;
    charsEaten = srcCount - inbytesleft/sizeof(XMLCh);
    return maxBytes - outbytesleft;
  }
  else
    return 0;
}
 
bool SmscTranscoder::canTranscodeTo(const unsigned int toCheck) const
{
  const XMLCh inbuf[1] = {toCheck};
  size_t inbytesleft = sizeof(XMLCh);
  char outbuf[4];
  size_t outbytesleft = sizeof(outbuf);
  size_t ret = iconv(iconvHandlerTo, (const char**)&inbuf, &inbytesleft, (char**)&outbuf, &outbytesleft);
  return ret != (size_t)-1;
}

}
}
}
