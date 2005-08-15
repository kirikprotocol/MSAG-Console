#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_COMPS_HPP__
#define __SMSC_INMAN_INAP_COMPS_HPP__

#include <vector>

namespace smsc {
namespace inman {
namespace comp{

using std::vector;

class InternalInitialDPSMSArg;
class InitialDPSMSArg
{
    public:
      InitialDPSMSArg();
      ~InitialDPSMSArg();
      void setDestinationSubscriberNumber();
      void setCallingPartyNumber();
      void setMode();
      void setIMSI();
      void setlocationInformationMSC();
      void setSMSCAddress();
      void setTimeAndTimezone();
      void setTPShortMessageSpecificInfo();
      void setTPProtocolIdentifier();
      void setTPDataCodingScheme();
      void setTPValidityPeriod();
      int encode(vector<unsigned char>& buf);
    private:
      InternalInitialDPSMSArg* internal;
};

class ConnectSMSArg{};
class FurnishChargingInformationSMSArg{};
class ReleaseSMSArg{};
class RequestReportSMSEventArg{};
class ResetTimerSMSArg{};
class EventReportSMSArg{};

}
}
}

#endif
