#include "RouteConfig.h"

#include <fstream>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <logger/Logger.h>
#include <util/cstrings.h>
#include <util/debug.h>
#include <sms/sms_const.h>

namespace smsc {
namespace util {
namespace config {
namespace route {

using smsc::util::encode;

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
Subject *RouteConfig::createSubjectDef(const DOM_Element &elem)
{
  std::auto_ptr<char> id(elem.getAttribute("id").transcode());
  DOM_NodeList maskElems = elem.getElementsByTagName("mask");
  MaskVector masks;
  for (unsigned i=0; i<maskElems.getLength(); i++)
  {
    DOM_Node node = maskElems.item(i);
    DOM_Element &mask = (DOM_Element &) node;
    masks.push_back(Mask(mask.getAttribute("value").transcode()));
  }

  return new Subject(std::string(id.get()), masks);
}

void RouteConfig::createRouteSource(const DOM_Element &srcElem, const SubjectPHash &subjects, Route * r)
throw (SubjectNotFoundException)
{
  if (srcElem.getElementsByTagName("subject").getLength() > 0)
  {
    DOM_Node subjNode = srcElem.getElementsByTagName("subject").item(0);
    DOM_Element &subjElem = (DOM_Element &) subjNode;
    std::auto_ptr<char> subId(subjElem.getAttribute("id").transcode());
    if (!subjects.Exists(subId.get()))
      throw SubjectNotFoundException();
    r->sources[subId.get()] = *subjects[subId.get()];
  }
  else
  {
    DOM_Node maskNode = srcElem.getElementsByTagName("mask").item(0);
    DOM_Element &maskElem = (DOM_Element &) maskNode;
    std::auto_ptr<char> mask(maskElem.getAttribute("value").transcode());
    r->sources[mask.get()] = Source(Mask(mask.get()));
  }
}

void RouteConfig::createRouteDestination(const DOM_Element &dstElem, const SubjectPHash &subjects, Route * r)
throw (SubjectNotFoundException)
{
  std::auto_ptr<char> smeId(dstElem.getAttribute("sme").transcode());
  if (dstElem.getElementsByTagName("subject").getLength() > 0)
  {
    DOM_Node subjNode = dstElem.getElementsByTagName("subject").item(0);
    DOM_Element &subjElem = (DOM_Element &) subjNode;
    std::auto_ptr<char> subId(subjElem.getAttribute("id").transcode());
    if (!subjects.Exists(subId.get()))
      throw SubjectNotFoundException();
    r->destinations[subId.get()] = Destination(*subjects[subId.get()],
                                               std::string(smeId.get()));
  }
  else
  {
    DOM_Node maskNode = dstElem.getElementsByTagName("mask").item(0);
    DOM_Element &maskElem = (DOM_Element &) maskNode;
    std::auto_ptr<char> mask(maskElem.getAttribute("value").transcode());
    r->destinations[mask.get()] = Destination(Mask(mask.get()),
                                              std::string(smeId.get()));
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

Route * RouteConfig::createRoute(const DOM_Element &elem, const SubjectPHash &subjects)
throw (SubjectNotFoundException)
{
  std::auto_ptr<char> id(elem.getAttribute("id").transcode());
  std::auto_ptr<char> billing(elem.getAttribute("billing").transcode());
  std::auto_ptr<char> archiving(elem.getAttribute("archiving").transcode());
  std::auto_ptr<char> enabling(elem.getAttribute("enabling").transcode());
  std::auto_ptr<char> suppressDeliveryReports(elem.getAttribute("suppressDeliveryReports").transcode());
  std::auto_ptr<char> active(elem.getAttribute("active").transcode());
  std::auto_ptr<char> hide(elem.getAttribute("hide").transcode());
  std::auto_ptr<char> forceRP(elem.getAttribute("forceReplyPath").transcode());
  std::auto_ptr<char> priorityStr(elem.getAttribute("priority").transcode());
  std::auto_ptr<char> serviceIdStr(elem.getAttribute("serviceId").transcode());
  unsigned int priority = atoi(priorityStr.get());
  unsigned int serviceId = atoi(serviceIdStr.get());
  std::auto_ptr<char> srcSmeSystemId(elem.getAttribute("srcSmeId").transcode());
  std::auto_ptr<char> deliveryModeStr(elem.getAttribute("deliveryMode").transcode());
  std::auto_ptr<char> forwardToStr(elem.getAttribute("forwardTo").transcode());

  std::auto_ptr<Route> r(new Route(std::string(id.get()),
                                   priority,
                                   strcmp("true", billing.get()) == 0,
                                   strcmp("true", archiving.get()) == 0,
                                   strcmp("true", enabling.get()) == 0,
                                   strcmp("true", suppressDeliveryReports.get()) == 0,
                                   strcmp("true", active.get()) == 0,
                                   strcmp("false", hide.get()) != 0,
                                   strcmp("true", forceRP.get()) == 0,
                                   serviceId,
                   std::string(srcSmeSystemId.get()),
                   strToDeliveryMode(deliveryModeStr.get()),
                   std::string(forwardToStr.get()))
                         );

  DOM_NodeList srcs = elem.getElementsByTagName("source");
  for (unsigned i=0; i<srcs.getLength(); i++)
  {
    DOM_Node node = srcs.item(i);
    DOM_Element &srcElem = (DOM_Element &)node;
    createRouteSource(srcElem, subjects, r.get());
  }

  DOM_NodeList dsts = elem.getElementsByTagName("destination");
  for (unsigned i=0; i<dsts.getLength(); i++)
  {
    DOM_Node node = dsts.item(i);
    DOM_Element &dstElem = (DOM_Element &)node;
    createRouteDestination(dstElem, subjects, r.get());
  }

  return r.release();
}

RouteConfig::status RouteConfig::load(const char * const filename)
{
  config_filename.reset(cStringCopy(filename));
  try
  {
    DOM_Document document = reader.read(filename);
    DOM_Element elem = document.getDocumentElement();
    DOM_NodeList subj_defs = elem.getElementsByTagName("subject_def");
    // Subjects
    for (unsigned i=0; i<subj_defs.getLength(); i++)
    {
      DOM_Node node(subj_defs.item(i));
      DOM_Element &elem2 = (DOM_Element &)node;
      Subject *s = createSubjectDef(elem2);
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
    DOM_NodeList route_list = elem.getElementsByTagName("route");
    for (unsigned i=0; i<route_list.getLength(); i++)
    {
      DOM_Node node(route_list.item(i));
      DOM_Element &elem2 = (DOM_Element &) node;
      try
      {
        std::auto_ptr<Route> route(createRoute(elem2, subjects));
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
      << "\" billing=\""   << (r->isBilling() ? "true" : "false")
      << "\" archiving=\"" << (r->isArchiving() ? "true" : "false")
      << "\" enabling=\""  << (r->isEnabling() ? "true" : "false")
      << "\" priority=\""  << r->getPriority()
      << "\" serviceId=\""  << r->getServiceId()
      << "\" deliveryMode=\""  << deliveryModeToStr(r->getDeliveryMode())
      << "\" forwardTo=\""  << r->getForwardTo()
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
