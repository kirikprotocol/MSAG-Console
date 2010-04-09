#ifndef INFOSME_PERFORMANCE_TESTER_H
#define INFOSME_PERFORMANCE_TESTER_H

namespace smsc {
namespace infosme {

class PerformanceTester
{
public:
    PerformanceTester( const std::string& smscId,
                       InfosmePduListener& listener );
    void connect();
    SmppHeader* sendPdu( SmppHeader* pdu );
    void sendDeliverySmResp( PduDeliverySmResp& resp );
    void sendDataSmResp( PduDataSmResp& resp );
};

}
}

#endif /* INFOSME_PERFORMANCE_TESTER_H */
