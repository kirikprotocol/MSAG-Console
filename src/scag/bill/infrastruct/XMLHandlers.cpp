/* $Id$ */

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include "XMLHandlers.h"

namespace scag { namespace bill { namespace infrastruct {

using namespace smsc::util;

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

//------------------------------------------------------------------------------
/*XMLTariffMatrixHandler::XMLTariffMatrixHandler(Hash<uint32_t> *cat, Hash<uint32_t> *mt)
{
    logger = Logger::getInstance("xmlhndlr");
	category_hash = cat;
	media_type_hash = mt;
	category_level = 0;
	media_type_level = 0;
	in_category_id = false;
	in_category_name = false;
	in_media_type_id = false;
	in_media_type_name = false;
	category_id = 0;
	category_name = "";
	media_type_id = 0;
	media_type_name = "";
}

void XMLTariffMatrixHandler::characters(const XMLCh *const chars, const unsigned int length) 
{
	if(in_media_type_id)
	{
    smsc_log_error(logger,"in_media_type_id");
		mask_chars[mask_len] = 0;
		StrX s(mask_chars);

		media_type_id = atoi();
	}
	else if(in_media_type_name)
	{
    smsc_log_error(logger,"in_media_type_name");
	}
	else if(in_category_id)	
	{
    smsc_log_error(logger,"in_category_id");
	}
	else if(in_category_name)	
	{
    smsc_log_error(logger,"in_category_name");
	}


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

void XMLTariffMatrixHandler::startElement(const XMLCh* const qname, AttributeList& attrs)
{
    StrX XMLQName(qname);

	if(!strcmp(qname, "tariff_matrix"))
	{
		if(!category_level) category_level++;
		if(!media_type_level) media_type_level++;
	} else if(category_level == 1 && !strcmp(qname, "categories"))
		category_level++;
	else if(category_level == 2 && !strcmp(qname, "category"))
		category_level++;
	else if(category_level == 3 && !strcmp(qname, "id"))
		in_category_id = true;
	else if(category_level == 3 && !strcmp(qname, "name"))
		in_category_name = true;

	else if(media_type_level == 1 && !strcmp(qname, "media_types"))
		media_type_level++;
	else if(media_type_level == 2 && !strcmp(qname, "media_type"))
		media_type_level++;
	else if(media_type_level == 3 && !strcmp(qname, "id"))
		in_media_type_id = true;
	else if(media_type_level == 3 && !strcmp(qname, "name"))
		in_media_type_name = true;

	{
	 	StrX s = attrs.getValue("id");
		uint32_t id = atoi(s.localForm());
		if(!id)
			throw Exception("Invalid service id");
		service_hash->Insert(id, cur_provider_id);
	}
}

void XMLTariffMatrixHandler::endElement(const XMLCh* const qname)
{
    StrX XMLQName(qname);

	if(!strcmp(qname, "tariff_matrix"))
	{
		if(category_level) category_level--;
		if(media_type_level) media_type_level--;
	} else if(category_level == 2 && !strcmp(qname, "categories"))
		category_level--;
	else if(category_level == 3 && !strcmp(qname, "category"))
	{
		category_level--;
	    smsc_log_error(logger,"end_category: store");
	}
	else if(category_level == 4 && !strcmp(qname, "id"))
		in_category_id = false;
	else if(category_level == 4 && !strcmp(qname, "name"))
		in_category_name = false;
	
	else if(media_type_level == 2 && !strcmp(qname, "media_types"))
		media_type_level--;
	else if(media_type_level == 3 && !strcmp(qname, "media_type"))
	{
		media_type_level--;
	    smsc_log_error(logger,"end_media_type: store");
	}
	else if(media_type_level == 4 && !strcmp(qname, "id"))
		in_media_type_id = false;
	else if(media_type_level == 4 && !strcmp(qname, "name"))
		in_media_type_name = false;

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
		}
		in_mask = false;
	}
}

void XMLTariffMatrixHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLTariffMatrixHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());

}

void XMLTariffMatrixHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}*/

}}}

