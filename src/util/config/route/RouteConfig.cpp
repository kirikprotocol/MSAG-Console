#include "RouteConfig.h"

#include <fstream>
#include <xercesc/dom/DOM.hpp>
#include "util/xml/utilFunctions.h"
#include "logger/Logger.h"
#include "util/cstrings.h"
#include "util/debug.h"
#include "sms/sms_const.h"
#include "util/xml/DOMTreeReader.h"

namespace smsc {
namespace util {
namespace config {
namespace route {

using smsc::util::encode;
using namespace xercesc;
using namespace smsc::util::xml;

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
: logger(smsc::logger::Logger::getInstance("smsc.util.config.route.RouteConfig"))
{
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
  DOMNodeList *maskElems = elem.getElementsByTagName(XmlStr("mask"));
  MaskVector masks;
  unsigned maskElemsLength = maskElems->getLength();
  for (unsigned i=0; i<maskElemsLength; i++)
  {
    DOMElement *mask = (DOMElement *) maskElems->item(i);
    masks.push_back(Mask(XmlStr(mask->getAttribute(XmlStr("value")))));
  }

  return new Subject(XmlStr(elem.getAttribute(XmlStr("id"))).c_str(), masks);
}

void RouteConfig::createRouteSource(const DOMElement &srcElem, const SubjectPHash &subjects, Route * r)
throw (SubjectNotFoundException)
{
  if (srcElem.getElementsByTagName(XmlStr("subject"))->getLength() > 0)
  {
    DOMElement *subjElem = (DOMElement *) srcElem.getElementsByTagName(XmlStr("subject"))->item(0);
    XmlStr subId(subjElem->getAttribute(XmlStr("id")));
    if (!subjects.Exists(subId))
      throw SubjectNotFoundException();
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
      throw SubjectNotFoundException();
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
  if (::stricmp("store", deliveryModeStr) == 0)
    return smsc::sms::SMSC_STOREANDFORWARD_MSG_MODE;
  if (::stricmp("forward", deliveryModeStr) == 0)
    return smsc::sms::SMSC_TRANSACTION_MSG_MODE;
  if (::stricmp("datagram", deliveryModeStr) == 0)
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

const Route::ReplyPath strToReplyPath(const char * const replyPathStr)
{
  if (::stricmp("pass", replyPathStr) == 0)     return smsc::util::config::route::Route::REPLY_PATH_PASS;
  if (::stricmp("force", replyPathStr) == 0)    return smsc::util::config::route::Route::REPLY_PATH_FORCE;
  if (::stricmp("SUPPRESS", replyPathStr) == 0) return smsc::util::config::route::Route::REPLY_PATH_SUPPRESS;
  return smsc::util::config::route::Route::REPLY_PATH_PASS;
}

const char * const replyPathToStr(const Route::ReplyPath replyPath)
{
  switch(replyPath)
  {
    case smsc::util::config::route::Route::REPLY_PATH_PASS:     return "pass";
    case smsc::util::config::route::Route::REPLY_PATH_FORCE:    return "force";
    case smsc::util::config::route::Route::REPLY_PATH_SUPPRESS: return "suppress";
    default:                                                    return "pass";
  }
}

Route * RouteConfig::createRoute(const DOMElement &elem, const SubjectPHash &subjects)
throw (SubjectNotFoundException)
{
  XmlStr id(elem.getAttribute(XmlStr("id")));
  XmlStr billing(elem.getAttribute(XmlStr("billing")));
  XmlStr archiving(elem.getAttribute(XmlStr("archiving")));
  XmlStr enabling(elem.getAttribute(XmlStr("enabling")));
  XmlStr suppressDeliveryReports(elem.getAttribute(XmlStr("suppressDeliveryReports")));
  XmlStr active(elem.getAttribute(XmlStr("active")));
  XmlStr hide(elem.getAttribute(XmlStr("hide")));
  XmlStr forceRP(elem.getAttribute(XmlStr("replyPath")));
  //XmlStr priorityStr(elem.getAttribute(XmlStr("priority")));
  //XmlStr serviceIdStr(elem.getAttribute(XmlStr("serviceId")));
  unsigned int priority = atoi(XmlStr(elem.getAttribute(XmlStr("priority"))));
  unsigned int serviceId = atoi(XmlStr(elem.getAttribute(XmlStr("serviceId"))));
  XmlStr srcSmeSystemId(elem.getAttribute(XmlStr("srcSmeId")));
  XmlStr deliveryModeStr(elem.getAttribute(XmlStr("deliveryMode")));
  XmlStr forwardToStr(elem.getAttribute(XmlStr("forwardTo")));
  XmlStr trafRulesStr(elem.getAttribute(XmlStr("trafficRules")));
  const AclIdent aclId(atol(XmlStr(elem.getAttribute(XmlStr("aclId")))));
  XmlStr forceDelivery(elem.getAttribute(XmlStr("forceDelivery")));

  std::auto_ptr<Route> r(new Route(std::string(id),
                                   priority,
                                   strcmp("true", billing) == 0,
                                   strcmp("true", archiving) == 0,
                                   strcmp("true", enabling) == 0,
                                   strcmp("true", suppressDeliveryReports) == 0,
                                   strcmp("true", active) == 0,
                                   strcmp("false", hide) != 0,
                                   strToReplyPath(forceRP),
                                   serviceId,
                                   std::string(srcSmeSystemId),
                                   strToDeliveryMode(deliveryModeStr),
                                   std::string(forwardToStr),
                                   std::string(trafRulesStr),
                                   aclId,
                                   strcmp("true", forceDelivery) == 0)
                         );

  DOMNodeList *srcs = elem.getElementsByTagName(XmlStr("source"));
  unsigned srcsLength = srcs->getLength();
  for (unsigned i=0; i<srcsLength; i++)
  {
    DOMElement *srcElem = (DOMElement *)srcs->item(i);
    createRouteSource(*srcElem, subjects, r.get());
  }

  DOMNodeList *dsts = elem.getElementsByTagName(XmlStr("destination"));
  unsigned dstsLength = dsts->getLength();
  for (unsigned i=0; i<dstsLength; i++)
  {
    DOMElement *dstElem = (DOMElement *)dsts->item(i);
    createRouteDestination(*dstElem, subjects, r.get());
  }

  return r.release();
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
    unsigned subj_defsLength = subj_defs->getLength();
    for (unsigned i=0; i<subj_defsLength; i++)
    {
      DOMElement *elem2 = (DOMElement *)subj_defs->item(i);
      Subject *s = createSubjectDef(*elem2);
      if (subjects.Exists(s->getId()))
      {
        smsc_log_warn(logger, "Duplicate of subject \"%s\" definition. Second subject definition skipped", s->getId());
      }
      else
      {
        subjects[s->getId()] = s;
      }
    }

    // routes
    DOMNodeList *route_list = elem->getElementsByTagName(XmlStr("route"));
    unsigned route_listLength = route_list->getLength();
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
  catch (ParseException &e)
  {
    return fail;
  }
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
      out << "  <route id=\""  << encode(r->getId())
      << "\" billing=\""       << (r->isBilling() ? "true" : "false")
      << "\" archiving=\""     << (r->isArchiving() ? "true" : "false")
      << "\" enabling=\""      << (r->isEnabling() ? "true" : "false")
      << "\" priority=\""      << r->getPriority()
      << "\" serviceId=\""     << r->getServiceId()
      << "\" deliveryMode=\""  << deliveryModeToStr(r->getDeliveryMode())
      << "\" forwardTo=\""     << r->getForwardTo()
      << "\" aclId=\""         << r->getAclId()
      << "\" forceDelivery=\"" << (r->isForceDelivery() ? "true" : "false")
      << "\" replyPath=\""    << replyPathToStr(r->getReplyPath())
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

RouteConfig::RouteIterator RouteConfig::getRouteIterator() const
{
  return RouteIterator(routes);
}

}
}
}
}
