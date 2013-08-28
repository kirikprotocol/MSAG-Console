#ifndef SMSC_UTIL_XML_SMSC_TRANSCODER_HEADER
#define SMSC_UTIL_XML_SMSC_TRANSCODER_HEADER

#define LIBICONV_PLUG EMPTY

#include <iconv.h>
#include <xercesc/util/XercesVersion.hpp>
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

#if XERCES_VERSION_MAJOR > 2
  virtual XMLSize_t transcodeFrom(const XMLByte* const srcData,
                                       const XMLSize_t srcCount,
                                       XMLCh* const toFill,
                                       const XMLSize_t maxChars,
                                       XMLSize_t& bytesEaten,
                                       unsigned char* const charSizes);

  virtual XMLSize_t transcodeTo(const XMLCh* const srcData,
                                   const XMLSize_t srcCount,
                                   XMLByte* const toFill,
                                   const XMLSize_t maxBytes,
                                   XMLSize_t& charsEaten,
                                   const UnRepOpts options);

  virtual bool canTranscodeTo(const unsigned int toCheck);
#else
  virtual unsigned int transcodeFrom(const XMLByte* const srcData,
                                       const unsigned int srcCount,
                                       XMLCh* const toFill,
                                       const unsigned int maxChars,
                                       unsigned int& bytesEaten,
                                       unsigned char* const charSizes);

  virtual XMLSize_t transcodeTo(const XMLCh* const srcData,
                                   const XMLSize_t srcCount,
                                   XMLByte* const toFill,
                                   const XMLSize_t maxBytes,
                                   XMLSize_t& charsEaten,
                                   const UnRepOpts options);
 
  virtual bool canTranscodeTo(const unsigned int toCheck) const;
#endif


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
