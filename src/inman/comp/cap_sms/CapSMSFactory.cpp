static char const ident[] = "$Id$";

#include "inman/comp/cap_sms/CapSMSFactory.hpp"
#include "inman/comp/cap_sms/CapSMSComps.hpp"

namespace smsc {
namespace inman {
namespace comp {

OperationFactory * initCAP3SMSComponents(OperationFactory * fact)
{
    if (!fact) { //called from ApplicationContextFactory::getFactory()
        fact = CAP3SMSFactory::getInstance();   //getInstance() calls FIF in turn
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.comp.CAP3SMSFactory"));
        fact->registerArg(CapSMSOp::RequestReportSMSEvent,
                new CompFactory::ProducerT<smsc::inman::comp::RequestReportSMSEventArg>() );
        fact->registerArg(CapSMSOp::ReleaseSMS,
                new CompFactory::ProducerT<smsc::inman::comp::ReleaseSMSArg>() );
        fact->registerArg(CapSMSOp::ConnectSMS,
                new CompFactory::ProducerT<smsc::inman::comp::ConnectSMSArg>() );
        fact->registerArg(CapSMSOp::ResetTimerSMS,
                new CompFactory::ProducerT<smsc::inman::comp::ResetTimerSMSArg>() );

        fact->bindErrors(CapSMSOp::InitialDPSMS, 8,
                         CAP3SMSerrCode::missingParameter,
                         CAP3SMSerrCode::taskRefused,
                         CAP3SMSerrCode::unexpectedComponentSequence, 
                         CAP3SMSerrCode::unexpectedDataValue,
                         CAP3SMSerrCode::unexpectedParameter,
                         CAP3SMSerrCode::parameterOutOfRange,
                         CAP3SMSerrCode::systemFailure, 
                         InitialDPSMSArg::missingCustomerRecord
                         );
    }
    return fact;
}


} //comp
} //inman
} //smsc

