#include "DOMTreeReader.h"

#include <logger/Logger.h>
#include <util/xml/DOMErrorLogger.h>
#include <util/xml/init.h>
#include <util/xml/DtdResolver.h>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/sax/SAXException.hpp>

namespace smsc {
namespace util {
namespace xml {

using smsc::logger::Logger;

DOMTreeReader::DOMTreeReader()
{
  initXerces();
}

DOMTreeReader::~DOMTreeReader()
{
}


DOMParser * DOMTreeReader::createParser() {
  //logger.debug("Entering createParser()");
  DOMParser *parser = new DOMParser;
  parser->setValidationScheme(DOMParser::Val_Always);
  parser->setDoNamespaces(false);
  parser->setDoSchema(false);
  parser->setValidationSchemaFullChecking(false);
  //logger.debug("  Creating ErrorReporter");
  DOMErrorLogger *errReporter = new DOMErrorLogger();
  parser->setErrorHandler(errReporter);
  parser->setCreateEntityReferenceNodes(false);
  parser->setToCreateXMLDeclTypeNode(false);
  parser->setEntityResolver(new DtdResolver());
  //logger.debug("Leaving createParser()");

  return parser;
}

DOM_Document DOMTreeReader::read(const char * const filename)
  throw (ParseException)
{
  LocalFileInputSource source(DOMString(filename).rawBuffer());
  return read(source);
}

DOM_Document DOMTreeReader::read(const InputSource & source)
  throw (ParseException)
{
  try
  {
    std::auto_ptr<DOMParser> parser(createParser());
    parser->parse(source);
    int errorCount = parser->getErrorCount();
    if (errorCount > 0) {
      smsc::logger::Logger logger(Logger::getInstance("smsc.util.xml.DOMTreeReader"));
      logger.error("An %d errors occured during parsing received command", errorCount);
      throw ParseException("An errors occured during parsing");
    }
    return parser->getDocument();
  }
  catch (const XMLException& e)
  {
    std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
    XMLExcepts::Codes code = e.getCode();
    const char *srcFile = e.getSrcFile();
    unsigned int line = e.getSrcLine();
    smsc::logger::Logger logger(Logger::getInstance("smsc.util.xml.DOMTreeReader"));
    logger.error("An error occured during parsing received (\"%s\") command on line %d. Nested: %d: %s", srcFile, line, code, message.get());
    throw ParseException(message.get());
  }
  catch (const DOM_DOMException& e)
  {
    smsc::logger::Logger logger(Logger::getInstance("smsc.util.xml.DOMTreeReader"));
    logger.error("A DOM error occured during parsing received command. DOMException code: %i", e.code);
    throw ParseException("An errors occured during parsing");
  }
  catch (const SAXException &e)
  {
    std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
    smsc::logger::Logger logger(Logger::getInstance("smsc.util.xml.DOMTreeReader"));
    logger.error("A DOM error occured during parsing received command, nested: %s", message.get());
    throw ParseException(message.get());
  }
  catch (...)
  {
    smsc::logger::Logger logger(Logger::getInstance("smsc.util.xml.DOMTreeReader"));
    logger.error("An error occured during parsing  received command");
    throw ParseException("An errors occured during parsing");
  }
}

}
}
}
