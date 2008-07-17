static char const ident[] = "$Id$";
#include "Processor.h"
#include "mtsmsme/sccp/SccpProcessor.hpp"
#include "mtsmsme/sua/SuaProcessor.hpp"
namespace smsc {
namespace mtsmsme {
namespace processor {

using smsc::mtsmsme::processor::RequestProcessor;
static RequestProcessorFactory* factory = 0;
Mutex flock;
class RequestProcessorFactoryImpl: public RequestProcessorFactory {
  public:
    virtual RequestProcessor* createRequestProcessor(const char* type);
};
RequestProcessorFactory* RequestProcessorFactory::getInstance()
{
  if ( factory == 0 ) {
    MutexGuard g(flock);
    if ( factory == 0 )
    {
      factory = new RequestProcessorFactoryImpl();
    }
  }
  return factory;
}
RequestProcessor* RequestProcessorFactoryImpl::createRequestProcessor(const char* type)
{
  RequestProcessor* processor = 0;
  if (strcmp(type,"tietoenator") == 0 )
    processor = new SccpProcessor();
  if (strcmp(type,"sibinco") == 0 )
    processor = new SuaProcessor();
  return processor;
}
} /* namespace processor */
} /* namespace mtsmsme   */
} /* namespace smsc      */
