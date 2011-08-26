#ifndef SMSC_SCAG_STAT_STATISTICS2
#define SMSC_SCAG_STAT_STATISTICS2

//#include <smeman/smeproxy.h>
//#include <smeman/smeman.h>
//#include <router/route_types.h>
#include <sms/sms.h>
#include "scag/transport/smpp/router/route_types.h"
#include "scag/stat/sacc/SACC_Defs.h"
//#include <util/BufferSerialization.hpp>
#include "scag/util/SerializeBuffer.h"
#include "core/network/Socket.hpp"

#include <inttypes.h>

namespace scag2 {
namespace stat {

using namespace scag::stat::sacc;

using smsc::core::network::Socket;
using scag::util::SerializeBuffer;

    class SaccSerialBuffer //: public SerializationBuffer
    {
    public:
        void writeStr(std::string& s, uint16_t maxLen);
        void writeInt16(uint16_t i);
        void WriteNetInt16(uint16_t i);
        void writeInt32(uint32_t i);
        void WriteNetInt32(uint32_t i);
        void writeInt64(uint64_t i);
        void writeByte(uint8_t i);
        void writeFloat(float i);
        bool writeToSocket(Socket& sock);
        void* getBuffer();
        uint32_t getPos();
        void setPos(uint32_t newPos);
    private:
        SerializeBuffer buff_;
    };

    class SaccEvent
    {
        uint16_t sEventType;
    public:
        SaccEvent(uint16_t et) : sEventType(et) {};
        SaccEvent(const SaccEvent& src) : sEventType(src.sEventType) {};
        virtual ~SaccEvent() {}

        uint16_t getEventType()const {return sEventType; }
        virtual void write(SaccSerialBuffer& buf) = 0;
        virtual const char* getName() = 0;
    };

    struct SaccEventHeader
    {
    public:
        std::string pAbonentNumber;
        uint8_t  cCommandId;
        uint8_t  cProtocolId;
        uint16_t sCommandStatus;
        uint64_t lDateTime;
        uint32_t iServiceProviderId;
        uint32_t iServiceId;
        uint32_t iOperatorId;
        std::string pSessionKey;

        SaccEventHeader()
        {
            cCommandId=0;
            cProtocolId=0;
            sCommandStatus=0;
            lDateTime=0;
            iServiceProviderId=0;
            iServiceId=0;
            iOperatorId=0;
        }
        SaccEventHeader(const SaccEventHeader& src)
        {
            pSessionKey = src.pSessionKey;
            pAbonentNumber = src.pAbonentNumber;
            cCommandId=src.cCommandId;
            cProtocolId=src.cProtocolId;
            sCommandStatus=src.sCommandStatus;
            lDateTime=src.lDateTime;
            iServiceProviderId=src.iServiceProviderId;
            iServiceId=src.iServiceId;
            iOperatorId = src.iOperatorId;
        }
        void write(SaccSerialBuffer& buf);
    };

    struct SaccTrafficInfoEvent : public SaccEvent
    {
        SaccEventHeader Header;
        uint8_t  cDirection;
        std::string pMessageText;
        std::string keywords;

        SaccTrafficInfoEvent() : SaccEvent(sec_transport), cDirection(0) {};

        SaccTrafficInfoEvent(const SaccTrafficInfoEvent & src) : SaccEvent(src), Header(src.Header)
        {
            pMessageText=src.pMessageText;
            cDirection=src.cDirection;
            keywords = src.keywords;
        }
        void write(SaccSerialBuffer& buf);
        const char* getName() { return "SaccTrafficInfoEvent"; };
    };

    struct SaccBillingInfoEvent : public SaccEvent
    {
        SaccEventHeader Header;

        uint32_t iMediaResourceType;
        uint32_t iPriceCatId;
        //float    fBillingSumm;
        std::string  fBillingSumm;
        std::string  pBillingCurrency;
        std::string keywords;

        SaccBillingInfoEvent() : SaccEvent(sec_bill)
        {
            iMediaResourceType=0;
            iPriceCatId=0;
            fBillingSumm="";
        }
        SaccBillingInfoEvent(const SaccBillingInfoEvent & src) : SaccEvent(src), Header(src.Header)
        {
            pBillingCurrency=src.pBillingCurrency;
            iMediaResourceType=src.iMediaResourceType;
            iPriceCatId=src.iPriceCatId;
            fBillingSumm=src.fBillingSumm;
            keywords = src.keywords;
        }
        void write(SaccSerialBuffer& buf);
        const char* getName() { return "SaccBillingInfoEvent"; };
    };

    struct SaccAlarmEvent : public SaccEvent
    {
        SaccEventHeader Header;
        uint8_t  cDirection;
        uint32_t iAlarmEventId;
        std::string pMessageText;//[MAX_TEXT_MESSAGE_LENGTH];
        SaccAlarmEvent() : SaccEvent(sec_alarm)
        {
            cDirection=0;
            iAlarmEventId =0;
        }
        SaccAlarmEvent(const SaccAlarmEvent& src) : SaccEvent(src), Header(src.Header)
        {
            pMessageText=src.pMessageText;
            cDirection=src.cDirection;
            iAlarmEventId = src.iAlarmEventId;
        }
        void write(SaccSerialBuffer& buf);
        const char* getName() { return "SaccAlarmEvent"; };
    };

    struct SaccAlarmMessageEvent : public SaccEvent
    {
    public:
        std::string  pAbonentsNumbers;
        std::string  pAddressEmail;
        std::string  pMessageText;
        std::string  pDeliveryTime;
        std::string  pPacketType;

        uint8_t  cCriticalityLevel;
        uint16_t  sUsr;
        uint16_t  sDestPort;
        uint16_t  sSrcPort;
        uint8_t cEsmClass;
        SaccAlarmMessageEvent() : SaccEvent(sec_alarm_message)
        {
            cCriticalityLevel=scl_info;
            sUsr = 0;
            sDestPort = 0;
            sSrcPort = 0;
            cEsmClass = 0;
        }
        SaccAlarmMessageEvent(const SaccAlarmMessageEvent& src) : SaccEvent(src)
        {
            pAbonentsNumbers=src.pAbonentsNumbers;
            pAddressEmail=src.pAddressEmail;
            pMessageText=src.pMessageText;
            pDeliveryTime=src.pDeliveryTime;
            pPacketType=src.pPacketType;
            cCriticalityLevel = src.cCriticalityLevel;
            sUsr = src.sUsr;
            sDestPort = src.sDestPort;
            sSrcPort = src.sSrcPort;
            cEsmClass = src.cEsmClass;
        }
        void write(SaccSerialBuffer& buf);
        const char* getName() { return "SaccAlarmMessageEvent"; };
    };

    namespace events{
        namespace smpp{
            const int RECEIPT_OK = 18;
            const int RECEIPT_FAILED = 19;

            const int FAILED = 20;
            const int REJECTED = 21;
            const int ACCEPTED = 22;

            const int RESP_OK = 23;
            const int RESP_EXPIRED = 24;
            const int RESP_REJECTED = 25;
            const int RESP_GW_REJECTED = 26;
            const int RESP_FAILED = 27;
            const int GW_REJECTED = 28;
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

      SmppStatEvent() :
      routeProviderId(-1), srcType(false), dstType(false), event(-1), errCode(-1)
      {
        srcId[0]=0;
        dstId[0]=0;
        routeId[0]=0;
      }
      SmppStatEvent(const char* src, bool _srcType, const char* dst,
                    bool _dstType, int cnt, int errcode) :
      routeProviderId(-1), srcType(_srcType), dstType(_dstType), event(cnt), errCode(errcode)
      {
        srcId[0] = 0;
        if(src != NULL)
            strncpy(srcId, src, sizeof(srcId));
        dstId[0] = 0;
        if(dst != NULL)
            strncpy(dstId, dst, sizeof(dstId));
        routeId[0]=0;
      }
      SmppStatEvent(const char* src, bool _srcType, const char* dst, bool _dstType,
                    const char* rid, int cnt, int errcode) :
      routeProviderId(-1), srcType(_srcType), dstType(_dstType), event(cnt), errCode(errcode)
      {
        srcId[0] = 0;
        if(src != NULL)
            strncpy(srcId, src, sizeof(srcId));
        dstId[0] = 0;
        if(dst != NULL)
            strncpy(dstId, dst, sizeof(dstId));
        routeId[0]=0;
        if(rid!=NULL)
          strncpy(routeId, rid, sizeof(routeId));
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
        : routeId(rId), url(_url), site(_url), serviceId(sId), serviceProviderId(spId), event(cnt), errCode(err)
       {
            url += _path;
       };

      HttpStatEvent(const HttpStatEvent& cp)
        : routeId(cp.routeId), url(cp.url), site(cp.site), serviceId(cp.serviceId), serviceProviderId(cp.serviceProviderId), event(cp.event), errCode(cp.errCode) {};

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

        virtual void registerSaccEvent( SaccTrafficInfoEvent* ev) = 0;
        virtual void registerSaccEvent( SaccBillingInfoEvent* ev) = 0;
        virtual void registerSaccEvent( SaccAlarmEvent* ev) = 0;
        virtual void registerSaccEvent( SaccAlarmMessageEvent* ev) = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;

        virtual ~Statistics();

    protected:
        Statistics();

    private:
        Statistics(const Statistics& statistics) {};
    };

}//namespace stat
}//namespace scag

#endif // SMSC_STAT_STATISTICS
