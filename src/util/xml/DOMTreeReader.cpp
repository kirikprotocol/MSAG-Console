#include "DOMTreeReader.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/SAXException.hpp>

#include <logger/Logger.h>
#include <util/xml/DOMErrorLogger.h>
#include <util/xml/init.h>
#include <util/xml/DtdResolver.h>

namespace smsc {
namespace util {
namespace xml {

using smsc::logger::Logger;

DOMTreeReader::DOMTreeReader()
  :parser(0)
{
  initXerces();
  parser.reset(createParser());
}

DOMTreeReader::~DOMTreeReader()
{
  delete parser->getErrorHandler();
  delete parser->getEntityResolver();
}


DOMBuilder * DOMTreeReader::createParser() {
  XMLCh tempStr[100];
  XMLString::transcode("LS", tempStr, 99);
  DOMBuilder *parser = ((DOMImplementationLS*)DOMImplementationRegistry::getDOMImplementation(tempStr))->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
  
  if (parser->canSetFeature(XMLUni::fgDOMValidation, true))
    parser->setFeature(XMLUni::fgDOMValidation, true);
  if (parser->canSetFeature(XMLUni::fgDOMNamespaces, false))
    parser->setFeature(XMLUni::fgDOMNamespaces, false);

	parser->setErrorHandler(new DOMErrorLogger());
	parser->setEntityResolver(new DtdResolver()); 

	return parser;
}


DOMDocument * DOMTreeReader::read(const char * const filename)
  throw (ParseException)
{
  try {
    Wrapper4InputSource source(new LocalFileInputSource(XmlStr(filename)));
    return read(source);
  } catch (Exception &e)
  {
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An error occured during parsing file (\"%s\"): %s", filename, e.what());
    throw ParseException(e.what());
  }
}

DOMDocument * DOMTreeReader::read(const DOMInputSource & source)
  throw (ParseException)
{
  MutexGuard guard(mutex);
  
  try
  {
    DOMDocument * result = parser->parse(source);
    if (result == NULL) {
      smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An errors occured during parsing");
      throw ParseException("An errors occured during parsing");
    }
    return result;
  }
  catch (const XMLException& e)
  {
    std::auto_ptr<char> message(XMLString::transcode(e.getMessage()));
    XMLExcepts::Codes code = e.getCode();
    const char *srcFile = e.getSrcFile();
    unsigned int line = e.getSrcLine();
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An error occured during parsing received (\"%s\") command on line %d. Nested: %d: %s", srcFile, line, code, message.get());
    throw ParseException(message.get());
  }
  catch (const DOMException& e)
  {
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "A DOM error occured during parsing. DOMException code: %i", e.code);
    throw ParseException("An errors occured during parsing");
  }
  catch (const SAXException &e)
  {
    std::auto_ptr<char> message(XMLString::transcode(e.getMessage()));
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "A SAX error occured during parsing, nested: %s", message.get());
    throw ParseException(message.get());
  }
  catch (...)
  {
    smsc_log_error(Logger::getInstance("smsc.util.xml.DOMTreeReader"), "An error occured during parsing");
    throw ParseException("An errors occured during parsing");
  }
}

}
}
}
