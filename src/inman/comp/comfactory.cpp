static char const ident[] = "$Id$";

#include "comfactory.hpp"
#include "inman/comp/comps.hpp"

//using smsc::inman::comp::InapOpCode;
//using smsc::inman::comp::CompFactory;

namespace smsc {
namespace inman {
namespace comp {

OperationFactory * initCAP3SMSComponents(OperationFactory * fact)
{
    if (!fact) { //called from ApplicationContextFactory::getFactory()
        fact = CAP3SMSFactory::getInstance();   //getInstance() calls FIF in turn
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.comp.CAP3SMSFactory"));
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

