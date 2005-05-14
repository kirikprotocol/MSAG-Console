#ifndef SMSC_SCAG_STAT_STATISTICS
#define SMSC_SCAG_STAT_STATISTICS

#include "smeman/smeproxy.h"
#include "router/route_types.h"
#include "sms/sms.h"

namespace smsc {
namespace scag {
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

    struct StatInfo{
      char smeId[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
      char routeId[smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH+1];
      int  smeProviderId;
      int  routeProviderId;
      StatInfo()
      {
        smeId[0]=0;
        routeId[0]=0;
        smeProviderId=-1;
        routeProviderId=-1;
      }
      StatInfo(smsc::smeman::SmeProxy* proxy)
      {
        strncpy(smeId,proxy->getSystemId(),sizeof(smeId));
        smeProviderId=proxy->getProviderId();
        routeId[0]=0;
        routeProviderId=-1;
      }
      StatInfo(smsc::smeman::SmeProxy* proxy,const smsc::router::RouteInfo& ri)
      {
        strncpy(smeId,proxy->getSystemId(),sizeof(smeId));
        smeProviderId=proxy->getProviderId();
        strncpy(routeId,ri.routeId.c_str(),sizeof(routeId));
        routeProviderId=ri.providerId;
      }
      StatInfo(const StatInfo& src)
      {
        memcpy(smeId,src.smeId,sizeof(smeId));
        memcpy(routeId,src.routeId,sizeof(routeId));
        smeProviderId=src.smeProviderId;
        routeProviderId=src.routeProviderId;
      }
    };


    class IStatistics
    {
    public:

        virtual void flushStatistics() = 0;

        virtual void updateCounter(int counter,const StatInfo& si,int errorCode=0) = 0;

        virtual ~IStatistics() {};

    protected:

        IStatistics() {};
    };

}//namespace stat
}//namespace scag
}//namespace smsc

#endif // SMSC_STAT_STATISTICS
