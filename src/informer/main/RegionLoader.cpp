#include "RegionLoader.h"
#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"
#include "util/xml/DOMTreeReader.h"
#include "util/findConfigFile.h"
#include "informer/io/InfosmeException.h"
#include "informer/data/CommonSettings.h"

using namespace smsc::util::xml;
using namespace xercesc;

namespace eyeline {
namespace informer {

RegionLoader::RegionLoader( const char* xmlfile,
                            const char* defaultSmscId,
                            regionid_type requestedRegionId ) :
log_(smsc::logger::Logger::getInstance("regloader"))
{
    smsc_log_info(log_,"region loader from '%s', default smsc S='%s', requestedId=%u",
                  xmlfile, defaultSmscId, requestedRegionId );
    try {
        initXerces();
        DOMTreeReader reader;
        const std::string cfgfile(findConfigFile(xmlfile));
        DOMDocument* document = reader.read(cfgfile.c_str());
        if (!document) throw InfosmeException(EXC_CONFIG,"parse result is null");
        DOMElement* root = document->getDocumentElement();
        if (!root) throw InfosmeException(EXC_CONFIG,"no root element");

        smsc_log_debug(log_,"xmlfile is parsed, root element obtained");

        // default region
        DOMNodeList* regions;
        if (requestedRegionId == defaultRegionId || requestedRegionId == anyRegionId ) {
            const regionid_type rid(defaultRegionId);
            smsc_log_debug(log_,"reading default region R=%u",rid);
            regions = root->getElementsByTagName(XmlStr("region_default"));
            if (regions->getLength() != 1) {
                throw InfosmeException(EXC_CONFIG,"default region is not found!");
            }
            DOMElement* region = static_cast<DOMElement*>(regions->item(0));
            const XmlStr sbandwidth = region->getAttribute(XmlStr("bandwidth"));
            char* endptr;
            const unsigned bandwidth(unsigned(strtoul(sbandwidth.c_str(),&endptr,10)));
            if (*endptr != '\0') {
                throw InfosmeException(EXC_CONFIG,"invalid bandwidth='%s' for region id=%u",
                                       sbandwidth.c_str(),rid);
            }
            const XmlStr stimezone = region->getAttribute(XmlStr("timezone"));
            int timezone;
            std::string tzname;
            if ( strlen(stimezone.c_str()) == 0 ) {
                // empty
                timezone = getLocalTimezone();
            } else {
                int shift = 0;
                sscanf(stimezone.c_str(),"%d,%n",&timezone,&shift);
                if (shift == 0) {
                    throw InfosmeException(EXC_CONFIG,"invalid timezone='%s' for region id=%u",
                                           stimezone.c_str(),rid);
                }
                const char* p = stimezone.c_str() + shift;
                while ( *p == ' ' || *p == '\t' ) { ++p; }
                tzname = p;
                size_t plen = tzname.size();
                if ( plen ) {
                    --plen;
                    for ( ; tzname[plen] == ' ' ; --plen ) {
                        tzname.erase(plen);
                        if ( !plen ) break;
                    }
                }
            }
            const TimezoneGroup* tzgroup = getCS()->lookupTimezoneGroup(tzname.c_str());
            if ( !tzgroup ) {
                throw InfosmeException(EXC_CONFIG,"unknown timezone='%s'",tzname.c_str());
            }
            if ( timezone/3600*3600 != timezone ) {
                throw InfosmeException(EXC_CONFIG,"invalid timezone=%d, it does not divisable by 3600 w/o remainder");
            }
            Region* r = new Region(rid,"default",defaultSmscId,bandwidth,
                                   timezone,
                                   tzgroup,
                                   false,0);
            regions_.push_back(r);
        }

        // other regions
        size_t count = 0;
        if (requestedRegionId == anyRegionId || requestedRegionId != defaultRegionId ) {
            regions = root->getElementsByTagName(XmlStr("region"));
            count = regions->getLength();
            if ( requestedRegionId == anyRegionId ) {
                regions_.reserve(regions_.size() + count);
            }
        }
        std::vector<std::string> masks;
        for ( size_t i = 0; i < count; ++i ) {
            DOMElement* region = static_cast<DOMElement*>(regions->item(i));
            const XmlStr id = region->getAttribute(XmlStr("id"));
            char* endptr;
            regionid_type rid(regionid_type(strtoul(id.c_str(),&endptr,10)));
            if (*endptr != '\0') {
                throw InfosmeException(EXC_CONFIG,"invalid id='%s'",id.c_str());
            }
            if ( rid == defaultRegionId || rid == anyRegionId ) {
                throw InfosmeException(EXC_CONFIG,"invalid region id=%u",rid);
            }
            if ( requestedRegionId != anyRegionId && rid != requestedRegionId ) continue;
            smsc_log_debug(log_,"reading region R=%u",rid);

            // loading masks
            DOMNodeList* maskList = region->getElementsByTagName(XmlStr("mask"));
            const size_t maskSize = maskList->getLength();
            if (maskSize==0) {
                // region has no masks!
                continue;
            }
            masks.reserve(maskSize);
            for ( size_t j = 0; j < maskSize; ++j ) {
                DOMElement* mask = static_cast<DOMElement*>(maskList->item(j));
                const XmlStr maskValue(mask->getAttribute(XmlStr("value")));
                masks.push_back(maskValue.c_str());
            }

            const XmlStr name = region->getAttribute(XmlStr("name"));
            const XmlStr smscId = region->getAttribute(XmlStr("infosme_smsc"));
            const XmlStr sbandwidth = region->getAttribute(XmlStr("bandwidth"));
            const unsigned bandwidth(unsigned(strtoul(sbandwidth.c_str(),&endptr,10)));
            if (*endptr != '\0') {
                throw InfosmeException(EXC_CONFIG,"invalid bandwidth='%s' for region id=%u",
                                       sbandwidth.c_str(),rid);
            }
            const XmlStr stimezone = region->getAttribute(XmlStr("timezone"));
            int timezone, shift = 0;
            sscanf(stimezone.c_str(),"%d,%n",&timezone,&shift);
            if (shift == 0) {
                throw InfosmeException(EXC_CONFIG,"invalid timezone='%s' for region id=%u",
                                       stimezone.c_str(),rid);
            } else if ( timezone/3600*3600 != timezone ) {
                throw InfosmeException(EXC_CONFIG,"invalid timezone=%d, it does not divisable by 3600 w/o remainder");
            }
            const TimezoneGroup* tzgroup;
            {
                const char* p = stimezone.c_str() + shift;
                while ( *p == ' ' || *p == '\t' ) { ++p; }
                std::string tzname = p;
                size_t plen = tzname.size();
                if ( plen ) {
                    --plen;
                    for ( ; tzname[plen] == ' '; --plen ) {
                        tzname.erase(plen);
                        if ( !plen ) break;
                    }
                }
                tzgroup = getCS()->lookupTimezoneGroup(tzname.c_str());
                if (!tzgroup) {
                    throw InfosmeException(EXC_CONFIG,"unknown timezone='%s'",tzname.c_str());
                }
            }
            const XmlStr sdeleted = region->getAttribute(XmlStr("deleted"));
            const bool deleted = ( sdeleted.c_str() == std::string("true") );

            Region* r = new Region(rid,name.c_str(),smscId.c_str(),bandwidth,timezone,tzgroup,deleted,&masks);
            regions_.push_back(r);
        }

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"cannot load regions, exc: %s",e.what());
        for ( std::vector< Region* >::reverse_iterator i = regions_.rbegin();
              i != regions_.rend(); ++i ) {
            delete *i;
        }
        throw InfosmeException(EXC_CONFIG,"regions from '%s', exc: %s",xmlfile,e.what());
    }
    smsc_log_info(log_,"%u regions are loaded", unsigned(regions_.size()));
}


Region* RegionLoader::popNext()
{
    if (regions_.empty()) return 0;
    Region* r = regions_.back();
    regions_.pop_back();
    return r;
}

}
}
