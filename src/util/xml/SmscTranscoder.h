#ifndef SMSC_UTIL_XML_SMSC_TRANSCODER_HEADER
#define SMSC_UTIL_XML_SMSC_TRANSCODER_HEADER

#define LIBICONV_PLUG EMPTY

#include <iconv.h>
#include <xercesc/util/TransService.hpp>

namespace smsc {
namespace util {
namespace xml {

using namespace xercesc;

class SmscTranscoder : public XMLTranscoder
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  SmscTranscoder(const XMLCh *const encodingName, const unsigned int blockSize, MemoryManager *const manager = XMLPlatformUtils::fgMemoryManager);
  virtual ~SmscTranscoder();


  // -----------------------------------------------------------------------
  //  Implementation of the virtual transcoder interface
  // -----------------------------------------------------------------------
  virtual unsigned int transcodeFrom(const XMLByte *const srcData, 
                                     const unsigned int srcCount, 
                                     XMLCh *const toFill, 
                                     const unsigned int maxChars, 
                                     unsigned int &bytesEaten, 
                                     unsigned char *const charSizes);
 
  virtual unsigned int transcodeTo(const XMLCh *const srcData, 
                                   const unsigned int srcCount, 
                                   XMLByte *const toFill, 
                                   const unsigned int maxBytes, 
                                   unsigned int &charsEaten, 
                                   const UnRepOpts options);
 
  virtual bool canTranscodeTo(const unsigned int toCheck) const;


private:
  // -----------------------------------------------------------------------
  //  Unimplemented constructors and operators
  // -----------------------------------------------------------------------
  SmscTranscoder(const SmscTranscoder&);
  SmscTranscoder& operator=(const SmscTranscoder&);
  iconv_t iconvHandlerFrom;
  iconv_t iconvHandlerTo;
};

}
}
}

#endif //SMSC_UTIL_XML_SMSC_TRANSCODER_HEADER
