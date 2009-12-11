/* $Id$ */

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include "XMLHandlers.h"
#include <locale.h>

namespace scag2 {
namespace transport {
namespace http {

using namespace smsc::util;

XMLBasicHandler::XMLBasicHandler(RouteArray* r, PlacementKindArray* inap, PlacementKindArray* outap)
{
    logger = Logger::getInstance("http.xml");
    routes = r;
    in_options = in_sites = in_abonents = false;
    inPlace = inap;
    outPlace = outap;
}

void XMLBasicHandler::characters(const XMLCh *const chars, const unsigned int length) 
{
}

Placement XMLBasicHandler::assignPlacement(const std::string& rname, AttributeList& attrs, bool req)
{
    Placement p;
    StrX s = attrs.getValue("type");
    if(!p.setType(s.localForm()))
        throw Exception("Invalid placement type: route_name=%s, type=%s", route.name.c_str(), s.localForm());

    StrX s1 = attrs.getValue("name");
    p.name = s1.localForm();

//    for (unsigned int i = 0; i < p.name.length(); ++i)
//        p.name[i] = tolower(p.name[i]);

    p.prio = 0;
    if(req)
    {
        const XMLCh* c = attrs.getValue("priority");
        if(c != NULL)
        {
            StrX s2 = attrs.getValue("priority");
            p.prio = atoi(s2.localForm());
        }
        if(!p.prio)
            throw Exception("Priority is required: route_name=%s", route.name.c_str());
    }
    return p;
}

void XMLBasicHandler::insertPlacement(PlacementArray* pa, const Placement& p)
{
    int i = 0;

    if(p.prio)
    {
        while(i < pa->Count() && pa->operator[](i).prio < p.prio)
            i++;
    }

    pa->Insert(i, p);
}

uint32_t XMLBasicHandler::getKind(const std::string& s)
{
    if(s.compare("address_place") == 0)    return PlacementKind::ADDR;
    if(s.compare("route_id_place") == 0)   return PlacementKind::ROUTE_ID;
    if(s.compare("service_id_place") == 0) return PlacementKind::SERVICE_ID;
    return PlacementKind::UNKNOWN;
}

void XMLBasicHandler::handlePlacement(uint32_t k, AttributeList& attrs)
{
    if(in_options)
    {
        Placement p = assignPlacement("", attrs, in_abonents);
        if(in_abonents)
            insertPlacement(&(*inPlace)[k], p);
        else if(in_sites)
            insertPlacement(&(*outPlace)[k], p);
//        smsc_log_debug(logger, "place record: [options] kind=%d name=%s, type=%d, prio=%d", k, p.name.c_str(), p.type, p.prio);
    }
    else if(route.id)
    {
        Placement p = assignPlacement(route.name, attrs, in_abonents);

        if(in_abonents)
            insertPlacement(&route.inPlace[k], p);
        else if(in_sites)
            insertPlacement(&route.outPlace[k], p);

//        smsc_log_debug(logger, "place record: route name=%s, name=%s, type=%d, prio=%d", route.name.c_str(), p.name.c_str(), p.type, p.prio);
    }
    else
        throw Exception("Invalid XML usr_place record: No route id");
}

bool XMLBasicHandler::getBool(AttributeList& attrs, const char *v, bool def)
{
    StrX s2 = attrs.getValue(v);
    if(s2.localForm())
    {
        if(!strcmp(s2.localForm(), "false"))
            return false;
        else if(!strcmp(s2.localForm(), "true"))
            return true;
    }
    return def;
}

void XMLBasicHandler::startElement(const XMLCh* const nm, AttributeList& attrs)
{
    uint32_t placeKind;
    StrX XMLQName(nm);
    const char *qname = XMLQName.localForm();

//    smsc_log_debug(logger, "Start element %s route_id=%s", qname, route.id.c_str());
    if(!strcmp(qname, "options"))
        in_options = true;
    else if(!strcmp(qname, "sites"))
        in_sites = true;
    else if(!strcmp(qname, "abonents"))
        in_abonents = true;
    else if(!strcmp(qname, "address_prefix") )
    {
        StrX s = attrs.getValue("ton");
        StrX s1 = attrs.getValue("npi");
        if(route.id)
        {
            route.addressPrefix = '.';
            route.addressPrefix += s.localForm();
            route.addressPrefix += '.';
            route.addressPrefix += s1.localForm();
            route.addressPrefix += '.';
        }
        else
            throw Exception("Invalid XML address_prefix: No route id");
    }
    else if((placeKind = getKind(qname)) != PlacementKind::UNKNOWN) {
          handlePlacement(placeKind, attrs);
    }
    else if(!strcmp(qname, "route"))
    {
        StrX s = attrs.getValue("name");
        route.name = s.localForm();
        StrX s1 = attrs.getValue("id");
        route.id = atoi(s1.localForm());
        if(!route.id)
            throw Exception("Invalid XML route id: name = %s, id = %s", s.localForm(), s1.localForm());
        StrX s2 = attrs.getValue("serviceId");
        route.service_id = atoi(s2.localForm());
        if(!route.service_id)
            throw Exception("Invalid XML service id: name = %s, id = %s, service_id=%s", s.localForm(), s1.localForm(), s2.localForm());
        route.enabled = getBool(attrs, "enabled", true);
        route.def = getBool(attrs, "default", false);
        route.transit = getBool(attrs, "transit", false);
        route.statistics = getBool(attrs, "saa", true);
    }
    else if((route.id || subj_id.length()) && !strcmp(qname, "address"))
    {
        StrX s = attrs.getValue("value");
//        Address addr(s.localForm());
        if(route.id)
            route.masks.Push(s.localForm());
        else
        {
            try{
                Array<std::string>& masks = subj_hash.Get(subj_id.c_str());
                masks.Push(s.localForm());
            }
            catch(HashInvalidKeyException &e)
            {
                StringArray m;
                m.Push(s.localForm());
                subj_hash.Insert(subj_id.c_str(), m);
            }
        }
    }
    else if(route.id && !strcmp(qname, "subject"))
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
            str += "route_name=%s" + route.name;
            throw Exception(str.c_str());
        }
    }
    else if(route.id && !strcmp(qname, "site_subject"))
    {
        StrX s = attrs.getValue("id");
        bool d = getBool(attrs, "default", false);

        try{
            SiteSubjDef& st = site_subj_hash.Get(s.localForm());
            for(int i = 0; i < st.sites.Count(); i++)
            {
                bool p = st.sites[i].def;
                if(!d && st. sites[i].def) st.sites[i].def = false;
                route.sites.Push(st.sites[i]);
                st.sites[i].def = p;
            }
        }
        catch(HashInvalidKeyException &e)
        {
            std::string str = "Invalid XML site_subject not found id=";
            str += s.localForm();
            str += "route_name=" + route.name;
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
        site_subj.id = s.localForm();
    }
    else if((site_subj.id.length() || route.id) && !strcmp(qname, "site"))
    {
        StrX s = attrs.getValue("host");
        site.host = s.localForm();
        StrX s1 = attrs.getValue("port");
        site.port = atoi(s1.localForm());
        site.def = getBool(attrs, "default", false);
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
        if(route.id == 0 || route.service_id == 0 || route.sites.Count() == 0 || route.masks.Count() == 0)
            throw Exception("Invalid XML http_route record");

//        smsc_log_debug(logger, "Push route. %s", route.toString().c_str());

        routes->Push(route);

        route.enabled = true;
        route.id = 0;
        route.name = "";
        route.addressPrefix = "";
        route.service_id = 0;
        route.transit = route.def = false;
        route.statistics = true;
        route.enabled = true;
        route.masks.Empty();
        route.sites.Empty();
        for(int i = 0; i < PLACEMENT_KIND_COUNT; i++)
        {
            route.inPlace[i].Empty();
            route.outPlace[i].Empty();
        }
    }
    else if(!strcmp(qname, "subject_def"))
        subj_id = "";
    else if(!strcmp(qname, "site_subject_def"))
    {
        if(site_subj.id.length() == 0 || site_subj.sites.Count() == 0)
            throw Exception("Invalid XML site_subj_def record");

        site_subj_hash.Insert(site_subj.id.c_str(), site_subj);
        smsc_log_debug(logger, "Site subj def inserted id=%s", site_subj.id.c_str());
        site_subj.id = "";
        site_subj.sites.Empty();
    }
    else if(!strcmp(qname, "site"))
    {
        if(site.host.length() == 0 || site.port == 0 || site.paths.Count() == 0)
            throw Exception("Invalid XML site record");

        if(route.id)
            route.sites.Push(site);
        else if(site_subj.id.length())
            site_subj.sites.Push(site);

        site.host = "";
        site.port = 0;
        site.def = false;
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


