#ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTECONFIG
#define SMSC_UTIL_CONFIG_ROUTE_ROUTECONFIG

#include <util/config/route/RouteStructures.h>
#include <util/xml/DOMTreeReader.h>

namespace smsc {
namespace util {
namespace config {
namespace route {

using smsc::util::xml::DOMTreeReader;

class RouteConfig {
public:
	enum status {success, fail};

	class RouteIterator
	{
	public:
		bool hasRecord() {return iter != end;}
		status fetchNext(Route *&record);
		RouteIterator(RoutePVector const &routs_vector);
	protected:
		RoutePVector::const_iterator iter;
		RoutePVector::const_iterator end;
	};

	RouteConfig();

	void clear();
	status putRoute(Route *record);
	status putSubject(Subject *subj);

	status load(const char * const filename);

  status reload();

	status store(const char * const filename) const;

	RouteIterator getRouteIterator() const;

protected:
	DOMTreeReader reader;
	log4cpp::Category &logger;
	RoutePVector routes;
	SubjectPHash subjects;
  std::auto_ptr<char> config_filename;

	class SubjectNotFoundException {};

	static Subject * createSubjectDef(const DOM_Element &elem);
	static void createRouteSource(const DOM_Element &srcElem,
																const SubjectPHash &subjects,
																Route * r)
	                  throw (SubjectNotFoundException);
	static void createRouteDestination(const DOM_Element &dstElem,
																		 const SubjectPHash &subjects,
																		 Route * r)
	                  throw (SubjectNotFoundException);
	static Route * createRoute(const DOM_Element &elem,
														 const SubjectPHash &subjects)
	                  throw (SubjectNotFoundException);
private:
};

}
}
}
}
#endif // ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTECONFIG
