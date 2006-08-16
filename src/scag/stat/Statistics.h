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

    struct SACC_EVENT_HEADER_t
    {
    private:
        uint16_t sEventType;
    public:
        //uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH]; 
        std::string pAbonentNumber;
        uint8_t  cCommandId;        
        uint8_t  cProtocolId;       
        uint16_t sCommandStatus;
        uint64_t lDateTime;
        uint32_t iServiceProviderId;
        uint32_t iServiceId;
        uint32_t iOperatorId;
        
        SACC_EVENT_HEADER_t()
        {
            //memset(pAbonentNumber,0,MAX_ABONENT_NUMBER_LENGTH);
            pAbonentNumber="";  
            sEventType=0;
            cCommandId=0;       
            cProtocolId=0;      
            sCommandStatus=0;
            lDateTime=0;
            iServiceProviderId=0;
            iServiceId=0;
            iOperatorId=0;
        }
        SACC_EVENT_HEADER_t(const SACC_EVENT_HEADER_t& src)
        {
            //memcpy(pAbonentNumber,src.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);

            pAbonentNumber = src.pAbonentNumber;
            sEventType=src.sEventType;
            cCommandId=src.cCommandId;      
            cProtocolId=src.cProtocolId;        
            sCommandStatus=src.sCommandStatus;
            lDateTime=src.lDateTime;
            iServiceProviderId=src.iServiceProviderId;
            iServiceId=src.iServiceId;
            iOperatorId = src.iOperatorId;
        }
         uint16_t getEventType()const{return sEventType;} ;
        
         void setEventType(uint16_t et){sEventType = et;};
    };

    struct SACC_TRAFFIC_INFO_EVENT_t
    {
        SACC_EVENT_HEADER_t Header;
        
        uint8_t  cDirection;
        //uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];//512*32
        //uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
        std::string pMessageText;
        std::string pSessionKey;

        SACC_TRAFFIC_INFO_EVENT_t()
        {
            //memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
            //memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
            //memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
            pMessageText="";
            pSessionKey="";

            Header.setEventType(sec_transport);
            cDirection=0;
        };

        SACC_TRAFFIC_INFO_EVENT_t(const SACC_TRAFFIC_INFO_EVENT_t & src)
        {
            Header = src.Header;
            //memcpy(&Header,&src.Header,sizeof(SACC_EVENT_HEADER_t));
            //memcpy(pMessageText,src.pMessageText,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
            pMessageText=src.pMessageText;
            //memcpy(pSessionKey,src.pSessionKey,MAX_SESSION_KEY_LENGTH);
            pSessionKey=src.pSessionKey;
            cDirection=src.cDirection;
        }

        SACC_TRAFFIC_INFO_EVENT_t(SACC_TRAFFIC_INFO_EVENT_t *src)
        {
        
            Header = src->Header;
            //memcpy(&Header,&src.Header,sizeof(SACC_EVENT_HEADER_t));
            //memcpy(pMessageText,src.pMessageText,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
            pMessageText=src->pMessageText;
            //memcpy(pSessionKey,src.pSessionKey,MAX_SESSION_KEY_LENGTH);
            pSessionKey=src->pSessionKey;
            cDirection=src->cDirection;
            
        }


        uint16_t getEventType()const{return Header.getEventType();} ;
    };

    struct SACC_BILLING_INFO_EVENT_t
    {
        SACC_EVENT_HEADER_t Header;

        uint32_t iMediaResourceType;
        uint32_t iPriceCatId;
        float    fBillingSumm; 
        std::string  pBillingCurrency;//[MAX_BILLING_CURRENCY_LENGTH];
        std::string  pSessionKey;//[MAX_SESSION_KEY_LENGTH];
     
        SACC_BILLING_INFO_EVENT_t()
        {
            //memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
            //memset(pBillingCurrency,0,MAX_BILLING_CURRENCY_LENGTH);
            //memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
            pBillingCurrency="";
            pSessionKey="";
            Header.setEventType(sec_bill);
            iMediaResourceType=0;
            iPriceCatId=0;
            fBillingSumm=0.00000; 
        }   
        SACC_BILLING_INFO_EVENT_t(const SACC_BILLING_INFO_EVENT_t & src)
        {
            Header = src.Header;
            //memcpy(&Header,&src.Header,sizeof(SACC_EVENT_HEADER_t));
            //memcpy(pBillingCurrency,src.pBillingCurrency,MAX_BILLING_CURRENCY_LENGTH);
            //memcpy(pSessionKey,src.pSessionKey,MAX_SESSION_KEY_LENGTH);
            pBillingCurrency=src.pBillingCurrency;
            pSessionKey=src.pSessionKey;
            iMediaResourceType=src.iMediaResourceType;
            iPriceCatId=src.iPriceCatId;
            fBillingSumm=src.fBillingSumm; 
        }
        SACC_BILLING_INFO_EVENT_t(SACC_BILLING_INFO_EVENT_t *src)
        {
            Header = src->Header;
            //memcpy(&Header,&src.Header,sizeof(SACC_EVENT_HEADER_t));
            //memcpy(pBillingCurrency,src.pBillingCurrency,MAX_BILLING_CURRENCY_LENGTH);
            //memcpy(pSessionKey,src.pSessionKey,MAX_SESSION_KEY_LENGTH);
            pBillingCurrency=src->pBillingCurrency;
            pSessionKey=src->pSessionKey;
            iMediaResourceType=src->iMediaResourceType;
            iPriceCatId=src->iPriceCatId;
            fBillingSumm=src->fBillingSumm; 
        }
        uint16_t getEventType()const{return Header.getEventType();}; 
    };


    struct SACC_ALARM_t
    {

        SACC_EVENT_HEADER_t Header;
        uint8_t  cDirection;
        uint32_t iAlarmEventId;
        std::string  pSessionKey;//[MAX_SESSION_KEY_LENGTH];
        std::string pMessageText;//[MAX_TEXT_MESSAGE_LENGTH];
        SACC_ALARM_t()
        {
            
            pSessionKey="";
            pMessageText="";
            Header.setEventType(0x0003);
            cDirection=0;
            iAlarmEventId =0;

        }
        SACC_ALARM_t(const SACC_ALARM_t& src)
        {
            Header = src.Header;
            pSessionKey= src.pSessionKey;
            pMessageText=src.pMessageText;
            cDirection=src.cDirection;
            iAlarmEventId = src.iAlarmEventId; 

        }

        SACC_ALARM_t(SACC_ALARM_t* src)
        {
            
            Header = src->Header;
            pSessionKey= src->pSessionKey; 
            pMessageText=src->pMessageText;
            cDirection=src->cDirection;
            iAlarmEventId = src->iAlarmEventId; 
            
        }

        uint16_t getEventType()const{return Header.getEventType();} ;

    };

    struct SACC_ALARM_MESSAGE_t
    {
    private:
        uint16_t sEventType;
    public:
        std::string  pAbonentsNumbers;//[MAX_NUMBERS_TEXT_LENGTH];
        std::string  pAddressEmail;//[MAX_EMAIL_ADDRESS_LENGTH];
        std::string  pMessageText;//[MAX_TEXT_MESSAGE_LENGTH];
        std::string  pDeliveryTime;//[DELEVIRY_TIME_LENGTH];

        uint8_t  cCriticalityLevel;
        uint16_t  sUsr;
        SACC_ALARM_MESSAGE_t()
        {
            pAbonentsNumbers="";//memset(pAbonentsNumbers,0,MAX_NUMBERS_TEXT_LENGTH);
            pAddressEmail="";//memset(pAddressEmail,0,MAX_EMAIL_ADDRESS_LENGTH*sizeof(uint16_t));
            pMessageText="";//memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
            pDeliveryTime="";//memset(pDeliveryTime,0,DELEVIRY_TIME_LENGTH);
            cCriticalityLevel=scl_info; 
            sEventType = sec_alarm_message;
            sUsr = 0;
        }
        SACC_ALARM_MESSAGE_t(const SACC_ALARM_MESSAGE_t & src)
        {
            pAbonentsNumbers=src.pAbonentsNumbers;  
            pAddressEmail=src.pAddressEmail;
            pMessageText=src.pMessageText;
            pDeliveryTime=src.pDeliveryTime;

         //   memcpy(pAbonentsNumbers,src.pAbonentsNumbers ,MAX_NUMBERS_TEXT_LENGTH);
         //   memcpy(pAddressEmail,src.pAddressEmail ,MAX_EMAIL_ADDRESS_LENGTH*sizeof(uint16_t));
         //   memcpy(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
         //   memcpy(pDeliveryTime,src.pDeliveryTime,DELEVIRY_TIME_LENGTH);
            cCriticalityLevel = src.cCriticalityLevel;
            sEventType=src.sEventType;
            sUsr = src.sUsr;
        }
        SACC_ALARM_MESSAGE_t(SACC_ALARM_MESSAGE_t * src)
        {
            pAbonentsNumbers=src->pAbonentsNumbers; 
            pAddressEmail=src->pAddressEmail;
            pMessageText=src->pMessageText;
            pDeliveryTime=src->pDeliveryTime;

         //   memcpy(pAbonentsNumbers,src.pAbonentsNumbers ,MAX_NUMBERS_TEXT_LENGTH);
         //   memcpy(pAddressEmail,src.pAddressEmail ,MAX_EMAIL_ADDRESS_LENGTH*sizeof(uint16_t));
         //   memcpy(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
         //   memcpy(pDeliveryTime,src.pDeliveryTime,DELEVIRY_TIME_LENGTH);
            cCriticalityLevel = src->cCriticalityLevel;
            sEventType=src->sEventType;
            sUsr = src->sUsr;
        }
        uint16_t getEventType()const{return sEventType;};

    };

    namespace events{
        namespace smpp{
/*            const int SUBMIT_FAILED = 1;
            const int SUBMIT_REJECTED = 2;
            const int SUBMIT_ACCEPTED = 3;
            const int DELIVER_FAILED = 4;
            const int DELIVER_REJECTED = 5;
            const int DELIVER_ACCEPTED = 6;
            const int SUBMIT_RESP_OK = 7;
            const int SUBMIT_RESP_FAILED = 8;
            const int DELIVER_RESP_OK = 9;
            const int DELIVER_RESP_FAILED = 10;*/
            const int RECEIPT_OK = 11;
            const int RECEIPT_FAILED = 12;

            const int FAILED = 20;
            const int REJECTED = 21;
            const int ACCEPTED = 22;

            const int RESP_OK = 23;
            const int RESP_FAILED = 24;
        }
        namespace http{
            const int REQUEST_OK = 1;
            const int REQUEST_FAILED = 2;
            const int RESPONSE_OK = 3;
            const int RESPONSE_FAILED = 4;
            const int DELIVERED = 5;
            const int FAILED = 6;
        }
    }

    struct SmppStatEvent
    {
      char srcId[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
      char dstId[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
      char routeId[smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH+1];
      int  routeProviderId;
      bool srcType, dstType;

      int event; // use constants from scag::stat::events::smpp
      int errCode;

      SmppStatEvent()
      {
        srcId[0]=0;
        dstId[0]=0;
        routeId[0]=0;
        routeProviderId=-1;
        event = -1;
        errCode = -1;
      }
      SmppStatEvent(const char* src, bool _srcType, const char* dst, bool _dstType, int cnt, int errcode)
      {
        srcId[0] = 0;
        if(src != NULL)
            strncpy(srcId, src, sizeof(srcId));
        srcType = _srcType;
        dstId[0] = 0;
        if(dst != NULL)
            strncpy(dstId, dst, sizeof(dstId));
        dstType = _dstType;
        routeId[0]=0;

        routeProviderId=-1;
        event = cnt;
        errCode = errcode;
      }
      SmppStatEvent(const char* src, bool _srcType, const char* dst, bool _dstType, const char* rid, int cnt, int errcode)
      {
        srcId[0] = 0;
        if(src != NULL)
            strncpy(srcId, src, sizeof(srcId));
        srcType = _srcType;
        dstId[0] = 0;
        if(dst != NULL)
            strncpy(dstId, dst, sizeof(dstId));
        dstType = _dstType;
        strncpy(routeId, rid, sizeof(routeId));
        event = cnt;
        errCode = errcode;
      }
      SmppStatEvent(const SmppStatEvent& src)
      {
        memcpy(srcId,src.srcId,sizeof(srcId));
        memcpy(dstId,src.dstId,sizeof(dstId));
        memcpy(routeId,src.routeId,sizeof(routeId));
        routeProviderId=src.routeProviderId;
        errCode = src.errCode;
        event = src.event;
        srcType = src.srcType;
        dstType = src.dstType;
      }
    };
    
    struct HttpStatEvent
    {
      std::string routeId;
      std::string url;
      std::string site;
      uint32_t serviceId;
      uint32_t serviceProviderId;
      int event; // use constants from scag::stat::events::http
      int errCode;
      
      HttpStatEvent(int cnt=-1, const std::string& rId="", const uint32_t sId=0, int spId=0, const std::string _url = "", const std::string _path = "", int err=0)
        : routeId(rId), serviceId(sId), serviceProviderId(spId), event(cnt), errCode(err), url(_url + "/" +_path), site(_url) {};

      HttpStatEvent(const HttpStatEvent& cp)
        : routeId(cp.routeId), serviceId(cp.serviceId), serviceProviderId(cp.serviceProviderId), event(cp.event), errCode(cp.errCode), url(cp.url), site(cp.site) {};

        HttpStatEvent& operator=(const HttpStatEvent& cp)
        {
            routeId = cp.routeId;
            serviceId = cp.serviceId;
            serviceProviderId = cp.serviceProviderId;
            event = cp.event;
            errCode = cp.errCode;
            url = cp.url;
            site = cp.site;
            return *this;
        }
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
//        virtual void registerSaccEvent(const scag::stat::SACC_OPERATOR_NOT_FOUND_ALARM_t& ev) = 0;
  //      virtual void registerSaccEvent(const scag::stat::SACC_SESSION_EXPIRATION_TIME_ALARM_t& ev) = 0;
        virtual void registerSaccEvent(const scag::stat::SACC_ALARM_t& ev) = 0;
        virtual void registerSaccEvent(const scag::stat::SACC_ALARM_MESSAGE_t& ev) = 0;

    protected:

        Statistics() {};
        Statistics(const Statistics& statistics) {};
        virtual ~Statistics() {};
    };

}//namespace stat
}//namespace scag

#endif // SMSC_STAT_STATISTICS
