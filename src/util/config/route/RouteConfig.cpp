#include "RouteConfig.h"

#include <fstream>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <util/Logger.h>
#include <util/cstrings.h>
#include <util/debug.h>

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
	} else {
		return fail;
	}
}

RouteConfig::RouteConfig()
	: logger(smsc::util::Logger::getCategory("smsc.util.config.route.RouteConfig"))
{
}

void RouteConfig::clear()
{
	routes.clear();
	subjects.Empty();
}

RouteConfig::status RouteConfig::putRoute(Route *record)
{
	routes.push_back(record);
	return success;
}

RouteConfig::status RouteConfig::putSubject(Subject *subj)
{
	subjects.Insert(subj->getId(), subj);
	return success;
}

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
		r->sources[mask.get()] = Subject(Mask(mask.get()));
		mask.release();
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
		mask.release();
	}
}

Route * RouteConfig::createRoute(const DOM_Element &elem, const SubjectPHash &subjects)
	throw (SubjectNotFoundException)
{
	std::auto_ptr<char> id(elem.getAttribute("id").transcode());
	std::auto_ptr<char> billing(elem.getAttribute("billing").transcode());
	std::auto_ptr<char> archiving(elem.getAttribute("archiving").transcode());
	std::auto_ptr<char> enabling(elem.getAttribute("enabling").transcode());
	std::auto_ptr<Route> r(new Route(std::string(id.get()),
																	 strcmp("true", billing.get()) == 0,
																	 strcmp("true", archiving.get()) == 0,
																	 strcmp("true", enabling.get()) == 0));
	
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
		for (unsigned i=0; i<subj_defs.getLength(); i++)
		{
			DOM_Node node(subj_defs.item(i));
			DOM_Element &elem = (DOM_Element &)node;
			Subject *s = createSubjectDef(elem);
			subjects[s->getId()] = s;
		}

		DOM_NodeList route_list = elem.getElementsByTagName("route");
		for (unsigned i=0; i<route_list.getLength(); i++)
		{
			DOM_Node node(route_list.item(i));
			DOM_Element &elem = (DOM_Element &) node;
			try
			{
				routes.push_back(createRoute(elem, subjects));
			}
			catch (SubjectNotFoundException &ex)
			{
				logger.error("incorrect subject id: subject not defined");
			}
		}
	} catch (DOMTreeReader::ParseException &e) {
		return fail;
	}
	return success;
}

RouteConfig::status RouteConfig::reload()
{
  __trace2__("smsc::util::config::route::RouteConfig - Reload routes");
  clear();
  std::auto_ptr<char> tmp = config_filename;
  load(tmp.get());
  __trace2__("smsc::util::config::route::RouteConfig - loaded %u routes, %u subjects", routes.size(), subjects.GetCount());
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
			    << "\">" << std::endl;
			
			Source s;
			for (SourceHash::Iterator j = r->getSources().getIterator(); j.Next(key, s);)
			{
				out << "    <source>" << std::endl;
				out << "      " << (s.isSubject() ? "<subject id=\"" : "<mask value=\"") << encode(s.getId()) << "\"/>" << std::endl;
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
	catch(...)
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
