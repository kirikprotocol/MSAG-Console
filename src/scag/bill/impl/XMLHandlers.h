/* $Id$ */

#ifndef __SCAG_BILL_IMPL_INFRA_XMLHANDLERS_H__
#define __SCAG_BILL_IMPL_INFRA_XMLHANDLERS_H__

#include <xercesc/sax/HandlerBase.hpp>

#include <util/Exception.hpp>

#include <core/buffers/IntHash.hpp>
#include <logger/Logger.h>
#include "scag/bill/base/Infrastructure.h"

namespace scag2 {
namespace bill {
namespace infrastruct {

using namespace smsc::util;
using namespace smsc::logger;
using smsc::core::buffers::Hash;
using smsc::core::buffers::IntHash;

XERCES_CPP_NAMESPACE_USE

class XMLBasicHandler : public HandlerBase
{
    int type;
    Logger * logger;
    IntHash<uint32_t> *service_hash;
    Hash<uint32_t>  *abonent_hash;
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

class XMLTariffMatrixHandler : public HandlerBase
{
    Logger * logger;
    IntHash<uint32_t>  *category_hash;
    IntHash<uint32_t>  *media_type_hash;
    Hash<uint32_t>  *category_str_hash;
    Hash<uint32_t>  *media_type_str_hash;
    IntHash<TariffRec>  *tariff_hash;
    uint32_t category_idx, media_type_idx;
    uint32_t category_id, media_type_id;
    std::string media_type_name, category_name;
    XMLCh chars[1024];
    uint8_t media_type_tag, category_tag, bill_tag;
    uint32_t bill_category_id, bill_media_type_id, bill_operator_id, bill_type;
    std::string bill_price;
    std::string bill_currency, bill_service_number;

    void characters(const XMLCh *const chrs, const unsigned int length);

protected:

public:
    XMLTariffMatrixHandler(IntHash<uint32_t>*, IntHash<uint32_t>*, Hash<uint32_t>*, Hash<uint32_t>*, IntHash<TariffRec>*);
    ~XMLTariffMatrixHandler() {};

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
    char* fLocalForm;
};

}}}

#endif
