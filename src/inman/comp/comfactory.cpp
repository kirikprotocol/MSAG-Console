#include <assert.h>

#include "comfactory.hpp"

namespace smsc {
namespace inman {
namespace comp{

ComponentFactory::ComponentFactory()
    : logger(Logger::getInstance("smsc.inman.comp.ComponentFactory"))
{
	registerProduct( InapOpCode::RequestReportSMSEvent, new ProducerT< RequestReportSMSEventArg >() );
}

ComponentFactory::~ComponentFactory()
{
}

ComponentFactory* ComponentFactory::getInstance()
{
	static ComponentFactory instance;
	return &instance;
}

}
}
}