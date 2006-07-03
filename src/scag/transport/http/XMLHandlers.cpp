/* $Id$ */

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include "XMLHandlers.h"
#include <locale.h>

namespace scag { namespace transport { namespace http {

using namespace smsc::util;

XMLBasicHandler::XMLBasicHandler(RouteArray* r, PlacementArray* ap)
{
    logger = Logger::getInstance("httpxml");
    routes = r;
    in_options = in_sites = in_abonents = false;
    addrPlace = ap;
}

void XMLBasicHandler::characters(const XMLCh *const chars, const unsigned int length) 
{
}

Placement XMLBasicHandler::assignPlacement(const std::string& rid, AttributeList& attrs)
{
    Placement p;

    StrX s = attrs.getValue("type");
    if(!p.setType(s.localForm()))
        throw Exception("Invalid placement type: route id=%s, type=%s", route.id.c_str(), s.localForm());

    StrX s1 = attrs.getValue("name");
    p.name = s1.localForm();

    StrX s2 = attrs.getValue("priority");
    p.prio = atoi(s2.localForm());
    if(!p.prio)
        throw Exception("Invalid priority: route id=%s, prio=%s", route.id.c_str(), s2.localForm());

    return p;
}

void XMLBasicHandler::insertPlacement(PlacementArray* pa, const Placement& p)
{
    int i = 0;

    while(i < pa->Count() && pa->operator[](i).prio < p.prio)
        i++;

    pa->Insert(i, p);
}

void XMLBasicHandler::startElement(const XMLCh* const nm, AttributeList& attrs)
{
    StrX XMLQName(nm);
    const char *qname = XMLQName.localForm();

//    smsc_log_debug(logger, "Start element %s route_id=%s", qname, route.id.c_str());

    if(!strcmp(qname, "options"))
        in_options = true;
    else if(!strcmp(qname, "sites"))
        in_sites = true;
    else if(!strcmp(qname, "abonents"))
        in_abonents = true;
    else if(route.id.length() && !strcmp(qname, "usr_place"))
    {
        Placement p = assignPlacement(route.id, attrs);
        PlacementArray* pa;
        if(in_abonents)
            pa = &route.inUSRPlace;
        else if(in_sites)
            pa = &route.outUSRPlace;
        else
            throw Exception("Invalid XML usr_place record: route id=%s, name=%s, type=%d, prio=%d", route.id.c_str(), p.name.c_str(), p.type, p.prio);

        insertPlacement(pa, p);
            
        smsc_log_debug(logger, "usr_place record: route id=%s, name=%s, type=%d, prio=%d", route.id.c_str(), p.name.c_str(), p.type, p.prio);
    }
    else if(!strcmp(qname, "address_place"))
    {
        if(in_options)
        {
            Placement p = assignPlacement("", attrs);
            insertPlacement(addrPlace, p);
            smsc_log_debug(logger, "address_place record: [options] name=%s, type=%d, prio=%d", p.name.c_str(), p.type, p.prio);
        }
        else if(route.id.length() && in_sites)
        {
            Placement p = assignPlacement(route.id, attrs);
            insertPlacement(&route.outAddressPlace, p);
            smsc_log_debug(logger, "address_place record: route id=%s, name=%s, type=%d, prio=%d", route.id.c_str(), p.name.c_str(), p.type, p.prio);
        }
        else
            throw Exception("Invalid XML address_place record: No route id or not in sites");
    }
    else if(!strcmp(qname, "route"))
    {
        StrX s = attrs.getValue("id");
        route.id = s.localForm();
        StrX s1 = attrs.getValue("serviceId");
        route.service_id = atoi(s1.localForm());
        StrX s2 = attrs.getValue("enabled");
        route_enabled = true;
        if(s2.localForm() && !strcmp(s2.localForm(), "false"))
            route_enabled = false;
    }
    else if((route.id.length() || subj_id.length()) && !strcmp(qname, "address"))
    {
        StrX s = attrs.getValue("value");
        std::string str = s.localForm();
        if(route.id.length())
            route.masks.Push(str);
        else
        {
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
    else if(route.id.length() && !strcmp(qname, "subject"))
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
            str += "route_id=" + route.id;
            throw Exception(str.c_str());
        }
    }
    else if(route.id.length() && !strcmp(qname, "site_subject"))
    {
        StrX s = attrs.getValue("id");

        try{
            SiteArray& st = site_subj_hash.Get(s.localForm());
            for(int i = 0; i < st.Count(); i++)
                route.sites.Push(st[i]);
        }
        catch(HashInvalidKeyException &e)
        {
            std::string str = "Invalid XML site_subject not found id=";
            str += s.localForm();
            str += "route_id=" + route.id;
            throw Exception(str.c_str());
        }
    }
    else if(!strcmp(qname, "subject_def"))
    {
        StrX s = attrs.getValue("id");
        subj_id = s.localForm();
    }
    else if(!strcmp(qname, "site_subject_def"))
    {
        StrX s = attrs.getValue("id");
        site_subj_id = s.localForm();
    }
    else if((site_subj_id.length() || route.id.length()) && !strcmp(qname, "site"))
    {
        StrX s = attrs.getValue("host");
        site.host = s.localForm();
        StrX s1 = attrs.getValue("port");
        site.port = atoi(s1.localForm());
    }
    else if(site.host.length() && !strcmp(qname, "path"))
    {
        StrX s = attrs.getValue("value");
        site.paths.Push(s.localForm());
    }
}

void XMLBasicHandler::endElement(const XMLCh* const nm)
{
    StrX XMLQName(nm);
    const char *qname = XMLQName.localForm();


    if(!strcmp(qname, "options"))
        in_options = false;
    else if(!strcmp(qname, "sites"))
        in_sites = false;
    else if(!strcmp(qname, "abonents"))
        in_abonents = false;
    else if(!strcmp(qname, "route"))
    {
        if(route.id.length() == 0 || route.service_id == 0 || route.sites.Count() == 0 || route.masks.Count() == 0)
            throw Exception("Invalid XML http_route record");

        if(route_enabled)
        {
//            smsc_log_debug(logger, "Push route id=%s", route.id.c_str());
            routes->Push(route);
        }

        route_enabled = true;
        route.id = "";
        route.service_id = 0;
        route.masks.Empty();
        route.sites.Empty();
    }
    else if(!strcmp(qname, "subject_def"))
        subj_id = "";
    else if(!strcmp(qname, "site_subject_def"))
    {
        if(site_subj_id.length() == 0 || sites.Count() == 0)
            throw Exception("Invalid XML site_subj_def record");

        site_subj_hash.Insert(site_subj_id.c_str(), sites);

        site_subj_id = "";
        sites.Empty();
    }
    else if(!strcmp(qname, "site"))
    {
        if(site.host.length() == 0 || site.port == 0 || site.paths.Count() == 0)
            throw Exception("Invalid XML site record");

        if(route.id.length())
            route.sites.Push(site);
        else if(site_subj_id.length())
            sites.Push(site);

        site.host = "";
        site.port = 0;
        site.paths.Empty();
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
    smsc_log_error(logger, "Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

}}}


