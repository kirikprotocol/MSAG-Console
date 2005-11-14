static char const ident[] = "$Id$";
#include "comfactory.hpp"

using smsc::inman::comp::InapOpCode;

using smsc::inman::comp::CompFactory;
using smsc::inman::comp::OperationFactory;
using smsc::inman::comp::OperationFactoryInstanceT;
using smsc::inman::comp::ApplicationContextFactory;

namespace smsc {
namespace inman {
namespace comp {

OperationFactory * initCAP3SMSComponents(OperationFactory * fact)
{
    if (!fact) { //called from ApplicationContextFactory::getFactory()
        //getInstance() calls FIF in turn
        fact = smsc::ac::CAP3SMSFactory::getInstance();
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.comp.ComponentFactory"));
        fact->registerArg(InapOpCode::RequestReportSMSEvent,
                new CompFactory::ProducerT<smsc::inman::comp::RequestReportSMSEventArg>() );
        fact->registerArg(InapOpCode::ReleaseSMS,
                new CompFactory::ProducerT<smsc::inman::comp::ReleaseSMSArg>() );
        fact->registerArg(InapOpCode::ConnectSMS,
                new CompFactory::ProducerT<smsc::inman::comp::ConnectSMSArg>() );
        fact->registerArg(InapOpCode::ResetTimerSMS,
                new CompFactory::ProducerT<smsc::inman::comp::ResetTimerSMSArg>() );
    }
    return fact;
}


} //comp
} //inman
} //smsc

