#include "RouteConfig.h"

#include <list>
#include <fstream>
#include <xercesc/dom/DOM.hpp>
#include "util/xml/utilFunctions.h"
#include "logger/Logger.h"
#include "util/cstrings.h"
#include "util/debug.h"
#include "sms/sms_const.h"
#include "util/xml/DOMTreeReader.h"

namespace scag {
namespace config {

using smsc::util::encode;
using namespace xercesc;
using namespace smsc::util::xml;

smsc::logger::Logger* RouteConfig::logger = 0;

RouteConfig::RouteIterator::RouteIterator(RoutePVector const &records_vector)
{
  iter = records_vector.begin();
  end = records_vector.end();
}

RouteConfig::status RouteConfig::RouteIterator::fetchNext(Route *&record)
{
  if (hasRecord())
  {
    record = *iter;
    iter++;
    return success;
  }
  else
  {
    return fail;
  }
}

RouteConfig::RouteConfig()
{
  if ( !logger )
    logger = smsc::logger::Logger::getInstance("smsc.util.config.route.RouteConfig");
  log_ = logger;
}

RouteConfig::~RouteConfig()
{
  clear();
}

void RouteConfig::clear()
{
  for (size_t i = 0; i < routes.size(); i++)
  {
    delete routes[i];
  }
  routes.clear();

  char * key;
  Subject * value;
  for (SubjectPHash::Iterator j = subjects.getIterator(); j.Next(key, value); )
  {
    delete value;
  }
  subjects.Empty();
}

/*RouteConfig::status RouteConfig::putRoute(Route *record)
{
  routes.push_back(record);
  return success;
}

RouteConfig::status RouteConfig::putSubject(Subject *subj)
{
  subjects.Insert(subj->getId(), subj);
  return success;
}
*/
Subject *RouteConfig::createSubjectDef(const DOMElement &elem)
{
/*
  DOMNodeList *maskElems = elem.getElementsByTagName(XmlStr("mask"));
  MaskVector masks;
  unsigned maskElemsLength = unsigned(maskElems->getLength());
  for (unsigned i=0; i<maskElemsLength; i++)
  {
    DOMElement *mask = (DOMElement *) maskElems->item(i);
    masks.push_back(Mask(XmlStr(mask->getAttribute(XmlStr("value")))));
  }

  Subject* ret = new Subject(XmlStr(elem.getAttribute(XmlStr("id"))).c_str(), masks);
  smsc_log_debug(logger,"subject loaded: '%s'",ret->getId());
  return ret;
*/
  DOMNodeList *maskElems = elem.getElementsByTagName(XmlStr("mask"));
  MaskVector masks;
  size_t maskElemsLength = maskElems->getLength();
  for (size_t i=0; i<maskElemsLength; i++)
  {
    DOMElement *mask = (DOMElement *) maskElems->item(i);
    masks.push_back(Mask(XmlStr(mask->getAttribute(XmlStr("value")))));
  }
  MaskVector refs;
  DOMNodeList *subjElems = elem.getElementsByTagName(XmlStr("subject"));
  size_t subjElemsLength = subjElems->getLength();
  for(size_t i=0;i<subjElemsLength;i++)
  {
    DOMElement *subj = (DOMElement *) subjElems->item(i);
    XmlStr subjId(subj->getAttribute(XmlStr("id")));
    refs.push_back(subjId.c_str());
  }

  return new Subject(XmlStr(elem.getAttribute(XmlStr("id"))).c_str(), masks,refs);
}

void RouteConfig::createRouteSource(const DOMElement &srcElem, const SubjectPHash &subjects, Route * r)
throw (SubjectNotFoundException)
{
  if (srcElem.getElementsByTagName(XmlStr("subject"))->getLength() > 0)
  {
    DOMElement *subjElem = (DOMElement *) srcElem.getElementsByTagName(XmlStr("subject"))->item(0);
    XmlStr subId(subjElem->getAttribute(XmlStr("id")));
    if (!subjects.Exists(subId))
      throw SubjectNotFoundException(/*subId.c_str(),r->getId()*/);
    r->sources[subId] = *subjects[subId];
  }
  else
  {
    DOMElement *maskElem = (DOMElement *) srcElem.getElementsByTagName(XmlStr("mask"))->item(0);
    XmlStr mask(maskElem->getAttribute(XmlStr("value")));
    r->sources[mask] = Source(Mask(mask));
  }
}

void RouteConfig::createRouteDestination(const DOMElement &dstElem, const SubjectPHash &subjects, Route * r)
throw (SubjectNotFoundException)
{
  XmlStr smeId(dstElem.getAttribute(XmlStr("sme")));
  if (dstElem.getElementsByTagName(XmlStr("subject"))->getLength() > 0)
  {
    DOMElement *subjElem = (DOMElement *) dstElem.getElementsByTagName(XmlStr("subject"))->item(0);
    XmlStr subId(subjElem->getAttribute(XmlStr("id")));
    if (!subjects.Exists(subId))
      throw SubjectNotFoundException(/*subId.c_str(),r->getId()*/);
    r->destinations[subId] = Destination(*subjects[subId], smeId.c_str());
  }
  else
  {
    DOMElement *maskElem = (DOMElement *) dstElem.getElementsByTagName(XmlStr("mask"))->item(0);
    XmlStr mask(maskElem->getAttribute(XmlStr("value")));
    r->destinations[mask] = Destination(Mask(mask), smeId.c_str());
  }
}

const uint8_t strToDeliveryMode(const char * const deliveryModeStr)
{
  if (strcasecmp("store", deliveryModeStr) == 0)
    return smsc::sms::SMSC_STOREANDFORWARD_MSG_MODE;
  if (strcasecmp("forward", deliveryModeStr) == 0)
    return smsc::sms::SMSC_TRANSACTION_MSG_MODE;
  if (strcasecmp("datagram", deliveryModeStr) == 0)
    return smsc::sms::SMSC_DATAGRAM_MSG_MODE;
  return smsc::sms::SMSC_DEFAULT_MSG_MODE;
}

const char * const deliveryModeToStr(const uint8_t deliveryMode)
{
  switch (deliveryMode)
  {
  case smsc::sms::SMSC_STOREANDFORWARD_MSG_MODE:
    return "store";
  case smsc::sms::SMSC_TRANSACTION_MSG_MODE:
    return "forward";
  case smsc::sms::SMSC_DATAGRAM_MSG_MODE:
    return "datagram";
  default:
    return "default";
  }
}

/*const scag::transport::smpp::router::ReplyPath strToReplyPath(const char * const replyPathStr)
{
  if (::strcasecmp("pass", replyPathStr) == 0)     return scag::transport::smpp::router::ReplyPathPass;
  if (::strcasecmp("force", replyPathStr) == 0)    return scag::transport::smpp::router::ReplyPathForce;
  if (::strcasecmp("SUPPRESS", replyPathStr) == 0) return scag::transport::smpp::router::ReplyPathSuppress;
  return scag::transport::smpp::router::ReplyPathPass;
}

const char * const replyPathToStr(const scag::transport::smpp::router::ReplyPath replyPath)
{
  switch(replyPath)
  {
    case scag::transport::smpp::router::ReplyPathPass:     return "pass";
    case scag::transport::smpp::router::ReplyPathForce:    return "force";
    case scag::transport::smpp::router::ReplyPathSuppress: return "suppress";
    default:                              return "pass";
  }
}*/

/*
bool RouteConfig::getAttribBool(const DOMElement &elem, const char * name)
{
    XmlStr attrib(elem.getAttribute(XmlStr(name)));
    return (strcmp(attrib, "true") == 0);
}

int RouteConfig::getAttribInt(const DOMElement &elem, const char * name)
{
    XmlStr attrib(elem.getAttribute(XmlStr(name)));
    return atoi(attrib);
}

std::string RouteConfig::getAttribStr(const DOMElement &elem, const char * name)
{
    XmlStr attrib(elem.getAttribute(XmlStr(name)));
    return std::string(attrib);
}
*/

static bool getBoolAttr(const DOMElement &elem,const char* name)
{
  XmlStr xname(name);
  if(!elem.hasAttribute(xname))
  {
    return false;
  }
  XmlStr val(elem.getAttribute(xname));
  return val=="true";
}

static const XMLCh* getAttr(const DOMElement& elem,const char* name)
{
  static XMLCh null=0;
  XmlStr xname(name);
  if(!elem.hasAttribute(xname))
  {
    return &null;
  }
  return elem.getAttribute(xname);
}

static int getIntAttr(const DOMElement& elem,const char* name,int defaultValue=0)
{
  XmlStr xname(name);
  if(!elem.hasAttribute(xname))
  {
    return defaultValue;
  }
  return atoi(XmlStr(elem.getAttribute(xname)));
}


Route * RouteConfig::createRoute(const DOMElement &elem, const SubjectPHash &subjects)
throw (SubjectNotFoundException)
{


  std::string routeid(XmlStr(getAttr(elem, "id")));
  if(routeid.length()>32)
  {
    __warning2__("\n\n\nROUTE ID TOO LONG=%s\n\n\n",routeid.c_str());
    routeid.erase(32);
  }

  std::auto_ptr<Route> r(new Route( std::string(XmlStr(getAttr(elem, "id"))),
                                    false, // getAttribBool(elem, "archived"),
                                    getBoolAttr(elem, "enabled"),
                                    getBoolAttr(elem, "active") /*not used*/,
                                    getBoolAttr(elem, "transit"),
                                    getBoolAttr(elem, "saa"),
                                    getBoolAttr(elem, "hideSaaText"),
                                    std::string(XmlStr(getAttr(elem, "srcSmeId"))),
                                    getIntAttr(elem, "serviceId"),
                                    std::string(XmlStr(getAttr(elem, "slicing"))),
                                    std::string(XmlStr(getAttr(elem, "slicedRespPolicy"))) )
                         );
  smsc_log_debug(logger,"loading route '%s'",r->getId());


  DOMNodeList *srcs = elem.getElementsByTagName(XmlStr("source"));
  unsigned srcsLength = unsigned(srcs->getLength());
  for (unsigned i=0; i<srcsLength; i++)
  {
    DOMElement *srcElem = (DOMElement *)srcs->item(i);
    createRouteSource(*srcElem, subjects, r.get());
  }

  DOMNodeList *dsts = elem.getElementsByTagName(XmlStr("destination"));
  unsigned dstsLength = unsigned(dsts->getLength());
  for (unsigned i=0; i<dstsLength; i++)
  {
    DOMElement *dstElem = (DOMElement *)dsts->item(i);
    createRouteDestination(*dstElem, subjects, r.get());
  }

  return r.release();
}


void RouteConfig::expandSubject(Subject& subj, subjNameSet& names)
{
  smsc_log_debug(log_,"Expanding subject:%s",subj.getId());
  std::pair<subjNameSet::iterator,bool> insertResult = names.insert(subj.getId());
  if (!insertResult.second)
  {
    std::string branch;
    for (subjNameSet::iterator it=names.begin(); it!=names.end(); ++it)
    {
      branch.append(*it);
      branch.append("-->");
    }
    branch.append(subj.getId());
    smsc_log_info(log_,"Subject tree cyclic alert: %s", branch.c_str());
    return;
  }
  while(!subj.getSubjRefs().empty())
  {
    std::string id=subj.getSubjRefs().back();
    subj.getSubjRefs().pop_back();
    smsc_log_debug(log_,"Found referenced subj:%s",id.c_str());
    try{
      Subject& refS=getSubject(id.c_str());
      if(!refS.getSubjRefs().empty())
      {
        smsc_log_debug(log_,"Referenced subj isn't expanded yet. Expanding");
        expandSubject(refS, names);
      }
      subj.getMasks().insert(subj.getMasks().end(),refS.getMasks().begin(),refS.getMasks().end());
    }catch(...)
    {
      smsc_log_warn(log_,"Referenced subject not found:%s",id.c_str());
    }
  }
  names.erase(insertResult.first);
}

RouteConfig::status RouteConfig::load(const char * const filename)
{
  config_filename.reset(cStringCopy(filename));
  try
  {
    DOMTreeReader reader;
    DOMDocument *document = reader.read(filename);
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *subj_defs = elem->getElementsByTagName(XmlStr("subject_def"));
    // Subjects
    size_t subj_defsLength = subj_defs->getLength();

    std::list<Subject*> subjs;
    subjNameSet subjNames;

    for (size_t i=0; i<subj_defsLength; i++)
    {
      DOMElement *elem2 = (DOMElement *)subj_defs->item(i);
      Subject *s = createSubjectDef(*elem2);
      if (subjects.Exists(s->getId()))
      {
        smsc_log_warn(logger, "Duplicate of subject \"%s\" definition. Second subject definition skipped", s->getId());
      }
      else
      {
//        subjects[s->getId()] = s;
        subjects.Insert(s->getId(),s);
        subjs.push_back(s);
      }
    }

    // expand subjrefs
    for(std::list<Subject*>::iterator it=subjs.begin();it!=subjs.end();it++)
    {
      Subject& subj=**it;
      expandSubject(subj, subjNames);
      std::set<Mask> ms;
      MaskVector& mv=subj.getMasks();
      for(MaskVector::iterator mit=mv.begin();mit!=mv.end();mit++)
      {
        ms.insert(*mit);
      }
      size_t before=mv.size();
      mv.clear();
      for(std::set<Mask>::iterator sit=ms.begin();sit!=ms.end();sit++)
      {
        mv.push_back(*sit);
      }
      smsc_log_debug(logger,"Unique mask pack for '%s': %d->%d",subj.getId(),before,mv.size());
    }

    // routes
    DOMNodeList *route_list = elem->getElementsByTagName(XmlStr("route"));
    unsigned route_listLength = unsigned(route_list->getLength());
    for (unsigned i=0; i<route_listLength; i++)
    {
      DOMElement *elem2 = (DOMElement *) route_list->item(i);
      try
      {
        std::auto_ptr<Route> route(createRoute(*elem2, subjects));
        if (route->isActive())
          routes.push_back(route.release());
      }
      catch (SubjectNotFoundException &ex)
      {
        smsc_log_error(logger, "incorrect subject id: subject not defined");
      }
    }
  }
  catch (SmscParseException &e)
  {
    return fail;
  }
  smsc_log_info(logger, "Smpp routes loaded successfully");  
  return success;
}

RouteConfig::status RouteConfig::reload()
{
  __trace__("smsc::util::config::route::RouteConfig - Reload routes");
  clear();
  std::auto_ptr<char> tmp = config_filename;
  status result = load(tmp.get());
  __trace2__("smsc::util::config::route::RouteConfig - loaded %u routes, %u subjects", routes.size(), subjects.GetCount());
  return result;
}
/*
RouteConfig::status RouteConfig::store(const char * const filename) const
{
  try
  {
    std::ofstream out(filename);
    out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
    out << "<!DOCTYPE records SYSTEM \"SmeRecords.dtd\">" << std::endl;
    out << "<routes>" << std::endl;
    Subject *s;
    char *key;
    for (SubjectPHash::Iterator i = subjects.getIterator(); i.Next(key, s);)
    {
      out << "  <subject_def id=\"" << encode(s->getId()) << "\">" << std::endl;
      for (MaskVector::const_iterator j = s->getMasks().begin(); j != s->getMasks().end(); j++)
      {
        out << "    <mask value=\"" << encode(j->c_str()) << "\"/>" << std::endl;
      }
      out << "  </subject_def>" << std::endl;
    }

    for (RoutePVector::const_iterator i = routes.begin(); i != routes.end(); i++)
    {
      Route *r = *i;
      const char* slice;
        switch (r->getSlicingType()) {
        case SlicingType::SAR : slice = "SAR"; break;
        case SlicingType::UDH8 : slice = "UDH8"; break;
        case SlicingType::UDH16 : slice = "UDH16"; break;
        case SlicingType::NONE:
        default:
            slice = "NONE";
        }
        const char* slicePol;
        switch (r->getSlicingRespPolicy()) {
        case SlicingRespPolicy::ANY: slicePol = "ANY"; break;
        case SlicingRespPolicy::ALL:
        default: slicePol = "ALL"; break;
        }

      out << "  <route id=\""  << encode(r->getId())
      // << "\" archiving=\""     << (r->isArchiving() ? "true" : "false")
      << "\" enabled=\""      << (r->isActive() ? "true" : "false")
      << "\" transit=\""       << (r->isTransit() ? "true" : "false")
      << "\" saa=\""           << (r->hasStatistics() ? "true" : "false")
      << "\" hideSaaText=\""  << (r->hideMessageBody() ? "true" : "false")
      << "\" slicing=\"" << slice <<
            "\" slicedRespPolicy=\"" << slicePol <<
            "\" srcSmeId=\"" << encode(r->getSrcSmeSystemId().c_str()) <<
            "\" serviceId=\"" << r->getServiceId()
      << "\">" << std::endl;

      Source src;
      for (SourceHash::Iterator j = r->getSources().getIterator(); j.Next(key, src);)
      {
        out << "    <source>" << std::endl;
        out << "      " << (src.isSubject() ? "<subject id=\"" : "<mask value=\"") << encode(src.getId()) << "\"/>" << std::endl;
        out << "    </source>" << std::endl;
      }

      Destination d;
      for (DestinationHash::Iterator j = r->getDestinations().getIterator(); j.Next(key, d);)
      {
        out << "    <destination sme=\"" << d.getSmeId() << "\">" << std::endl;
        out << "      " << (d.isSubject() ? "<subject id=\"" : "<mask value=\"") << encode(d.getId()) << "\"/>" << std::endl;
        out << "    </destination>" << std::endl;
      }

      out << "  </route>" << std::endl;
    }
    out << "</routes>" << std::endl;
    out.flush();
    out.close();
  }
  catch (...)
  {
    return fail;
  }
  return success;
}
*/
RouteConfig::RouteIterator RouteConfig::getRouteIterator() const
{
  return RouteIterator(routes);
}

}
}
