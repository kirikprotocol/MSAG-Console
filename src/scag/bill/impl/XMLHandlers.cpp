/* $Id$ */

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include "XMLHandlers.h"
#include <locale.h>
#include "sms/sms.h"

namespace scag2 {
namespace bill {
namespace infrastruct {

using namespace smsc::util;
using namespace smsc::sms;

typedef struct{
    const char *name;
    uint8_t top_tag;
} tag_t;

static tag_t category_tags[] = {
{"categories", -1},
{"category", 0},
{"name", 1},
};
#define CATEGORY_TAGS_SZ sizeof(category_tags) / sizeof(tag_t)

static tag_t media_type_tags[] = {
{"media_types", -1},
{"media_type", 0},
{"name", 1},
};

#define MEDIA_TYPE_TAGS_SZ sizeof(media_type_tags) / sizeof(tag_t)

static tag_t bill_tags[] = {
{"billing_info", -1},
{"operator", 0},
{"billing", 1},
{"category_id", 2},
{"media_type_id", 2},
{"service_number", 2},
{"price", 2},
{"currency", 2},
};
#define BILL_TAGS_SZ sizeof(bill_tags) / sizeof(tag_t)


XMLBasicHandler::XMLBasicHandler(IntHash<uint32_t> *h)
{
    logger = Logger::getInstance("xmlhndlr");
    cur_provider_id = 0;
    service_hash = h;
    in_mask = false;
    mask_len = 0;
    type = 0;
}

XMLBasicHandler::XMLBasicHandler(Hash<uint32_t> *h)
{
    logger = Logger::getInstance("xmlhndlr");
    cur_provider_id = 0;
    cur_operator_id = 0;
    abonent_hash = h;
    in_mask = false;
    mask_len = 0;
    type = 1;
}

void XMLBasicHandler::characters(const XMLCh *const chars, const unsigned int length) 
{
    uint32_t cnt = length;
    if(type == 1 && in_mask)
    {
        if(mask_len + cnt >= 200)
            cnt = 200 - mask_len - 1;
        if(cnt)
        {
            memcpy(mask_chars + mask_len, chars, cnt * sizeof(XMLCh));
            mask_len += cnt;
        }
    }
}

void XMLBasicHandler::ProviderMapStartElement(const char* qname, AttributeList& attrs)
{
    if(!strcmp(qname, "provider"))
    {
        StrX s = attrs.getValue("id");
        if(!(cur_provider_id = atoi(s.localForm()) ))
            throw Exception("Invalid provider id");
    } else if(cur_provider_id && !strcmp(qname, "service"))
    {
        StrX s = attrs.getValue("id");
        uint32_t id = atoi(s.localForm());
        if(!id)
            throw Exception("Invalid service id");
        service_hash->Insert(id, cur_provider_id);
    }
}

void XMLBasicHandler::OperatorMapStartElement(const char* qname, AttributeList& attrs)
{
    if(!strcmp(qname, "operator"))
    {
        StrX s = attrs.getValue("id");
        if(!(cur_operator_id = atoi(s.localForm()) ))
            throw Exception("Invalid operator id");
    }
    else if(cur_operator_id && !strcmp(qname, "mask"))
    {
        mask_len = 0;
        in_mask = true;
    }
}

void XMLBasicHandler::startElement(const XMLCh* const qname, AttributeList& attrs)
{
    StrX XMLQName(qname);

    if(!type)
        ProviderMapStartElement(XMLQName.localForm(), attrs);
    else if(type == 1)
        OperatorMapStartElement(XMLQName.localForm(), attrs);
}

void XMLBasicHandler::endElement(const XMLCh* const qname)
{
    StrX XMLQName(qname);
    if(type== 0 && !strcmp(XMLQName.localForm(), "provider"))
        cur_provider_id = 0;
    else if(type == 1 && !strcmp(XMLQName.localForm(), "operator"))
        cur_operator_id = 0;
    else if(type == 1 && cur_operator_id && !strcmp(XMLQName.localForm(), "mask"))
    {
        if(mask_len > 0)
        {
            mask_chars[mask_len] = 0;
            StrX s(mask_chars);
            Address addr(s.localForm());
            abonent_hash->Insert(addr.toString().c_str(), cur_operator_id);
            smsc_log_debug(logger, "Added: operator_id=%d, mask=%s", cur_operator_id, addr.toString().c_str());
        }
        in_mask = false;
    }
}

void XMLBasicHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    throw Exception("Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLBasicHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    throw Exception("Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLBasicHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

//------------------------------------------------------------------------------
XMLTariffMatrixHandler::XMLTariffMatrixHandler(IntHash<uint32_t> *cat, IntHash<uint32_t> *mt, Hash<uint32_t> *cat_str, Hash<uint32_t> *mt_str, IntHash<TariffRec> *th)
{
    logger = Logger::getInstance("xmlhndlr");
    category_hash = cat;
    media_type_hash = mt;
    category_str_hash = cat_str;
    media_type_str_hash = mt_str;
    tariff_hash = th;
    bill_tag = -1;
    media_type_tag = -1;
    category_tag = -1;

    category_idx = 0;
    media_type_idx = 0;
    category_id = 0;
    media_type_id = 0;

    bill_category_id = 0;
    bill_media_type_id = 0;
    bill_operator_id = 0;
    bill_type = 0;
}

//------------------------------------------------------------------------------
static double str_to_double(char* str)
{
    double d = 0, df = 1, f = 0;
    uint8_t j = '+', ch, adp = 0;
    if(*str && (*str == '+' || *str == '-'))
        j = *str++;
    while( (ch = *str++) )
    {
        if(ch == '.' && !adp)
            adp = true;
        else if(ch < 0x30 || ch > 0x39)
            break;
        else if(adp)
        {
            df /= 10;
            f += df * (ch - 0x30);
        }
        else
        {
            d *= 10;
            d += ch - 0x30;
        }
    }
    return (j == '-') ? -(d + f) : (d + f);
}

void XMLTariffMatrixHandler::characters(const XMLCh *const chrs, const unsigned int length) 
{
    uint32_t cnt = length;

    if(cnt >= 1024) cnt = 1023;
    memcpy(chars, chrs, cnt * sizeof(XMLCh));
    chars[cnt] = 0;
    StrX q(chars);
    char *str = (char*)q.localForm();

    if(media_type_tag == 2)
        media_type_name = str;
    else if(category_tag == 2)
        category_name = str;
    else if(bill_tag == 3)
        bill_category_id = atoi(str);
    else if(bill_tag == 4)
        bill_media_type_id = atoi(str);
    else if(bill_tag == 5)
        bill_service_number = str;
    else if(bill_tag == 6)
//        bill_price = atof(str);
        //bill_price = str_to_double(str);        
        bill_price = str;
    else if(bill_tag == 7)
        bill_currency = str;
}

void XMLTariffMatrixHandler::startElement(const XMLCh* const nm, AttributeList& attrs)
{
    StrX XMLQName(nm);
    const char *qname = XMLQName.localForm();

    for( size_t i = 0; i < CATEGORY_TAGS_SZ; i++)
        if(!strcmp(category_tags[i].name, qname) && category_tag == category_tags[i].top_tag)
        {
            category_tag = i;
            if(i == 1)
            {
                StrX s = attrs.getValue("id");
                if (s.localForm() == 0) throw Exception("Error parsing Tarif Matrix - attribute 'id' not found in '%s' section",qname);

                category_id = atoi(s.localForm());
            }
            return;
        }

    for( size_t i = 0; i < MEDIA_TYPE_TAGS_SZ; i++)
        if(!strcmp(media_type_tags[i].name, qname) && media_type_tag == media_type_tags[i].top_tag)
        {
            media_type_tag = i;
            if(i == 1)
            {
                StrX s = attrs.getValue("id");
                if (s.localForm() == 0) throw Exception("Error parsing Tarif Matrix - attribute 'id' not found in '%s' section",qname);
                media_type_id = atoi(s.localForm());
            }
            return;
        }

    for( size_t i = 0; i < BILL_TAGS_SZ; i++)
        if(!strcmp(bill_tags[i].name, qname) && bill_tag == bill_tags[i].top_tag)
        {
            bill_tag = i;
            if(i == 1)
            {
                StrX s = attrs.getValue("id");
                if (s.localForm() == 0) throw Exception("Error parsing Tarif Matrix - attribute 'id' not found in '%s' section",qname);
                bill_operator_id = atoi(s.localForm());
            }
            else if(i == 2)
            {
                StrX s = attrs.getValue("type");
                if (s.localForm() != 0)
                {
                    if(!strcmp(s.localForm(), "inman"))
                        bill_type = INMAN;
                    else if(!strcmp(s.localForm(), "none"))
                        bill_type = NONE;
                    else if(!strcmp(s.localForm(), "inmansync"))
                        bill_type = INMANSYNC;
                    else if(!strcmp(s.localForm(), "ewallet"))
                        bill_type = EWALLET;
                }
            }            

            return;
        }
}

void XMLTariffMatrixHandler::endElement(const XMLCh* const nm)
{
    uint32_t mt, cat;
    StrX XMLQName(nm);
    const char *qname = XMLQName.localForm();

    for( size_t i = 0; i < CATEGORY_TAGS_SZ; i++)
        if(!strcmp(category_tags[i].name, qname) && category_tag == i)
        {
            category_tag = category_tags[i].top_tag;
            if(i == 1)
            {
                if(!category_id || category_name.length() == 0)
                    throw Exception("Invalid XML 'category' record");

                category_str_hash->Insert(category_name.c_str(), category_id);

                smsc_log_debug(logger,"end_category: store %d, %s", category_id, category_name.c_str());
                category_id = 0;
                category_name = "";
            }
            return;
        }
    for( size_t i = 0; i < MEDIA_TYPE_TAGS_SZ; i++)
        if(!strcmp(media_type_tags[i].name, qname) && media_type_tag == i)
        {
            media_type_tag = media_type_tags[i].top_tag;
            if(i == 1)
            {
                if(!media_type_id || media_type_name.length() == 0)
                    throw Exception("Invalid XML 'media_type' record");
           
                media_type_str_hash->Insert(media_type_name.c_str(), media_type_id);

                smsc_log_debug(logger,"end_media_type: store %d, %s", media_type_id, media_type_name.c_str());
                media_type_id = 0;
                media_type_name = "";
            }
            return;
        }

    for( size_t i = 0; i < BILL_TAGS_SZ; i++)
        if(!strcmp(bill_tags[i].name, qname) && bill_tag == i)
        {
            bill_tag = bill_tags[i].top_tag;
            if(i == 2)
            {
                if(!bill_category_id || !bill_media_type_id ||
                    bill_service_number.length() == 0 || bill_currency.length() == 0 || !bill_operator_id)
                    throw Exception("Invalid XML 'billing' record");

                //if(bill_price == 0)
                    //smsc_log_warn(logger, "Zero price in tariff matrix. ServiceNumber=%s, CategoryId=%d, MediaTypeId=%d, billType=%d", bill_service_number.c_str(), bill_category_id, bill_media_type_id, bill_type);

                TariffRec tr(bill_service_number, bill_price, bill_currency, bill_category_id, bill_media_type_id, bill_type);

                if(!media_type_hash->Get(bill_media_type_id, mt))
                {
                    mt = media_type_idx++;
                    media_type_hash->Insert(bill_media_type_id, mt);
                }
                if(!category_hash->Get(bill_category_id, cat))
                {
                    cat = category_idx++;
                    category_hash->Insert(bill_category_id, cat);
                }

                /*
                uint32_t id = (cat & 0x1ff) << 23;
                id |= (mt & 0x1FF) << 14;
                id |= bill_operator_id & 0xFFF;
                 */
                const uint32_t id = TariffRec::makeHashKeyChecked(cat,mt,bill_operator_id);

                smsc_log_debug(logger,"end_billing: store ci:%d, mt:%d, sn:%s, price:%s, op_id:%d, curr:%s, mt_idx:%d, cat_idx:%d, bill_type: %d", bill_category_id, bill_media_type_id, bill_service_number.c_str(), bill_price.c_str(), bill_operator_id, bill_currency.c_str(), media_type_idx, category_idx, bill_type);

                tariff_hash->Insert(id, tr);

                bill_category_id = 0;
                bill_media_type_id = 0;
                bill_service_number = "";
                bill_price = "";
                bill_currency = "";
                bill_type = 0;
            } else if(i == 1)
                bill_operator_id = 0;
            return;
        }
}

void XMLTariffMatrixHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    throw Exception("Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLTariffMatrixHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    throw Exception("Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLTariffMatrixHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

//#undef CATEGORY_TAGS_SZ
//#undef MEDIA_TYPE_TAGS_SZ
#undef BILL_TAGS_SZ

}}}


