#ifndef SMSC_SMPPGW_STAT_STATISTICS
#define SMSC_SMPPGW_STAT_STATISTICS


namespace smsc {
namespace smppgw {
namespace stat {

    namespace Counters{
        enum StatCounter{
          cntAccepted,
          cntRejected,
          cntDelivered,
          cntTemp,
          cntPerm,

          cntServiceBase=0x1000,

          cntDeniedByBilling,
          cntSmsTrOk,
          cntSmsTrFailed,
          cntSmsTrBilled,
          cntUssdTrFromScOk,
          cntUssdTrFromScFailed,
          cntUssdTrFromScBilled,
          cntUssdTrFromSmeOk,
          cntUssdTrFromSmeFailed,
          cntUssdTrFromSmeBilled
        };
    }

    class IStatistics
    {
    public:

        virtual void flushStatistics() = 0;

        virtual void updateCounter(int counter, const char* srcSmeId, const char* routeId,int errorCode=0) = 0;

        virtual ~IStatistics() {};

    protected:

        IStatistics() {};
    };

}//namespace stat
}//namespace smppgw
}//namespace smsc

#endif // SMSC_STAT_STATISTICS
