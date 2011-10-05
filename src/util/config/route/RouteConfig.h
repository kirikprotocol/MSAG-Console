#ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTECONFIG
#define SMSC_UTIL_CONFIG_ROUTE_ROUTECONFIG

#include <xercesc/dom/DOM.hpp>
#include "util/config/route/RouteStructures.h"
#include "logger/Logger.h"
#include "util/AutoArrPtr.hpp"

namespace smsc {
namespace util {
namespace config {
namespace route {

using namespace xercesc;

class SubjectNotFoundException {};

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
  virtual ~RouteConfig();

  void clear();
//  status putRoute(Route *record);
//  status putSubject(Subject *subj);

  status load(const char * const filename);

  status reload();

  status store(const char * const filename) const;

  RouteIterator getRouteIterator() const;

  Subject& getSubject(const char* subjName)
  {
    return *subjects.Get(subjName);
  }

protected:
  smsc::logger::Logger *logger;
  RoutePVector routes;
  SubjectPHash subjects;
  smsc::util::auto_arr_ptr<char> config_filename;

  void expandSubject(Subject& subj);

  static Subject * createSubjectDef(const DOMElement &elem);
  static void createRouteSource(const DOMElement &srcElem, const SubjectPHash &subjects, Route * r) throw (SubjectNotFoundException);
  static void createRouteDestination(const DOMElement &dstElem, const SubjectPHash &subjects, Route * r) throw (SubjectNotFoundException);
  static Route * createRoute(const DOMElement &elem, const SubjectPHash &subjects) throw (SubjectNotFoundException);
private:
};

}
}
}
}
#endif // ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTECONFIG
