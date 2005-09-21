#include "comps.hpp"


using smsc::inman::comp::DeliveryMode_e;

int main (void)
{
    Logger	tstLogger = Logger::getInstance("smsc.inman.comp.test");
    std::vector<unsigned char>	encBuf;

    /* testing initialDPSMS(): */
    smsc_log_debug(tstLogger, "--> InitialDPSMS");

    DeliveryMode_e mode = smsc::inman::comp::DeliveryMode_Originating;
    InitialDPSMSArg arg(mode);

    if (mode == smsc::inman::comp::DeliveryMode_Originating) {
	arg.setDestinationSubscriberNumber(".0.1.131133"); // missing for MT
	arg.setTPValidityPeriod( 120 , smsc::inman::comp::tp_vp_relative );
    } else {
	arg.setCalledPartyNumber(".0.1.131133"); // missing for MO
    }

    arg.setCallingPartyNumber(".1.1.79139163393");
    arg.setIMSI( "250013901388780" );
    Address vlr( ".1.1.79139860001" );
    arg.setlocationInformationMSC( vlr );

    arg.setSMSCAddress(".1.1.79029869990");

    time_t tm;
    time( &tm );
    arg.setTimeAndTimezone( tm );

    arg.setTPShortMessageSpecificInfo( 0x11 );
    arg.setTPProtocolIdentifier( 0x00 );
    arg.setTPDataCodingScheme( 0x08 );

    arg.encode(encBuf);


    return 0;
}
