#include <memory>
#include <vector>
#include <algorithm> // sort
#include <xercesc/dom/DOM.hpp>
#include "ConfigReader.h"
#include "util/config/ConfString.h"
#include "scag/counter/TemplateManager.h"
#include "util/xml/utilFunctions.h"
#include "util/xml/DOMTreeReader.h"

namespace {

const char* sysPrefix = "sys.";
const size_t sysPrefixLen = strlen(sysPrefix);

using smsc::util::xml::XmlStr;
using namespace scag2::counter;

int64_t getAttrInt( xercesc::DOMElement* elem, const char* tag )
{
    XmlStr str(elem->getAttribute(XmlStr(tag)));
    char* endptr;
    const int64_t val = strtoll(str.c_str(),&endptr,10);
    if ( endptr && *endptr == '\0' ) return val;
    char buf[100];
    snprintf(buf,sizeof(buf),"wrong value of parameter '%s' val='%s'",tag, str.c_str());
    throw smsc::util::xml::ParseException(buf);
}

std::string getAttrStr( xercesc::DOMElement* elem, const char* tag )
{
    return XmlStr(elem->getAttribute(XmlStr(tag))).c_str();
}

ActionList getActionList( xercesc::DOMElement* elem )
{
    const char* limitsTag = "limits";
    xercesc::DOMNodeList* limitsList = elem->getElementsByTagName(XmlStr(limitsTag));
    if (limitsList->getLength()!=1)
        throw smsc::util::xml::ParseException("wrong number of limits subsections");
    xercesc::DOMElement* limits = static_cast<xercesc::DOMElement*>(limitsList->item(0));
    const int theMin = getAttrInt(limits,"min");
    const int theMax = getAttrInt(limits,"max");

    const char* limitTag = "limit";
    xercesc::DOMNodeList* limitList = elem->getElementsByTagName(XmlStr(limitTag));
    std::vector< ActionLimit > limitVector;
    for ( unsigned i = 0; i < limitList->getLength(); ++i ) {
        xercesc::DOMElement* limit = static_cast<xercesc::DOMElement*>(limitList->item(i));
        const CntSeverity sev = stringToSeverity(getAttrStr(limit,"severity").c_str());
        const int64_t percent = getAttrInt(limit,"percent");
        const int value = theMin + (theMax-theMin)*percent/100;
        const CntOpType optype = stringToOpType(getAttrStr(limit,"op").c_str());
        limitVector.push_back( ActionLimit(value,optype,sev) );
    }
    std::sort(limitVector.begin(),limitVector.end());
    ActionList ret;
    for ( std::vector< ActionLimit >::reverse_iterator i = limitVector.rbegin();
          i != limitVector.rend(); ++i ) {
        ret.push_back(*i);
    }
    return ret;
}


std::map< std::string, int > getParamIntMap( xercesc::DOMNodeList* plist )
{
    std::map<std::string,int> res;
    if (!plist) return res;
    const unsigned plen = plist->getLength();
    for ( unsigned i = 0; i < plen; ++i ) {
        xercesc::DOMElement* elt = static_cast<xercesc::DOMElement*>(plist->item(i));
        const std::string pname = getAttrStr(elt,"name");
        if ( res.find(pname) != res.end() ) {
            char buf[100];
            snprintf(buf,sizeof(buf),"duplicate param '%s' in counter template", pname.c_str());
            throw smsc::util::xml::ParseException(buf);
        }
        const std::string ptype = getAttrStr(elt,"type");
        if ( ptype == "int" ) {
            smsc::util::config::ConfString s(smsc::util::xml::getNodeText(*elt));
            char* endptr;
            int val = strtoll(s.c_str(),&endptr,10);
            if ( endptr && *endptr == '\0' ) {
                res.insert(std::make_pair(pname,val));
            } else {
                char buf[100];
                snprintf(buf,sizeof(buf),"invalid value '%s' of param '%s' in counter template",
                         s.c_str(), pname.c_str());
                throw smsc::util::xml::ParseException(buf);
            }
        }
    }
    return res;
}


} // namespace


namespace scag2 {
namespace counter {
namespace impl {

bool ConfigReader::readConfig( const char* fname )
{
    limitMap_.clear();
    protoMap_.clear();
    if (!fname) return false;
    try {

        smsc::util::xml::DOMTreeReader reader;
        xercesc::DOMDocument* document = reader.read(fname);
        xercesc::DOMElement* elem = document->getDocumentElement();
        // reading limits
        const char* limitTag = "ca_table";
        xercesc::DOMNodeList* elist = elem->getElementsByTagName(XmlStr(limitTag));
        unsigned elistLen = elist ? elist->getLength() : 0;
        for ( unsigned i = 0; i < elistLen; ++i ) {
            xercesc::DOMElement* table = static_cast<xercesc::DOMElement*>(elist->item(i));
            const std::string tableName = getAttrStr(table,"id");
            if ( limitMap_.find(tableName) != limitMap_.end() ) {
                char buf[100];
                snprintf(buf,sizeof(buf),"duplicate %s '%s'", limitTag, tableName.c_str());
                throw smsc::util::xml::ParseException(buf);
            }
            ActionList actionList = getActionList(table);
            limitMap_.insert(std::make_pair(tableName,actionList));
        }

        // reading templates
        const char* templateTag = "template";
        elist = elem->getElementsByTagName(XmlStr(templateTag));
        elistLen = elist ? elist->getLength() : 0;
        for ( unsigned i = 0; i < elistLen; ++i ) {
            xercesc::DOMElement* tmpl = static_cast<xercesc::DOMElement*>(elist->item(i));
            const std::string tmplName = getAttrStr(tmpl,"id");
            if ( protoMap_.find(tmplName) != protoMap_.end() ) {
                char buf[100];
                snprintf(buf,sizeof(buf),"duplicate template '%s'",tmplName.c_str());
                throw smsc::util::xml::ParseException(buf);
            }
            const std::string ctypeStr = getAttrStr(tmpl,"type");
            const CountType ctype = stringToCountType(ctypeStr.c_str());
            if ( ctype == TYPEUNKNOWN ) {
                char buf[100];
                snprintf(buf,sizeof(buf),"unknown type '%s' for counter template '%s'",
                         ctypeStr.c_str(),tmplName.c_str());
                throw smsc::util::xml::ParseException(buf);
            }

            // reading params
            int param0 = 0, param1 = 0;
            xercesc::DOMNodeList* sublist = tmpl->getElementsByTagName(XmlStr("param"));
            typedef std::map<std::string,int> PMap;
            const PMap paramMap(getParamIntMap(sublist));
            if ( ctype == TYPEAVERAGE || ctype == TYPETIMESNAPSHOT ) {
                PMap::const_iterator ip = paramMap.find("nseconds");
                if ( ip == paramMap.end() ) {
                    char buf[100];
                    snprintf(buf,sizeof(buf),"missing parameter 'nseconds' for template '%s'",tmplName.c_str());
                    throw smsc::util::xml::ParseException(buf);
                }
                param0 = ip->second;
                if ( ctype == TYPETIMESNAPSHOT ) {
                    ip = paramMap.find("msecresol");
                    if ( ip == paramMap.end() ) {
                        char buf[100];
                        snprintf(buf,sizeof(buf),"missing parameter 'msecresol' for template '%s'",tmplName.c_str());
                        throw smsc::util::xml::ParseException(buf);
                    }
                    param1 = ip->second;
                }
            }

            // reading limits id
            std::string limitId;
            sublist = tmpl->getElementsByTagName(XmlStr("ca"));
            const unsigned sublistLen = sublist ? sublist->getLength() : 0;
            if ( sublistLen > 1 ) {
                char buf[100];
                snprintf(buf,sizeof(buf),"wrong number of elements 'ca' for template '%s'",tmplName.c_str());
                throw smsc::util::xml::ParseException(buf);
            } else if ( sublistLen == 1 ) {
                xercesc::DOMElement* ca = static_cast<xercesc::DOMElement*>(sublist->item(0));
                limitId = getAttrStr(ca,"id");
                if ( limitMap_.find(limitId) == limitMap_.end() ) {
                    char buf[100];
                    snprintf(buf,sizeof(buf),"unknown ca id='%s' for template '%s'",
                             limitId.c_str(), tmplName.c_str());
                    throw smsc::util::xml::ParseException(buf);
                }
            }
            
            // save template proto
            TemplateProto proto;
            proto.countType = ctype;
            proto.limitName = limitId;
            proto.param0 = param0;
            proto.param1 = param1;
            protoMap_.insert(std::make_pair(tmplName,proto));

        } // loop over templates

    } catch ( std::exception& e ) {
        smsc_log_error( log_,"exc: %s", e.what() );
        limitMap_.clear();
        protoMap_.clear();
        return false;
    }

    return true;

    /*
    smsc_log_info(log_,"reloading counter config");
    const char* sysPrefix = "sys.";
    const size_t sysPrefixLen = strlen(sysPrefix);
    std::auto_ptr<scag2::config::ConfigView> limits(view.getSubConfig("limits"));
    std::auto_ptr<scag2::config::ConfigView> templates(view.getSubConfig("templates"));
    bool ok = true;
    while ( limits.get() ) {
        // loading limits
        std::auto_ptr<CStrSet> sections(limits->getShortSectionNames());
        if (!sections.get()) {
            smsc_log_warn(log_,"cannot read 'limits' subsections");
            break;
        }
        for ( CStrSet::iterator i = sections->begin(); i != sections->end(); ++i ) {
            std::auto_ptr<ConfigView> subc(view.getSubConfig(i->c_str()));
            if (!subc.get()) {
                smsc_log_warn(log_,"cannot read limit subsection '%s'",i->c_str());
                continue;
            }
            std::pair<LimitMap::iterator,bool> insres
                ( limitMap_.insert(std::make_pair(*i,ActionList())) );
            if (!insres.second) {
                smsc_log_error(log_,"cannot insert limit '%s'",i->c_str());
                continue;
            }
            LimitMap::iterator ip = insres.first;
            std::auto_ptr<CStrSet> asect(subc->getShortSectionNames());
            if (!asect.get()) {
                smsc_log_warn(log_,"cannot read limit '%s' actions",i->c_str());
                continue;
            }
            // reading max value
            unsigned maxval = 0;
            try {
                maxval = unsigned(subc->getInt("maxvalue"));
            } catch (...) {}
            // reading actions
            for ( CStrSet::iterator j = asect->begin(); j != asect->end(); ++j ) {
                std::auto_ptr<ConfigView> psect(subc->getSubConfig(j->c_str()));
                if (!psect.get()) {
                    smsc_log_error(log_,"cannot read param '%s' of action '%s'",j->c_str(),i->c_str());
                    ok = false;
                    break;
                }
                
                CntOpType   optype = GT;
                CntSeverity severity;
                int64_t     limit;
                try {
                    smsc::util::config::ConfString optypeStr(psect->getString("optype"));
                    if (optypeStr.str() == "gt") optype = GT;
                    else if (optypeStr.str() == "lt") optype = LT;
                    else {
                        smsc_log_error(log_,"parameter optype of '%s' of action '%s' must be \"gt\" or \"lt\"",
                                       j->c_str(), i->c_str());
                        ok = false;
                        break;
                    }
                } catch (...) { optype = GT; }
                try {
                    limit = psect->getInt("limit");
                } catch (...) {
                    smsc_log_error(log_,"cannot read limit of '%s' of action '%s'",
                                   j->c_str(),i->c_str());
                    ok = false;
                    break;
                }
                try {
                    smsc::util::config::ConfString sevStr(psect->getString("severity"));
                    severity = stringToSeverity(sevStr.c_str());
                    if (severity==UNKNOWN) {
                        smsc_log_error(log_,"unknown severity '%s' for '%s' of action '%s'",
                                       sevStr.c_str(),j->c_str(),i->c_str());
                        ok = false;
                        break;
                    }
                } catch (...) {
                    smsc_log_error(log_,"cannot read severity of '%s' of action '%s'",
                                   j->c_str(),i->c_str());
                    ok = false;
                    break;
                }

                // adding a new limit to the action
                ip->second.push_back( ActionLimit(limit,optype,severity) );

            }
            if (!ok) break;
        } // loop over all for action lists
        break;
    }

    if (!ok) {
        limitMap_.clear();
        return false;
    }

    while (templates.get()) {
        // reading templates
        std::auto_ptr<CStrSet> sections(templates->getShortSectionNames());
        if (!sections.get()) {
            smsc_log_warn(log_,"cannot read 'templates' subsections");
            break;
        }

        for ( CStrSet::iterator i = sections->begin(); i != sections->end(); ++i ) {
            if ( 0 == strncmp(sysPrefix,i->c_str(),sysPrefixLen) ) {
                smsc_log_error(log_,"template prefix '%s' is reserved",sysPrefix);
                ok = false;
                break;
            }
            std::auto_ptr<ConfigView> subc(view.getSubConfig(i->c_str()));
            if (!subc.get()) {
                smsc_log_warn(log_,"cannot read template counter '%s'",i->c_str());
                continue;
            }
            TemplateProto proto;
            try {
                smsc::util::config::ConfString countTypeStr(subc->getString("type"));
                proto.countType = stringToCountType(countTypeStr.c_str());
                if (proto.countType == TYPEUNKNOWN) {
                    smsc_log_error(log_,"unknown type '%s' of counter '%s'",
                                   countTypeStr.c_str(),i->c_str());
                    ok = false;
                    break;
                }
            } catch (...) {
                smsc_log_error(log_,"type of counter '%s' cannot be read",i->c_str());
                ok = false;
                break;
            }
            try {
                smsc::util::config::ConfString limStr(subc->getString("limit"));
                proto.limitName = limStr.str();
            } catch (...) {
                smsc_log_warn(log_,"limit for counter '%s' is not specified, using its name",
                              i->c_str());
                proto.limitName = *i;
            }
            if (limitMap_.find(proto.limitName) == limitMap_.end()) {
                smsc_log_error(log_,"limit '%s' for counter '%s' is not defined in section limits",
                               proto.limitName.c_str(),i->c_str());
                ok = false;
                break;
            }
            // reading params according to type
            if (proto.countType!=TYPEACCUMULATOR) {
                const char* nseconds = "nseconds";
                try {
                    proto.param0 = subc->getInt(nseconds);
                    if (proto.param0<=0) {
                        smsc_log_error(log_,"wrong parameter '%s'=%d for counter '%s'",
                                       nseconds,proto.param0,i->c_str());
                        ok = false;
                        break;
                    }
                } catch (...) {
                    smsc_log_error(log_,"parameter '%s' is not specified for counter '%s'",
                                   nseconds,i->c_str());
                    ok = false;
                    break;
                }

                if (proto.countType==TYPETIMESNAPSHOT) {
                    const char* msecresol = "msecresol";
                    try {
                        proto.param1 = subc->getInt(msecresol);
                        if (proto.param1<=0) {
                            smsc_log_error(log_,"wrong parameter '%s'=%d for counter '%s'",
                                           msecresol,proto.param0,i->c_str());
                            ok = false;
                            break;
                        }
                    } catch (...) {
                        smsc_log_error(log_,"parameter '%s' is not specified for counter '%s'",
                                       msecresol,i->c_str());
                        ok = false;
                        break;
                    }
                }
            }

            // inserting
            std::pair<ProtoMap::iterator,bool> insres
                ( protoMap_.insert(std::make_pair(*i,proto)) );
            if (!insres.second) {
                smsc_log_error(log_,"cannot insert template '%s'",i->c_str());
                ok = false;
            }
        } // loop over all
        break;
    }

    return ok;
     */
}
        


void ConfigReader::reload( TemplateManager& tmgr )
{
    // everything is fine
    // deleting old contents of the template manager
    typedef std::vector< std::string > VS;
    VS oldLimits(tmgr.getObserverNames());
    for ( VS::const_iterator i = oldLimits.begin(); i != oldLimits.end(); ++i ) {
        if ( limitMap_.find(*i) == limitMap_.end() &&
             0 != strncmp(sysPrefix,i->c_str(),sysPrefixLen) ) {
            // delete not found not system limit
            smsc_log_info(log_,"removing limit '%s'",i->c_str());
            tmgr.replaceObserver(i->c_str(),0);
        }
    }
    for ( LimitMap::const_iterator i = limitMap_.begin(); i != limitMap_.end(); ++i ) {
        smsc_log_info(log_,"replacing limit '%s'",i->first.c_str());
        tmgr.replaceObserver(i->first.c_str(),
                             new ActionTable(new ActionList(i->second)));
    }
    VS oldTempl(tmgr.getTemplateNames());
    for ( VS::const_iterator i = oldTempl.begin(); i != oldTempl.end(); ++i ) {
        if ( protoMap_.find(*i) == protoMap_.end() &&
             0 != strncmp(sysPrefix,i->c_str(),sysPrefixLen) ) {
            smsc_log_info(log_,"removing templ '%s'",i->c_str());
            tmgr.replaceTemplate(i->c_str(),0);
        }
    }
    for ( ProtoMap::const_iterator i = protoMap_.begin(); i != protoMap_.end(); ++i ) {
        smsc_log_info(log_,"replacing templ '%s'",i->first.c_str());
        ObserverPtr o(tmgr.getObserver(i->second.limitName.c_str()));
        tmgr.replaceTemplate( i->first.c_str(),
                              CounterTemplate::create(i->second.countType,
                                                      o.get(),
                                                      i->second.param0,
                                                      i->second.param1));
    }
    smsc_log_info(log_,"counter config reloaded");
}

}
}
}
