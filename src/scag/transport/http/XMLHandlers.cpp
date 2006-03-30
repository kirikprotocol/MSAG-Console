/* $Id$ */

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include "XMLHandlers.h"
#include <locale.h>

namespace scag { namespace transport { namespace http {

using namespace smsc::util;

XMLBasicHandler::XMLBasicHandler(RouteArray* r)
{
    logger = Logger::getInstance("httpxml");
    in_route = false;
    in_subj_def = false;
    routes = r;
}

void XMLBasicHandler::characters(const XMLCh *const chars, const unsigned int length) 
{
}

void XMLBasicHandler::startElement(const XMLCh* const nm, AttributeList& attrs)
{
    StrX XMLQName(nm);
    const char *qname = XMLQName.localForm();

    if(!strcmp(qname, "route"))
    {
        in_route = true;
        StrX s = attrs.getValue("id");
        route.id = s.localForm();
        StrX s1 = attrs.getValue("serviceId");
        route.service_id = atoi(s1.localForm());
    }
    else if((in_route || in_subj_def) && !strcmp(qname, "address"))
    {
        StrX s = attrs.getValue("value");
        std::string str = s.localForm();
        if(in_route)
            route.masks.Push(str);
        else
        {
            if(subj_id == 0)
                throw Exception("Invalid XML subject id");

            try{
                Array<std::string>& masks = subj_hash.Get(subj_id.c_str());
                masks.Push(str);
            }
            catch(HashInvalidKeyException &e)
            {
                StringArray m;
                m.Push(str);
                subj_hash.Insert(subj_id.c_str(), m);
            }
        }
    }
    else if(in_route && !strcmp(qname, "url"))
    {
        StrX s = attrs.getValue("value");
        route.url = s.localForm();
    }
    else if(in_route && !strcmp(qname, "subject"))
    {
        StrX s = attrs.getValue("id");

        try{
            StringArray& masks = subj_hash.Get(s.localForm());
            for(int i = 0; i < masks.Count(); i++)
                route.masks.Push(masks[i]);
        }
        catch(HashInvalidKeyException &e)
        {
            std::string str = "Invalid XML subject not found id=";
            str += s.localForm();
            throw Exception(str.c_str());
        }
    }
    else if(!strcmp(qname, "subject_def"))
    {
        StrX s = attrs.getValue("id");
        subj_id = s.localForm();
        in_subj_def = true;
    }
}

void XMLBasicHandler::endElement(const XMLCh* const nm)
{
    StrX XMLQName(nm);
    const char *qname = XMLQName.localForm();

    if(!strcmp(qname, "route"))
    {
        in_route = false;

        if(route.id.length() == 0 || route.service_id == 0 || route.url.length() == 0 || route.masks.Count() == 0)
            throw Exception("Invalid XML http_route record");

        routes->Push(route);
        route.id = "";
        route.service_id = 0;
        route.url = "";
        route.masks.Empty();
    }
    else if(!strcmp(qname, "subject_def"))
    {
        subj_id = "";
        in_subj_def = false;
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


