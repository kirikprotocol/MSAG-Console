static char const ident[] = "$Id$";
#include "mtsmsme/processor/Processor.h"
#include "mtsmsme/sccphd/SccpHDProcessor.hpp"
namespace smsc {
namespace mtsmsme {
namespace processor {

using smsc::mtsmsme::processor::RequestProcessor;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

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
  return  new SccpHDProcessor();
}
RequestProcessor* RequestProcessorFactoryImpl::createRequestProcessor(TCO* _tco, SubscriberRegistrator* _reg)
{
  return new SccpHDProcessor(_tco, _reg);
}
} /* namespace processor */
} /* namespace mtsmsme   */
} /* namespace smsc      */
