#include "XMLHandlers.h"

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/Locator.hpp>
#include <xercesc/util/TransService.hpp>

#include <iostream>

namespace inmanemu { namespace util { 
/*
//static const char*  encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags  = XMLFormatter::UnRep_CharRef;
static XMLTranscoder::UnRepOpts unRepOpts = XMLTranscoder::UnRep_Throw;

static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };
*/
using namespace smsc::util;


/////////////////////////////////////////////XMLBasicHandler/////////////////////////////

XMLBasicHandler::XMLBasicHandler(IBillParserHandler * billParserHandler) : 
    logger(0), parent(billParserHandler)

{
    m_hasHandler = true;
    logger = Logger::getInstance("inmanemu");
}

XMLBasicHandler::~XMLBasicHandler()
{
}


void XMLBasicHandler::writeChars(const XMLByte* const toWrite)
{
}

void XMLBasicHandler::writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter)
{
//    XERCES_STD_QUALIFIER cout.write((char *) toWrite, (int) count);
//    XERCES_STD_QUALIFIER cout.flush();
}


void XMLBasicHandler::characters(const XMLCh* const chars, const unsigned int length)
{
    //fFormatter.formatBuf(chars, length, XMLFormatter::CharEscapes);
}


void XMLBasicHandler::startElement(const XMLCh* const qname, AttributeList& attributes)
{
    StrX XMLQName(qname);

    Address abonentAddr;
    Address nullAddr;
    int money = 0;

    MatrixKey key;
    key.serviceNumber = 0;
    int price = 0;

    bool setAccount = false;
    bool setBillRecord = false;


    if (strcmp(XMLQName.localForm(), "account") == 0) setAccount = true;
    if (strcmp(XMLQName.localForm(), "bill") == 0) setBillRecord = true;

    if ((!setAccount)&&(!setBillRecord)) return;

    unsigned int len = attributes.getLength();
    for (unsigned int index = 0; index < len; index++)
    {
        XMLCh const * XMLValue = attributes.getValue(index);
        XMLCh const * XMLName = attributes.getName(index);

        StrX attrName(XMLName);
        StrX attrValue(XMLValue);

        if (setAccount) 
        {
            if (strcmp(attrName.localForm(), "abonent") == 0) 
            {
//                  std::cout << "  ~~~~" << XMLString::stringLen(XMLValue) << std::endl;  
                if (XMLString::stringLen(XMLValue) > 0) 
                    abonentAddr.setValue(strlen(attrValue.localForm()),attrValue.localForm());
                else
                    abonentAddr = nullAddr;
            }
            if (strcmp(attrName.localForm(), "money") == 0) money = atoi(attrValue.localForm());
        }

        if (setBillRecord) 
        {
            if (XMLString::stringLen(XMLValue) > 0) 
            {
                if (strcmp(attrName.localForm(), "service_number") == 0) key.serviceNumber = atoi(attrValue.localForm());
                if (strcmp(attrName.localForm(), "price") == 0) price = atoi(attrValue.localForm());
            }
        }

    }

    /*
    virtual void RegisterAccount(Address& abonentAddr, int money) = 0;
    virtual void RegisterBillRecord(MattrixKey& key, int price) = 0;
    
    */

    if (!parent) return;

    if (setBillRecord) parent->RegisterBillRecord(key, price);
    if (setAccount) parent->RegisterAccount(abonentAddr, money);

}


void XMLBasicHandler::endElement(const XMLCh* const qname)
{
}

void XMLBasicHandler::endDocument()
{
}

void XMLBasicHandler::setDocumentLocator(const Locator * const locator)
{
    m_pLocator = locator;
}


// ---------------------------------------------------------------------------
//  SAX2PrintHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void XMLBasicHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    //smsc_log_error(logger,"Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
    throw Exception("Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLBasicHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    //smsc_log_error(logger,"Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
    throw Exception("Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());

}

void XMLBasicHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

}}

