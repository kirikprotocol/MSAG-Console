/* $Id$ */

#ifndef __SCAG_BILL_INFRA_XMLHANDLERS_H__
#define __SCAG_BILL_INFRA_XMLHANDLERS_H__

#include <xercesc/sax/HandlerBase.hpp>

#include <util/Exception.hpp>

#include <core/buffers/IntHash.hpp>
#include <logger/Logger.h>

namespace scag { namespace bill { namespace infrastruct {

using namespace smsc::util;
using namespace smsc::logger;
using smsc::core::buffers::Hash;

XERCES_CPP_NAMESPACE_USE

class XMLBasicHandler : public HandlerBase
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

	void characters(const XMLCh *const chars, const unsigned int length);

protected:
	void ProviderMapStartElement(const char* qname, AttributeList& attrs);
	void OperatorMapStartElement(const char* qname, AttributeList& attrs);

public:
    XMLBasicHandler(IntHash<uint32_t>* h);
    XMLBasicHandler(Hash<uint32_t>* h);
    ~XMLBasicHandler() {};

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);

    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
};

/*class XMLTariffMatrixHandler : public HandlerBase
{
    Logger * logger;
	Hash<uint32_t>	*category_hash;
	Hash<uint32_t>	*media_type_hash;
	uint32_t category_level, media_type_level;
	bool in_category_id, in_category_name;
	bool in_media_type_id, in_media_type_name;
	uint32_t category_id;
	uint32_t media_type_id;
	std::string media_type_name, category_name;

	void characters(const XMLCh *const chars, const unsigned int length);

protected:

public:
    XMLTariffMatrixHandler(Hash<uint32_t>*, Hash<uint32_t>*);
    ~XMLTariffMatrixHandler() {};

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);

    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
};*/

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
    char* fLocalForm;
};

}}}

#endif
