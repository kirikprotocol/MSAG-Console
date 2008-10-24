#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>

#include "MmsRouter.h"
#include "XMLHandlers.h"
//#include <scag/util/singleton/Singleton.h>

namespace scag { namespace transport { namespace mms {

using namespace smsc::util::xml;
//using namespace scag::util::singleton;

/*
static bool router_inited = false;
static Mutex router_init_lock;

inline unsigned GetLongevity(MmsRouter* ) { return 251; };
typedef scag::util::singleton::SingletonHolder<MmsRouterImpl> SingleMR;

MmsRouter& MmsRouter::Instance() {
  {
    MutexGuard g(router_init_lock);
    if (!router_inited) {
      throw std::runtime_error("MmsRouter is not inited!");
    }
  }
  return SingleMR::Instance();
}

void MmsRouter::Init(const std::string &cfg) {
  MutexGuard g(router_init_lock);
  if (!router_inited) {
    MmsRouterImpl &mr = SingleMR::Instance();
    mr.init(cfg);
    router_inited = true;
  }
}
*/

void MmsRouter::Init(const string& cfg_file) {
  cfg_file_name = cfg_file;
  logger = Logger::getInstance("mms.router");
  loadRoutes();
  printRoutes();
  smsc_log_debug(logger, "MmsRouter Initialized");
}

void MmsRouter::parseFile(const char* routesFile, HandlerBase *handler) {
  SAXParser parser;
  try {
    parser.setValidationScheme(SAXParser::Val_Always);
    parser.setDoSchema(true);
    parser.setValidationSchemaFullChecking(true);
    parser.setDoNamespaces(true);

    parser.setValidateAnnotations(false);

    parser.setValidationConstraintFatal(true);

    parser.setDocumentHandler(handler);
    parser.setErrorHandler(handler);
    parser.parse(routesFile);
  } catch (const OutOfMemoryException&) {
      throw Exception("XMLPlatform: OutOfMemoryException");
  } catch (const XMLException& toCatch) {
      StrX msg(toCatch.getMessage());
      throw Exception("XMLException: %s", msg.localForm());
  } catch (const Exception& e) {
      throw Exception(e);
  } catch (...) {
      throw Exception("Unknown fatal error");
  }
}

void MmsRouter::loadRoutes() {
  RouterXMLHandler handler(&routes);
  parseFile(cfg_file_name.c_str(), &handler);
}

void MmsRouter::printRoutes() {
  char* name = 0;
  MmsRouteInfo* route = 0;
  routes.First();
  while (routes.Next(name, route)) {
    smsc_log_debug(logger, "SourceId=\'%s\'", name);
    route->print(logger);
  }
}
const MmsRouteInfo* MmsRouter::findRouteBySourceEndpointId(const char* endpointId) {
  return routes.GetPtr(endpointId);
}
}//mms
}//transport
}//scag

