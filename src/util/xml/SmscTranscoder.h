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
#define XTYPE XMLSize_t
#else
#define XTYPE unsigned int
#endif

  virtual XTYPE transcodeFrom(const XMLByte *const srcData,
                                       const XTYPE srcCount,
                                       XMLCh *const toFill,
                                       const XTYPE maxChars,
                                       XTYPE &bytesEaten,
                                       unsigned char *const charSizes);

  virtual XTYPE transcodeTo(const XMLCh *const srcData,
                                   const XTYPE srcCount,
                                   XMLByte *const toFill, 
                                   const XTYPE maxBytes,
                                   XTYPE &charsEaten,
                                   const UnRepOpts options);
 
  virtual bool canTranscodeTo(const unsigned int toCheck);

//#undef XTYPE

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
