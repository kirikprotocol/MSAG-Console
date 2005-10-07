#ifndef SMSC_SCAG_STAT_STATISTICS
#define SMSC_SCAG_STAT_STATISTICS

#include <smeman/smeproxy.h>
#include <smeman/smeman.h>
#include <router/route_types.h>
#include <sms/sms.h>
#include "scag/transport/smpp/router/route_types.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"

namespace scag {
namespace stat {

using smsc::smeman::SmeRecord;

    namespace Counters{
        enum SmppStatCounter{
          cntAccepted,
          cntRejected,
          cntDelivered,
          cntGw_Rejected,
          cntFailed,

          cntBillingOk = 0x1000,
          cntBillingFailed,
          cntRecieptOk,
          cntRecieptFailed

        };
    }

    struct SmppStatEvent{
      char smeId[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
      char routeId[smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH+1];
      int  smeProviderId;
      int  routeProviderId;
      int counter;
      int errCode;
      bool internal;
      SmppStatEvent()
      {
        smeId[0]=0;
        routeId[0]=0;
        smeProviderId=-1;
        routeProviderId=-1;
        counter = -1;
        errCode = -1;
        internal = false;
      }
      SmppStatEvent(scag::transport::smpp::SmppEntityInfo& smppEntity, int cnt, int errcode)
      {
        strncpy(smeId, smppEntity.systemId, sizeof(smeId));
        //smeProviderId = smppEntity.providerId;
        routeId[0]=0;
        routeProviderId=-1;
        counter = cnt;
        errCode = errcode;
        internal = ( smppEntity.type == scag::transport::smpp::etSmsc );
      }
      SmppStatEvent(scag::transport::smpp::SmppEntityInfo& smppEntity, scag::transport::smpp::router::RouteInfo& ri, int cnt, int errcode)
      {
        strncpy(smeId, smppEntity.systemId, sizeof(smeId));
        //smeProviderId = smppEntity.providerId;
        strncpy(routeId, (char*)ri.routeId, sizeof(routeId));
        routeProviderId=ri.providerId;
        counter = cnt;
        errCode = errcode;
        internal = ( smppEntity.type == scag::transport::smpp::etSmsc );
      }
      SmppStatEvent(const SmppStatEvent& src)
      {
        memcpy(smeId,src.smeId,sizeof(smeId));
        memcpy(routeId,src.routeId,sizeof(routeId));
        smeProviderId=src.smeProviderId;
        routeProviderId=src.routeProviderId;
        counter = src.counter;
        errCode = src.errCode;
        internal = src.internal;
      }
    };

    enum CheckTrafficPeriod{
        checkMinPeriod,
        checkHourPeriod,
        checkDayPeriod,
        checkMonthPeriod
    };


    class Statistics
    {
    public:

        static Statistics& Instance();

        virtual void registerEvent(const SmppStatEvent& si) = 0;
        virtual bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value) = 0;

    protected:

        Statistics() {};
        Statistics(const Statistics& statistics) {};
        virtual ~Statistics() {};
    };

}//namespace stat
}//namespace scag

#endif // SMSC_STAT_STATISTICS
