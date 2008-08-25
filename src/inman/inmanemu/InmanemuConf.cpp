#include "InmanemuConf.h"
#include "XMLHandlers.h"

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/Locator.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <iostream>

namespace inmanemu { namespace util { 
/*
//static const char*  encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags  = XMLFormatter::UnRep_CharRef;
static XMLTranscoder::UnRepOpts unRepOpts = XMLTranscoder::UnRep_Throw;

static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };
*/
using namespace smsc::util;
using namespace xercesc;

class XMLInmanemuConfHandler : public XMLBasicHandler
{
    int port;
    std::string host;
    std::string cdr_dir;
public:
    std::string& getHost() {return host;}
    int getPort() {return port;}
    string& getCDRDir() {return cdr_dir;}

    XMLInmanemuConfHandler() : XMLBasicHandler() {};
    virtual void startElement(const XMLCh* const qname, AttributeList& attributes);
};

/////////////////////////////////////////////XMLInmanConfHandler/////////////////////////////



void XMLInmanemuConfHandler::startElement(const XMLCh* const qname, AttributeList& attributes)
{
    StrX XMLQName(qname);


    if (strcmp(XMLQName.localForm(), "server") != 0) return;


    unsigned int len = attributes.getLength();
    for (unsigned int index = 0; index < len; index++)
    {
        XMLCh const * XMLValue = attributes.getValue(index);
        XMLCh const * XMLName = attributes.getName(index);

        StrX attrName(XMLName);
        StrX attrValue(XMLValue);

        if (strcmp(attrName.localForm(), "host") == 0) 
        {
            if (XMLString::stringLen(XMLValue) > 0) host.append(attrValue.localForm());
        }

        if (strcmp(attrName.localForm(), "port") == 0) 
        {
            if (XMLString::stringLen(XMLValue) > 0) port = atoi(attrValue.localForm());
        }
   
        if (strcmp(attrName.localForm(), "cdr_dir") == 0) 
        {
            if (XMLString::stringLen(XMLValue) > 0) cdr_dir.append(attrValue.localForm());
        }
    }
}

//////////////////////////////////////////////InmanemuConfig///////////////////////////

void InmanemuConfig::Init()
{

    Logger * logger = Logger::getInstance("inmanemu.conf");

    smsc_log_info(logger,"Init inmanemu config");

    try
    {
        XMLPlatformUtils::Initialize("ru_RU.KOI8-R");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        throw Exception("Error during initialization XMLPlatform: %s", msg.localForm());
    }

    SAXParser parser;

    XMLInmanemuConfHandler handler;

    int errorCount = 0;
    try
    {
        //parser.setValidationScheme(SAXParser::Val_Always);
        //parser.setDoSchema(true);
        //parser.setValidationSchemaFullChecking(true);
        //parser.setDoNamespaces(true);


//        parser.setValidateAnnotations(false);   

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(&handler);
        parser.setErrorHandler(&handler);


        parser.parse("config.xml");

        host = handler.getHost();
        port = handler.getPort();

        errorCount = parser.getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        //smsc_log_error(logger,"Terminate parsing config: XMLPlatform: OutOfMemoryException");
        throw Exception("Terminate parsing config: XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        //smsc_log_error(logger,"Terminate parsing config: An error occurred. Error: %s", msg.localForm());
        throw Exception("Terminate parsing config: An error occurred. Error: %s", msg.localForm());
    }

    if (errorCount > 0) throw Exception("Terminate parsing config: Unknown error");
    smsc_log_info(logger,"Inmanemu config initialized");
}


InmanemuConfig::~InmanemuConfig()
{
    XMLPlatformUtils::Terminate();
}

}}

