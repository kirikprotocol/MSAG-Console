/* 
 * "@(#)$Id$"
 */
#include <iostream>
#include <xercesc/sax/SAXException.hpp>

#include "util/xml/DOMTreeReader.h"
#include "util/xml/init.h"
#include "util/xml/DtdResolver.h"

namespace smsc {
namespace util {
namespace xml {

//using smsc::logger::Logger;

DOMTreeReader::DOMTreeReader(unsigned throw_lvl/* = (unsigned)-1*/)
  : _throwLvl(throw_lvl)
{
  initXerces();
  parser.reset(createParser(_throwLvl));
}

DOMTreeReader::~DOMTreeReader()
{
  delete parser->getErrorHandler();
  delete parser->getEntityResolver();
}


DOMBuilder * DOMTreeReader::createParser(unsigned throw_lvl/* = (unsigned)-1*/) {
  XMLCh tempStr[100];
  XMLString::transcode("LS", tempStr, 99);
  DOMBuilder *parser = ((DOMImplementationLS*)DOMImplementationRegistry::getDOMImplementation(tempStr))->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
  
  if (parser->canSetFeature(XMLUni::fgDOMValidation, true))
    parser->setFeature(XMLUni::fgDOMValidation, true);
  if (parser->canSetFeature(XMLUni::fgDOMNamespaces, false))
    parser->setFeature(XMLUni::fgDOMNamespaces, false);

	parser->setErrorHandler(new DOMErrorLogger(throw_lvl));
	parser->setEntityResolver(new DtdResolver()); 

	return parser;
}


DOMDocument * DOMTreeReader::read(const char * const filename)
  throw (ParseException)
{
  try {
    Wrapper4InputSource source(new LocalFileInputSource(XmlStr(filename)));
    return read(source);
  }
  catch (const ParseException & pexc) {
    throw;
  }
  catch (smsc::util::Exception &e)
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
  catch (const ParseException & pexc) {
    throw;
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
