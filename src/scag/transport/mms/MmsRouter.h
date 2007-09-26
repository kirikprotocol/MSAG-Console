#ifndef SCAG_TRANSPORT_MMS_ROUTER
#define SCAG_TRANSPORT_MMS_ROUTER

#include <string>

#include <xercesc/sax/HandlerBase.hpp>
#include <logger/Logger.h>
#include <util/xml/DOMTreeReader.h>
#include <util/xml/utilFunctions.h>

#include "MmsRouterTypes.h"
#include "XMLHandlers.h"

namespace scag {
namespace transport {
namespace mms {

XERCES_CPP_NAMESPACE_USE
using smsc::logger::Logger;
using std::string;

class MmsRouter {
public:
  MmsRouter() {};
  virtual const MmsRouteInfo* findRouteBySourceEndpointId(const char* endpointId);
  virtual void loadRoutes();
  void Init(const string& cfg_file);
  void printRoutes();
  ~MmsRouter() {};
private:
  void parseFile(const char* routesFile, HandlerBase* handler);

private:
  string cfg_file_name;
  Logger* logger;
  RouteHash routes;
};

}//mms
}//transport
}//scag

#endif

