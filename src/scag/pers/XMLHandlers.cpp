/* $Id$ */

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include "XMLHandlers.h"
#include <locale.h>
#include "sms/sms.h"

namespace scag { namespace cpers {

XMLBasicHandler::XMLBasicHandler(smsc::core::buffers::IntHash<RegionInfo> *h)
{
    logger = Logger::getInstance("xmlhndlr");
    regionHash = h;
}

void XMLBasicHandler::characters(const XMLCh *const chars, const unsigned int length) 
{
    throw Exception("Region cannot has body characters");
}

void XMLBasicHandler::startElement(const XMLCh* const qname, AttributeList& attrs)
{
    StrX XMLQName(qname);
    RegionInfo ri;
    if(!strcmp(XMLQName.localForm(), "region"))
    {
        StrX s = attrs.getValue("id");
        if(!(ri.id = atoi(s.localForm()) ))
            throw Exception("Invalid region id");
        StrX s1 = attrs.getValue("name");
        ri.name = s1.localForm();
        if(!ri.name.length())
            throw Exception("Empty region name");
        StrX s2 = attrs.getValue("passwd");
        ri.passwd = s2.localForm();
        if(!ri.passwd.length())
            throw Exception("Empty region passwd, id=%d, name=%s", ri.id, ri.name.c_str());
        smsc_log_debug(logger, "Region added: id=%d, name=%s, passwd=%s", ri.id, ri.name.c_str(), ri.passwd.c_str());            
        regionHash->Insert(ri.id, ri);
    }
}

void XMLBasicHandler::endElement(const XMLCh* const qname)
{
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

}}
