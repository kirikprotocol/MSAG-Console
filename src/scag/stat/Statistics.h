#ifndef SMSC_SCAG_STAT_STATISTICS
#define SMSC_SCAG_STAT_STATISTICS

#include "smeman/smeproxy.h"
#include "router/route_types.h"
#include "sms/sms.h"

#include "scag/transport/smpp/SmppCommand.h"
#include "scag/transport/smpp/WapCommand.h"
#include "scag/transport/smpp/MmsCommand.h"

namespace scag {
namespace stat {

using scag::transport::smpp::SmppCommand;
using scag::transport::smpp::WapCommand;
using scag::transport::smpp::MmsCommand;

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


    class Statistics
    {
    public:

        virtual void registerCommand(SmppCommand cmd) = 0;
        virtual void registerCommand(WapCommand cmd) = 0;
        virtual void registerCommand(MmsCommand cmd) = 0;
        virtual bool checkTraffic(string routeId, int period) = 0;

        enum CheckTrafficPeriod{
            checkMinPeriod,
            checkHourPeriod,
            checkDayPeriod,
            checkMonthPeriod
        };

        virtual ~Statistics() {};

    protected:
        Statistics() {};
    };

}//namespace stat
}//namespace scag

#endif // SMSC_STAT_STATISTICS
