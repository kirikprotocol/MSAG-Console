#ifndef __INMANEMU_XMLHANDLERS_H__
#define __INMANEMU_XMLHANDLERS_H__

#include <string>
#include <stack>


#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <util/Exception.hpp>


#include <core/buffers/Array.hpp>
#include <acls/interfaces.h>
#include <logger/Logger.h>

#include "IBillParserHandler.h"

namespace inmanemu { namespace util { 

using namespace smsc::util;
using smsc::core::buffers::Hash;
using smsc::logger::Logger;


XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END


class XMLBasicHandler : public HandlerBase, private XMLFormatTarget
{
    const Locator * m_pLocator;
    Logger * logger;
    //XMLFormatter  fFormatter;
    IBillParserHandler& parent;

    void writeChars(const XMLByte* const toWrite);
    void writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter);
    void characters(const XMLCh* const chars, const unsigned int length);

protected:
public:
    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    XMLBasicHandler(IBillParserHandler& billParserHandler);
    ~XMLBasicHandler();

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);
    virtual void setDocumentLocator(const Locator * const locator);



    // -----------------------------------------------------------------------
    //  Implementations of the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
    void endDocument();
};
/*
class XMLBasicTranscoder : public XMLTranscoder
{
public:

};  */

class StrX
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
        return fLocalForm;
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   fLocalForm;
};
/*
inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump)
{
    //target << toDump.localForm();
    return target;
}        */


}}

#endif

