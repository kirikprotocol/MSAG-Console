/* $Id$ */

#ifndef __SCAG_BILL_INFRA_XMLHANDLERS_H__
#define __SCAG_BILL_INFRA_XMLHANDLERS_H__

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

class XMLBasicHandler;
class XMLBasicHandler : public HandlerBase, private XMLFormatTarget
{
	int type;
    Logger * logger;
	IntHash<uint32_t> *service_hash;
	Hash<uint32_t>	*abonent_hash;
	uint32_t cur_provider_id;
	uint32_t cur_operator_id;
	bool in_mask;
	XMLCh mask_chars[200];
	uint32_t mask_len;

    void writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter) {};
	void characters(const XMLCh *const chars, const unsigned int length);

protected:
	void ProviderMapStartElement(const char* qname, AttributeList& attrs);
	void OperatorMapStartElement(const char* qname, AttributeList& attrs);

public:
    XMLBasicHandler(const char* const encodingName, IntHash<uint32_t>* h);
    XMLBasicHandler(const char* const encodingName, Hash<uint32_t>* h);
    ~XMLBasicHandler() {};

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);

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
