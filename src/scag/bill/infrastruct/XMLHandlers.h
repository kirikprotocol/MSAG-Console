/* $Id$ */

#ifndef __SCAG_ENV_XMLHANDLERS_H__
#define __SCAG_ENV_XMLHANDLERS_H__

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <util/Exception.hpp>

#include <core/buffers/Array.hpp>
#include <core/buffers/IntHash.hpp>

#include <logger/Logger.h>

namespace scag { namespace bill { namespace infrastruct {

using namespace smsc::util;
using namespace smsc::logger;
using smsc::core::buffers::Hash;

XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

class XMLBasicHandler;
class XMLBasicHandler : public HandlerBase, private XMLFormatTarget
{
    Logger * logger;
    XMLFormatter  fFormatter;
	IntHash<uint32_t> *hash;

    void writeChars(const XMLByte* const toWrite);
    void writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter);
    void characters(const XMLCh* const chars, const unsigned int length);

protected:
public:
    XMLBasicHandler(const char* const encodingName, IntHash<uint32_t>* hash);
    ~XMLBasicHandler();

    void startDocument();
    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);
    void endDocument();

    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
};

class StrX
{
public :
    StrX(const XMLCh* const toTranscode)
    {
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }

    const char* localForm() const
    {
        return fLocalForm;
    }

private :
    char*   fLocalForm;
};

}}}

#endif

