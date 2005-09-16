#ifndef SCAG_CONFIG_ROUTE_ROUTECONFIG
#define SCAG_CONFIG_ROUTE_ROUTECONFIG

#include <xercesc/dom/DOM.hpp>
#include "scag/config/route/RouteStructures.h"
#include "logger/Logger.h"

namespace scag {
namespace config {

using namespace xercesc;

class SubjectNotFoundException {};

class RouteConfig{
public:
  enum status {success, fail};

  class RouteIterator
  {
  public:
    bool hasRecord() {return iter != end;}
    status fetchNext(scag::config::Route *&record);
    RouteIterator(scag::config::RoutePVector const &routs_vector);
  protected:
    scag::config::RoutePVector::const_iterator iter;
    scag::config::RoutePVector::const_iterator end;
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

protected:
  smsc::logger::Logger *logger;
  scag::config::RoutePVector routes;
  scag::config::SubjectPHash subjects;
  std::auto_ptr<char> config_filename;

  static bool getAttribBool(const DOMElement &elem, const char * name);
  static int  getAttribInt(const DOMElement &elem, const char * name);
  static std::string getAttribStr(const DOMElement &elem, const char * name);
  static scag::config::Subject * createSubjectDef(const DOMElement &elem);
  static void createRouteSource(const DOMElement &srcElem, const scag::config::SubjectPHash &subjects, scag::config::Route * r) throw (SubjectNotFoundException);
  static void createRouteDestination(const DOMElement &dstElem, const scag::config::SubjectPHash &subjects, scag::config::Route * r) throw (SubjectNotFoundException);
  static scag::config::Route * createRoute(const DOMElement &elem, const scag::config::SubjectPHash &subjects) throw (SubjectNotFoundException);
private:
};

}
}
#endif // ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTECONFIG
