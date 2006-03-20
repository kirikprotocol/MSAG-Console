#ifndef SMSC_SCAG_STAT_STATISTICS
#define SMSC_SCAG_STAT_STATISTICS

#include <smeman/smeproxy.h>
#include <smeman/smeman.h>
#include <router/route_types.h>
#include <sms/sms.h>
#include "scag/transport/smpp/router/route_types.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"
#include "sacc/SACC_Defs.h"

#include <inttypes.h>

using namespace scag::stat::sacc;
namespace scag {
namespace stat {

using smsc::smeman::SmeRecord;




    namespace Counters
    {

/* additions for sacc */
	


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

	struct SACC_EVENT_HEADER_t
	{
		uint16_t sEventType;
		uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH];	
		uint8_t  cCommandId;		
		uint8_t  cProtocolId;		
		uint16_t sCommandStatus;
		uint64_t lDateTime;
		uint32_t iServiceProviderId;
		uint32_t iServiceId;
		
		SACC_EVENT_HEADER_t()
		{
			memset(pAbonentNumber,0,MAX_ABONENT_NUMBER_LENGTH);

			sEventType=0;
			cCommandId=0;		
			cProtocolId=0;		
			sCommandStatus=0;
			lDateTime=0;
			iServiceProviderId=0;
			iServiceId=0;
		}
		SACC_EVENT_HEADER_t(const SACC_EVENT_HEADER_t& src)
		{
			memcpy(pAbonentNumber,src.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);

			sEventType=src.sEventType;
			cCommandId=src.cCommandId;		
			cProtocolId=src.cProtocolId;		
			sCommandStatus=src.sCommandStatus;
			lDateTime=src.lDateTime;
			iServiceProviderId=src.iServiceProviderId;
			iServiceId=src.iServiceId;
		}
	};

	struct SACC_TRAFFIC_INFO_EVENT_t
	{
		SACC_EVENT_HEADER_t Header;
		
		uint32_t iOperatorId;
		uint8_t  cDirection;
		uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];//512*32
		uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];

		SACC_TRAFFIC_INFO_EVENT_t()
		{
			memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
			memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
			memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
			iOperatorId=0;
			cDirection=0;
		};

		SACC_TRAFFIC_INFO_EVENT_t(const SACC_TRAFFIC_INFO_EVENT_t & src)
		{
			memcpy(&Header,&src.Header,sizeof(SACC_EVENT_HEADER_t));
			memcpy(pMessageText,src.pMessageText,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
			memcpy(pSessionKey,src.pSessionKey,MAX_SESSION_KEY_LENGTH);
			iOperatorId=src.iOperatorId;
			cDirection=src.cDirection;
		}

	};

	struct SACC_BILLING_INFO_EVENT_t
	{
		SACC_EVENT_HEADER_t Header;

		uint32_t iOperatorId;
		uint32_t iMediaResourceType;
		uint32_t iPriceCatId;
		float    fBillingSumm; 
		uint8_t  pBillingCurrency[MAX_BILLING_CURRENCY_LENGTH];
				 
		SACC_BILLING_INFO_EVENT_t()
		{
			memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
			memset(pBillingCurrency,0,MAX_BILLING_CURRENCY_LENGTH);
			iOperatorId=0;
			iMediaResourceType=0;
			iPriceCatId=0;
			fBillingSumm=0; 
		}	
		SACC_BILLING_INFO_EVENT_t(const SACC_BILLING_INFO_EVENT_t & src)
		{
			memcpy(&Header,&src.Header,sizeof(SACC_EVENT_HEADER_t));
			memcpy(pBillingCurrency,src.pBillingCurrency,MAX_BILLING_CURRENCY_LENGTH);
			iOperatorId=src.iOperatorId ;
			iMediaResourceType=src.iMediaResourceType;
			iPriceCatId=src.iPriceCatId;
			fBillingSumm=src.fBillingSumm; 
		}	
	};

	struct SACC_OPERATOR_NOT_FOUND_ALARM_t
	{
		SACC_EVENT_HEADER_t Header;

		uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
		uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];

		SACC_OPERATOR_NOT_FOUND_ALARM_t()
		{
			memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
			memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
			memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));

		}

		SACC_OPERATOR_NOT_FOUND_ALARM_t(const SACC_OPERATOR_NOT_FOUND_ALARM_t& src)
		{
			memcpy(&Header, &src.Header,sizeof(SACC_EVENT_HEADER_t));
			memcpy(pSessionKey,src.pSessionKey ,MAX_SESSION_KEY_LENGTH);
			memcpy(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));

		}
	};

	struct SACC_SESSION_EXPIRATION_TIME_ALARM_t
	{

		SACC_EVENT_HEADER_t Header;

		uint32_t iOperatorId;
		uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
		uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];
		SACC_SESSION_EXPIRATION_TIME_ALARM_t()
		{
			memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
			memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
			memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
			iOperatorId=0;
		}
		SACC_SESSION_EXPIRATION_TIME_ALARM_t(const SACC_SESSION_EXPIRATION_TIME_ALARM_t& src)
		{
			memcpy(&Header, &src.Header,sizeof(SACC_EVENT_HEADER_t));
			memcpy(pSessionKey,src.pSessionKey ,MAX_SESSION_KEY_LENGTH);
			memcpy(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
			iOperatorId=src.iOperatorId;
		}

	};

	struct SACC_ALARM_MESSAGE_t
	{
		uint16_t sEventType;
		uint8_t  pAbonentsNumbers[MAX_NUMBERS_TEXT_LENGTH];
		uint16_t pAddressEmail[MAX_EMAIL_ADDRESS_LENGTH];
		uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];

		SACC_ALARM_MESSAGE_t()
		{
			memset(pAbonentsNumbers,0,MAX_NUMBERS_TEXT_LENGTH);
			memset(pAddressEmail,0,MAX_EMAIL_ADDRESS_LENGTH*sizeof(uint16_t));
			memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
			sEventType=0;
		}
		SACC_ALARM_MESSAGE_t(const SACC_ALARM_MESSAGE_t & src)
		{
			memcpy(pAbonentsNumbers,src.pAbonentsNumbers ,MAX_NUMBERS_TEXT_LENGTH);
			memcpy(pAddressEmail,src.pAddressEmail ,MAX_EMAIL_ADDRESS_LENGTH*sizeof(uint16_t));
			memcpy(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
			sEventType=src.sEventType;

		}

	};

    struct SmppStatEvent
    {
      char smeId[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
      char routeId[smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH+1];
      int  routeProviderId;

      int counter;
      int errCode;
      bool internal;
	  

      SmppStatEvent()
      {
        smeId[0]=0;
        routeId[0]=0;
        routeProviderId=-1;
        counter = -1;
        errCode = -1;
		
        internal = false;
      }
      SmppStatEvent(scag::transport::smpp::SmppEntityInfo& smppEntity, int cnt, int errcode)
      {
        strncpy(smeId, smppEntity.systemId, sizeof(smeId));
        routeId[0]=0;

        routeProviderId=-1;
        counter = cnt;
        errCode = errcode;
        internal = ( smppEntity.type == scag::transport::smpp::etSmsc );
      }
      SmppStatEvent(scag::transport::smpp::SmppEntityInfo& smppEntity, scag::transport::smpp::router::RouteInfo& ri, int cnt, int errcode)
      {
        strncpy(smeId, smppEntity.systemId, sizeof(smeId));
        strncpy(routeId, (char*)ri.routeId, sizeof(routeId));
        counter = cnt;
        errCode = errcode;
        internal = ( smppEntity.type == scag::transport::smpp::etSmsc );
      }
      SmppStatEvent(const SmppStatEvent& src)
      {
        memcpy(smeId,src.smeId,sizeof(smeId));
        memcpy(routeId,src.routeId,sizeof(routeId));
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

        virtual void registerSaccEvent(const scag::stat::SACC_TRAFFIC_INFO_EVENT_t& ev) = 0;
        virtual void registerSaccEvent(const scag::stat::SACC_BILLING_INFO_EVENT_t& ev) = 0;
        virtual void registerSaccEvent(const scag::stat::SACC_OPERATOR_NOT_FOUND_ALARM_t& ev) = 0;
        virtual void registerSaccEvent(const scag::stat::SACC_SESSION_EXPIRATION_TIME_ALARM_t& ev) = 0;
        virtual void registerSaccEvent(const scag::stat::SACC_ALARM_MESSAGE_t& ev) = 0;

    protected:

        Statistics() {};
        Statistics(const Statistics& statistics) {};
        virtual ~Statistics() {};
    };

}//namespace stat
}//namespace scag

#endif // SMSC_STAT_STATISTICS
