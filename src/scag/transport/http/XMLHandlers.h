/* $Id$ */

#ifndef __SCAG_TRANSPORT_HTTP_XMLHANDLERS_H__
#define __SCAG_TRANSPORT_HTTP_XMLHANDLERS_H__

#include <xercesc/sax/HandlerBase.hpp>

#include <util/Exception.hpp>

#include <core/buffers/IntHash.hpp>
#include <logger/Logger.h>

#include "RouterTypes.h"

namespace scag { namespace transport { namespace http {

using namespace smsc::util;
using namespace smsc::logger;
using smsc::core::buffers::Hash;

XERCES_CPP_NAMESPACE_USE

class XMLBasicHandler : public HandlerBase
{
    Hash<StringArray> subj_hash;
    Hash<SiteArray> site_subj_hash;
    uint32_t service_id;
    Site site;
    StringArray addrs;
    SiteArray sites;

    std::string subj_id;
    std::string site_subj_id;
    RouteArray* routes;
    HttpRouteInt route;
    bool route_enabled;
    bool in_sites, in_abonents;

    void characters(const XMLCh *const chars, const unsigned int length);

    void insertPlacement(PlacementArray* pa, Placement& p);
    Placement assignPlacement(std::string& rid, AttributeList& attrs);

    Logger *logger;

public:
    XMLBasicHandler(RouteArray*);
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
    char* fLocalForm;
};

}}}

#endif
