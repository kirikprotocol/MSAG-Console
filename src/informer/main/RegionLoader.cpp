#include "RegionLoader.h"
#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"
#include "util/xml/DOMTreeReader.h"
#include "util/findConfigFile.h"
#include "informer/io/InfosmeException.h"

using namespace smsc::util::xml;
using namespace xercesc;

namespace eyeline {
namespace informer {

RegionLoader::RegionLoader( const char* xmlfile,
                            const char* defaultSmscId,
                            regionid_type regionId )
{
    try {
        initXerces();
        DOMTreeReader reader;
        const std::string cfgfile(findConfigFile(xmlfile));
        DOMDocument* document = reader.read(cfgfile.c_str());
        if (!document) throw InfosmeException("parse result is null");
        DOMElement* root = document->getDocumentElement();
        if (!root) throw InfosmeException("no root element");

        // default region
        DOMNodeList* regions;
        if (regionId == regionid_type(-1) || regionId == regionid_type(-2)) {
            regionid_type rid(-1);
            regions = root->getElementsByTagName(XmlStr("region_default"));
            if (regions->getLength() != 1) {
                throw InfosmeException("default region is not found!");
            }
            DOMElement* region = static_cast<DOMElement*>(regions->item(0));
            const XmlStr sbandwidth = region->getAttribute(XmlStr("max_per_second"));
            char* endptr;
            const unsigned bandwidth = strtoul(sbandwidth.c_str(),&endptr,10);
            if (*endptr != '\0') {
                throw InfosmeException("invalid max_per_second='%s' for region id=%u",
                                       sbandwidth.c_str(),rid);
            }
            Region* r = new Region(rid,"default",defaultSmscId,bandwidth,0);
            regions_.push_back(r);
        }

        // other regions
        size_t count = 0;
        if (regionId == regionid_type(-2) || regionId != regionid_type(-1) ) {
            regions = root->getElementsByTagName(XmlStr("region"));
            count = regions->getLength();
            if ( regionId == regionid_type(-2) ) {
                regions_.reserve(regions_.size() + count);
            }
        }
        std::vector<std::string> masks;
        for ( size_t i = 0; i < count; ++i ) {
            DOMElement* region = static_cast<DOMElement*>(regions->item(i));
            const XmlStr id = region->getAttribute(XmlStr("id"));
            char* endptr;
            regionid_type rid = strtoul(id.c_str(),&endptr,10);
            if (*endptr != '\0') {
                throw InfosmeException("invalid id='%s'",id.c_str());
            }
            if ( rid == regionid_type(-1) || rid == regionid_type(-2) ) {
                throw InfosmeException("invalid region id=%u",rid);
            }
            if ( regionId != regionid_type(-2) && rid != regionId ) continue;

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
            const XmlStr sbandwidth = region->getAttribute(XmlStr("max_per_second"));
            const unsigned bandwidth = strtoul(sbandwidth.c_str(),&endptr,10);
            if (*endptr != '\0') {
                throw InfosmeException("invalid max_per_second='%s' for region id=%u",
                                       sbandwidth.c_str(),rid);
            }
            const XmlStr stimezone = region->getAttribute(XmlStr("timezone"));
            int timezone, shift = 0;
            sscanf(stimezone.c_str(),"%d,%n",&timezone,&shift);
            if (shift == 0) {
                throw InfosmeException("invalid timezone='%s' for region id=%u",
                                       stimezone.c_str(),rid);
            } else if ( timezone/3600*3600 != timezone ) {
                throw InfosmeException("invalid timezone=%d, it does not divisable by 3600 w/o remainder");
            }

            Region* r = new Region(rid,name.c_str(),smscId.c_str(),bandwidth,timezone,&masks);
            regions_.push_back(r);
        }

    } catch ( std::exception& e ) {
        for ( std::vector< Region* >::reverse_iterator i = regions_.rbegin();
              i != regions_.rend(); ++i ) {
            delete *i;
        }
        throw InfosmeException("cannot load regions from '%s': %s",xmlfile,e.what());
    }
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
