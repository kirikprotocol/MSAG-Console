#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_COMPS_HPP__
#define __SMSC_INMAN_INAP_COMPS_HPP__

#include <vector>

namespace smsc {
namespace inman {
namespace comp{

using std::vector;
class Component
{
  public:
    virtual int encode(vector<unsigned char>& buf);
    virtual int decode(vector<unsigned char>& buf);
};


class ComponentFactory
{
public:
	ComponentFactory();
	virtual ~ComponentFactory();
	Component* createComponent(const vector<unsigned char>& opcode);
};

class InternalInitialDPSMSArg;
class InitialDPSMSArg: public Component
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
      int decode(vector<unsigned char>& buf);
    private:
      InternalInitialDPSMSArg* internal;
};

class ConnectSMSArg: public Component{};
class FurnishChargingInformationSMSArg: public Component{};
class ReleaseSMSArg: public Component{};
class RequestReportSMSEventArg: public Component{};
class ResetTimerSMSArg: public Component{};
class EventReportSMSArg: public Component{};

}
}
}

#endif
