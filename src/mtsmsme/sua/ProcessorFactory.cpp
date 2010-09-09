static char const ident[] = "$Id$";
#include "mtsmsme/processor/Processor.h"
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
    virtual RequestProcessor* createRequestProcessor();
    virtual RequestProcessor* createRequestProcessor(TCO* _tco, SubscriberRegistrator* _reg);
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
RequestProcessor* RequestProcessorFactoryImpl::createRequestProcessor()
{
  return new SuaProcessor();
}
RequestProcessor* RequestProcessorFactoryImpl::createRequestProcessor(TCO* _tco, SubscriberRegistrator* _reg)
{
  return new SuaProcessor(_tco, _reg);
}
} /* namespace processor */
} /* namespace mtsmsme   */
} /* namespace smsc      */
