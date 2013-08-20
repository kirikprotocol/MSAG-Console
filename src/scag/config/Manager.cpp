#include "Manager.h"

#include <xercesc/dom/DOM.hpp>
#include <fstream>
#include <sys/stat.h>

#include "logger/Logger.h"
#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"
#include "util/debug.h"
#include "util/xml/DOMTreeReader.h"

namespace smsc   {
namespace util   {
namespace config {

using namespace std;
using namespace xercesc;
using namespace smsc::util::xml;

std::auto_ptr<char> Manager::config_filename;
std::auto_ptr<Manager> Manager::manager;

Manager::Manager()
  throw(ConfigException)
{
  initXerces();
  findConfigFile();
  try 
  {
    __trace__("reading config...");
    DOMTreeReader reader;
    DOMDocument *document = reader.read(config_filename.get());
    if (document && document->getDocumentElement())
    {
      DOMElement *elem = document->getDocumentElement();
      __trace__("config readed");
      config.parse(*elem);
      __trace2__("parsed %u ints, %u booleans, %u strings\n",
                 config.intParams.GetCount(),
                 config.boolParams.GetCount(),
                 config.strParams.GetCount());
    } else {
      throw ConfigException("Parse result is null");
    }

  } catch (SmscParseException &e) {
    throw ConfigException(e.what());
  }
}

/**
 * Create our parser, then attach an error handler to the parser.
 * The parser will call back to methods of the ErrorHandler if it
 * discovers errors during the course of parsing the XML document.
 *
 * @return created parser
 *//*
DOMParser * Manager::createParser() {
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
*/

/*!
 * Parse the XML file, catching any XML exceptions that might propogate
 * out of it.
 */
/*
DOM_Document Manager::parse(DOMParser *parser, const char * const filename)
  throw (ConfigException)
{
  try
  {
    parser->parse(filename);
    int errorCount = parser->getErrorCount();
    if (errorCount > 0) {
      char exceptionMsg[1024];
      snprintf(exceptionMsg, sizeof(exceptionMsg), "An %d errors occured during parsing \"%s\"", errorCount, filename);
      throw ConfigException(exceptionMsg);
    }
  }
  catch (const XMLException& e)
  {
    std::auto_ptr<char> message = XMLString::transcode(e.getMessage());
    XMLExcepts::Codes code = e.getCode();
    const char *srcFile = e.getSrcFile();
    unsigned int line = e.getSrcLine();
    char exceptionMsg[1024];
    snprintf(exceptionMsg, sizeof(exceptionMsg), "An error occured during parsing \"%s\" at file \"%s\" on line %d. Nested: %d: %s", filename, srcFile, line, code, message.get());
    throw ConfigException(exceptionMsg);
  }
  catch (const DOM_DOMException& e)
  {
    char msg[1024];
    snprintf(msg, sizeof(msg), "A DOM error occured during parsing\"%s\". DOMException code: %i", filename, e.code);
    throw ConfigException(msg);
  }
  catch (...)
  {
    char msg[1024];
    snprintf(msg, sizeof(msg), "An error occured during parsing \"%s\"", filename);
    throw ConfigException(msg);
  }

  return parser->getDocument();
}


std::ostream & operator << (std::ostream & out, const XMLCh * const string)
{
  std::auto_ptr<char> p(XMLString::transcode(string));
  out << p.get();
  return out;
}
*/
/**
 * ���������� ������������ �������.
 */
void Manager::save()
{
  std::ofstream *out = new std::ofstream(config_filename.get());
  writeHeader(*out);
  config.save(*out);
  writeFooter(*out);
  out->flush();
  if (out->fail())
  {
    cerr << "Couldn't save config to \"" << config_filename.get() << "\"\n";
  }
  out->close();
  delete out;
}

void Manager::writeHeader(std::ostream &out)
{
  out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
  out << "<!DOCTYPE config SYSTEM \"configuration.dtd\">" << std::endl;
  out << "<config>" << std::endl;
}

void Manager::writeFooter(std::ostream &out)
{
  out << "</config>" << std::endl;
}

void Manager::findConfigFile()
{
  if (config_filename.get() == 0)
    return;

  struct stat s;
  std::auto_ptr<char> tmp_name(new char[strlen(config_filename.get())+10]);

  strcpy(tmp_name.get(), config_filename.get());
  if (stat(tmp_name.get(), &s) == 0)
    return;

  strcpy(tmp_name.get(), "conf/");
  strcat(tmp_name.get(), config_filename.get());
  if (stat(tmp_name.get(), &s) == 0)
  {
    config_filename = tmp_name;
    return;
  }

  strcpy(tmp_name.get(), "../conf/");
  strcat(tmp_name.get(), config_filename.get());
  if (stat(tmp_name.get(), &s) == 0)
  {
    config_filename = tmp_name;
    return;
  }
}

}
}
}
