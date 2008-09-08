/* $Id$ */

#ifndef __SCAG_TRANSPORT_HTTP_IMPL_XMLHANDLERS_H__
#define __SCAG_TRANSPORT_HTTP_IMPL_XMLHANDLERS_H__

#include <xercesc/sax/HandlerBase.hpp>

#include <util/Exception.hpp>

#include <core/buffers/IntHash.hpp>
#include <logger/Logger.h>

#include "RouterTypes.h"

namespace scag2 {
namespace transport {
namespace http {

using namespace smsc::util;
using namespace smsc::logger;
using smsc::core::buffers::Hash;

XERCES_CPP_NAMESPACE_USE

struct SiteSubjDef
{
    std::string id;
    SiteArray sites;
};

class XMLBasicHandler : public HandlerBase
{
    Hash<StringArray> subj_hash;
    Hash<SiteSubjDef> site_subj_hash;
    uint32_t service_id;
    Site site;
    StringArray addrs;

    std::string subj_id;
    SiteSubjDef site_subj;
    RouteArray* routes;
    HttpRouteInt route;
    PlacementKindArray* inPlace;
    PlacementKindArray* outPlace;    
    bool route_enabled;
    bool in_sites, in_abonents, in_options;

    void characters(const XMLCh *const chars, const unsigned int length);

    void insertPlacement(PlacementArray* pa, const Placement& p);
    Placement assignPlacement(const std::string& rid, AttributeList& attrs, bool req);
    void handlePlacement(uint32_t k, AttributeList& attrs);

    Logger *logger;

    uint32_t getKind(const std::string& s);
    bool getBool(AttributeList& attrs, const char *v, bool def);
public:
    XMLBasicHandler(RouteArray*, PlacementKindArray*, PlacementKindArray*);
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
        s[0] = 0;
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }

    const char* localForm() const
    {
        return fLocalForm ? fLocalForm : s;
    }

private :
    char* fLocalForm, s[1];
};

}}}

#endif
