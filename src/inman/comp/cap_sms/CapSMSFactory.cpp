#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/comp/cap_sms/CapSMSFactory.hpp"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
using smsc::inman::comp::CompFactory;

namespace smsc {
namespace inman {
namespace comp {

static uint8_t _octs_ac_cap3_sms[] = //id-ac-cap3-sms-AC:
//    { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) umts-network(1)
//      cap3OE(21) ac(3) id-ac-cap3-sms-AC(61) }
    {7, 0x04, 0x00, 0x00, 0x01, 0x15, 0x03, 0x3D};

EncodedOID _ac_cap3_sms(_octs_ac_cap3_sms, "ac_cap3_sms");

//ROSFactoryProducer implementation
ROSComponentsFactory * initCAP3SMSComponents(void)
{
    ROSComponentsFactory * fact = new ROSComponentsFactory(_ac_cap3_sms);
    fact->registerArg(CapSMSOp::RequestReportSMSEvent,
            new CompFactory::ProducerT<smsc::inman::comp::SMSRequestReportEventArg>() );
    fact->registerArg(CapSMSOp::ReleaseSMS,
            new CompFactory::ProducerT<smsc::inman::comp::SMSReleaseArg>() );
    fact->registerArg(CapSMSOp::ConnectSMS,
            new CompFactory::ProducerT<smsc::inman::comp::SMSConnectArg>() );
    fact->registerArg(CapSMSOp::ResetTimerSMS,
            new CompFactory::ProducerT<smsc::inman::comp::SMSResetTimerArg>() );
    fact->registerArg(CapSMSOp::FurnishChargingInformationSMS,
            new CompFactory::ProducerT<smsc::inman::comp::SMSFurnishChargingInformationArg>() );

    fact->bindErrors(CapSMSOp::InitialDPSMS, 8,
                     CAP3SMSerrCode::missingParameter,
                     CAP3SMSerrCode::taskRefused,
                     CAP3SMSerrCode::unexpectedComponentSequence, 
                     CAP3SMSerrCode::unexpectedDataValue,
                     CAP3SMSerrCode::unexpectedParameter,
                     CAP3SMSerrCode::parameterOutOfRange,
                     CAP3SMSerrCode::systemFailure, 
                     SMSInitialDPArg::missingCustomerRecord
                     );
    return fact;
}

} //comp
} //inman
} //smsc

