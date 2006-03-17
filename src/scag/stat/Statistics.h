#ifndef SMSC_SCAG_STAT_STATISTICS
#define SMSC_SCAG_STAT_STATISTICS

#include <smeman/smeproxy.h>
#include <smeman/smeman.h>
#include <router/route_types.h>
#include <sms/sms.h>
#include "scag/transport/smpp/router/route_types.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"

#include <inttypes.h>


namespace scag {
namespace stat {

using smsc::smeman::SmeRecord;

    namespace Counters
    {

/* additions for sacc */
		
		typedef enum
		{
			SACC_SEND_TRANSPORT_EVENT=0x0001,
			SACC_SEND_BILL_EVENT=0x0002,	
			SACC_SEND_ALARM_EVENT=0x0003,
			SACC_SEND_OPERATOR_NOT_FOUND_ALARM=0x0004,
			SACC_SEND_SESSION_EXPIRATION_TIME_ALARM=0x0005,
			SACC_SEND_ALARM_MESSAGE=0x0101

		}SaccEvents;
		typedef enum
		{
			pci_smppSubmitSm=1,
			pci_smppSubmitSmResp=2,
			pci_smppDeliverSm=3,
			pci_smppDeliverSmResp=4,
			pci_smppReceived=5,
			
			pci_httpRequest=6,
			pci_httpResponse=7,
			pci_httpDeliver=8

		}SaccProtocolCommandIds;

		typedef enum
		{
			pid_smpp_sms=1,
			pid_smpp_ussd=2,
			pid_http=3,
			pid_mms=4
		}SaccProtocolIds;
		
		typedef enum
		{
			bci_open=1,
			bci_commit=2,
			bci_rollback=3,
			bci_rollback_by_timeout=4

		}SaccBillCommandIds;

		typedef enum
		{
			eid_operatorNotFound=1,
			eid_routeNotFound=2,
			eid_sessionExpared=3
			
		}SaccEventsIds;
/**/
        typedef enum 
        {
          cntAccepted,
          cntRejected,
          cntDelivered,
          cntGw_Rejected,
          cntFailed,

          cntBillingOk = 0x1000,
          cntBillingFailed,
          cntRecieptOk,
          cntRecieptFailed
        } SmppStatCounter;

        typedef enum
        {
          httpRequest,
          httpRequestRejected,
          httpResponse,
          httpResponseRejected,
          httpDelivered,
          httpFailed,

          httpBillingOk = 0x1000,
          httpBillingFailed
        } HttpStatCounter;
    }

	struct SaccStatistics
	{
	  uint16_t 	command_id;
	  char		abonent_addr[25];
	  uint64_t	timestamp;
	  uint32_t	operator_id;
	  uint32_t	provider_id;
	  uint32_t	service_id;
	  uint8_t	session_id[45];
	  
	  uint8_t	protocol_id;
	  uint8_t	protocol_command_id;
	  uint16_t	protocol_command_status;
	  uint16_t	message[1024];
	  char		direction;


	  uint8_t	billing_command_id;	
	  uint16_t	billing_command_status;	
	  uint32_t	media_type;	
	  uint32_t	category_id;	
	  uint32_t	bill_value;	
	  char		bill_currency[10];

	  uint32_t  event_id;

	  char receiver_phones[1024];
	  char receiver_emails[1024];
	};
	
    struct SmppStatEvent
    {
      char smeId[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
      char routeId[smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH+1];
      int  smeProviderId;
      int  routeProviderId;

	  /**/

	  SaccStatistics sacc_stat;
	  
	  /**/	

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
		memset(&sacc_stat,0,sizeof(SaccStatistics));
		
        internal = false;
      }
      SmppStatEvent(scag::transport::smpp::SmppEntityInfo& smppEntity, int cnt, int errcode)
      {
		strncpy(smeId, smppEntity.systemId, sizeof(smeId));
        smeProviderId = smppEntity.providerId;
		routeId[0]=0;
        routeProviderId=-1;
        counter = cnt;
        errCode = errcode;
        internal = ( smppEntity.type == scag::transport::smpp::etSmsc );
      }
      SmppStatEvent(scag::transport::smpp::SmppEntityInfo& smppEntity, scag::transport::smpp::router::RouteInfo& ri, int cnt, int errcode)
      {
        strncpy(smeId, smppEntity.systemId, sizeof(smeId));
        smeProviderId = smppEntity.providerId;
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
		memcpy(&sacc_stat,&src.sacc_stat,sizeof(SaccStatistics));
        smeProviderId=src.smeProviderId;
        routeProviderId=src.routeProviderId;
        counter = src.counter;
        errCode = src.errCode;
        internal = src.internal;

      }
    };
    
    struct HttpStatEvent
    {
      std::string routeId;
      std::string serviceId;
      int serviceProviderId;
      int counter;
      int errCode;
      
  	  SaccStatistics sacc_stat;

      HttpStatEvent(int cnt=-1, const std::string& rId="", const std::string& sId="", int spId=-1, int err=0)
        : routeId(rId), serviceId(sId), serviceProviderId(spId), counter(cnt), errCode(err) {};

      // TODO: add copy constructor & operator=
    };

    enum CheckTrafficPeriod
    {
        checkMinPeriod,
        checkHourPeriod,
        checkDayPeriod,
        checkMonthPeriod
    };

    class Statistics
    {
    public:

        static Statistics& Instance();

        virtual void registerEvent(const SmppStatEvent& se) = 0;
        virtual void registerEvent(const HttpStatEvent& se) = 0;
        virtual bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value) = 0;

    protected:

        Statistics() {};
        Statistics(const Statistics& statistics) {};
        virtual ~Statistics() {};
    };

}//namespace stat
}//namespace scag

#endif // SMSC_STAT_STATISTICS
