//#include <assert.h>

#include "comfactory.hpp"

using smsc::inman::comp::InapOpCode;

using smsc::inman::comp::CompFactory;
using smsc::inman::comp::OperationFactory;
using smsc::inman::comp::OperationFactoryInstanceT;
using smsc::inman::comp::ApplicationContextFactory;

namespace smsc {
namespace inman {
namespace comp {

void initCAP4SMSComponents(OperationFactory * fact)
{
    fact->setLogger(Logger::getInstance("smsc.inman.comp.ComponentFactory"));
    fact->registerArg(InapOpCode::RequestReportSMSEvent,
	    new CompFactory::ProducerT<smsc::inman::comp::RequestReportSMSEventArg>() );
}


} //comp
} //inman
} //smsc

