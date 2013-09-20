/* 
 * "@(#)$Id$"
 */
#include <iostream>

#include "util/xml/DOMTreeReader.h"
#include "util/xml/init.h"
#include "util/xml/DtdResolver.h"
#include "util/Exception.hpp"
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>

namespace smsc {
namespace util {
namespace xml {

//using smsc::logger::Logger;
XERCES_CPP_NAMESPACE_USE

DOMTreeReader::DOMTreeReader(unsigned throw_lvl/* = (unsigned)-1*/)
  : _throwLvl(throw_lvl)
{
  initXerces();
  parser.reset(createParser(_throwLvl));
}

DOMTreeReader::~DOMTreeReader()
{
  delete parser->getErrorHandler();
#if XERCES_VERSION_MAJOR > 2
  delete parser->getXMLEntityResolver();
#else
  delete parser->getEntityResolver();
#endif
}


#if XERCES_VERSION_MAJOR > 2
  XercesDOMParser* DOMTreeReader::createParser(unsigned throw_lvl/* = (unsigned)-1*/)
#else
  DOMBuilder* DOMTreeReader::createParser(unsigned throw_lvl/* = (unsigned)-1*/)
#endif
{
  XMLCh tempStr[100];
  XMLString::transcode("LS", tempStr, 99);
  
#if XERCES_VERSION_MAJOR > 2
  XercesDOMParser* parser = new XercesDOMParser();
  parser->setErrorHandler(new DOMErrorLogger(throw_lvl));
  parser->setXMLEntityResolver(new DtdResolver());
#else
  DOMBuilder* parser = ((DOMImplementationLS*)DOMImplementationRegistry::getDOMImplementation(tempStr))->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

  if (parser->canSetFeature(XMLUni::fgDOMValidation, true))
    parser->setFeature(XMLUni::fgDOMValidation, true);
  if (parser->canSetFeature(XMLUni::fgDOMNamespaces, false))
    parser->setFeature(XMLUni::fgDOMNamespaces, false);
  parser->setErrorHandler(new DOMErrorLogger(throw_lvl));
  parser->setEntityResolver(new DtdResolver());
#endif

  return parser;
}

DOMDocument* DOMTreeReader::read(const char* const filename) throw (SmscParseException)
{
  try
  {
#if XERCES_VERSION_MAJOR > 2
    InputSource& source = *(new LocalFileInputSource(XmlStr(filename)));
#else
    Wrapper4InputSource source(new LocalFileInputSource(XmlStr(filename)));
#endif
    return read(source);
  }
  catch (const SmscParseException & pexc)
  {
    throw;
  }
  catch (smsc::util::Exception & e)
  {
    const char* w = e.what();
    SmscParseException pe("An error occured during parsing file (\"%s\"): %s", filename, w);
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), pe.what());
//    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An error occured during parsing file (\"%s\"): %s", filename, e.what());
    throw pe;
  }
  catch(...)
  { throw; }
  return 0;
}


#if XERCES_VERSION_MAJOR > 2
DOMDocument* DOMTreeReader::read(const InputSource& source) throw (SmscParseException)
#else
DOMDocument* DOMTreeReader::read(const DOMInputSource& source) throw (SmscParseException)
#endif
{
  MutexGuard guard(mutex);

  try
  {
#if XERCES_VERSION_MAJOR > 2
    parser->parse(source);
    DOMDocument* result = parser->getDocument();
#else
    DOMDocument* result = parser->parse(source);
#endif
    if (result == NULL)
    {
      smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An errors occured during parsing");
      throw SmscParseException("An errors occured during parsing");
    }
    return result;
  }
  catch (const XMLException& exx)
  {
    const XMLCh* m = exx.getMessage();
    std::auto_ptr<char> message(XMLString::transcode(exx.getMessage()));
    XMLExcepts::Codes code = exx.getCode();
    const char *srcFile = exx.getSrcFile();
    XMLSize_t line = exx.getSrcLine();
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An error occured during parsing received (\"%s\") command on line %u. Nested: %d: %s", srcFile, line, code, message.get());
    throw SmscParseException(message.get());
  }
  catch (const DOMException& e)
  {
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "A DOM error occured during parsing. DOMException code: %i", e.code);
    throw SmscParseException("An errors occured during parsing");
  }
  catch (const SAXException &e)
  {
    std::auto_ptr<char> message(XMLString::transcode(e.getMessage()));
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "A SAX error occured during parsing, nested: %s", message.get());
    throw SmscParseException(message.get());
  }
  catch (const SmscParseException & pexc) {
    throw;
  }
  catch (...)
  {
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An error occured during parsing");
    throw SmscParseException("An errors occured during parsing");
  }
  return 0;
}

}
}
}
