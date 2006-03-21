/* $Id$ */

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/TransService.hpp>

#include "XMLHandlers.h"

namespace scag { namespace bill { namespace infrastruct {

using namespace smsc::util;

XMLBasicHandler::XMLBasicHandler(const char* const encodingName, IntHash<uint32_t> *h)
{
    logger = Logger::getInstance("xmlhndlr");
	cur_provider_id = 0;
	service_hash = h;
	type = 0;
}

XMLBasicHandler::XMLBasicHandler(const char* const encodingName, Hash<uint32_t> *h)
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
	if(in_mask)
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
		smsc_log_debug(logger, "cur_prov_id=%d", cur_provider_id);
	} else if(cur_provider_id && !strcmp(qname, "service"))
	{
	 	StrX s = attrs.getValue("id");
		uint32_t id = atoi(s.localForm());
		if(!id)
			throw Exception("Invalid service id");
		service_hash->Insert(id, cur_provider_id);
		smsc_log_debug(logger, "prov_id:<<%d>> serv_id=%d", cur_provider_id, id);
	}
}

void XMLBasicHandler::OperatorMapStartElement(const char* qname, AttributeList& attrs)
{
	if(!strcmp(qname, "operator"))
	{
	 	StrX s = attrs.getValue("id");
		if(!(cur_operator_id = atoi(s.localForm()) ))
			throw Exception("Invalid operator id");
		smsc_log_debug(logger, "cur_op_id=%d", cur_operator_id);
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
	if(!strcmp(XMLQName.localForm(), "provider"))
		cur_provider_id = 0;
	else if(!strcmp(XMLQName.localForm(), "operator"))
		cur_operator_id = 0;
	else if(cur_operator_id && !strcmp(XMLQName.localForm(), "mask"))
	{
		if(mask_len > 0)
		{
			mask_chars[mask_len] = 0;
			StrX s(mask_chars);
			abonent_hash->Insert(s.localForm(), cur_operator_id);
			smsc_log_debug(logger, "mask: %s", s.localForm());
		}
		in_mask = false;
	}
}

void XMLBasicHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLBasicHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());

}

void XMLBasicHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

}}}

