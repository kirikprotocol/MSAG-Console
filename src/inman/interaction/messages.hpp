#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "inman/interaction/serializer.hpp"
#include "inman/common/factory.hpp"

using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::SerializerITF;
using smsc::inman::common::FactoryT;

#define INMAN_RPCAUSE_LIMIT         (unsigned int)255
#define INMAN_PROTOCOL_ERROR_BASE   (unsigned int)260
//TCP dialogue command sequence broken
#define INMAN_PROTOCOL_ERROR        INMAN_PROTOCOL_ERROR_BASE 
#define INMAN_PROTOCOL_ERROR_LIMIT  (unsigned int)279

#define INMAN_SCF_ERROR_BASE        (unsigned int)280

namespace smsc  {
namespace inman {
namespace interaction {

//serializer for Inman commands, transferred over TCP socket
class SerializerInap : public SerializerITF, public FactoryT< unsigned short, SerializableObject >
{
    public:
        enum { FORMAT_VERSION = 0x0001 };

        virtual ~SerializerInap();

        SerializableObject* deserialize(ObjectBuffer&);
        void                serialize(SerializableObject*, ObjectBuffer& out);

        static SerializerInap* getInstance();

    protected:
        SerializerInap();
};


// 1. ChargeSms         ( SMSC --> INMAN )
// 2. ChargeSmsResult   ( SMSC <-- INMAN )
// 3. DeliverySmsResult ( SMSC --> INMAN )


// Use 'visitor' pattern
class InmanHandler;
class InmanCommand : public SerializableObject
{
    public:
        virtual void handle( InmanHandler* ) = 0;
};

class SmscHandler;
class SmscCommand : public SerializableObject
{
    public:
        virtual void handle( SmscHandler* ) = 0;
};

enum {
    CHARGE_SMS_TAG          = 1,
    CHARGE_SMS_RESULT_TAG   = 2,
    DELIVERY_SMS_RESULT_TAG = 3
};


typedef enum
{
    CHARGING_POSSIBLE       = 0,
    CHARGING_NOT_POSSIBLE   = 1
} 
ChargeSmsResult_t;

typedef enum
{
    DELIVERY_SUCCESSED = 0,
    DELIVERY_FAILED    = 1
} 
DeliverySmsResult_t;


struct CDRRecord {
    typedef enum { dpOrdinary = 0, dpReaddressed } CDRRecordType;
    typedef enum { dpText = 0, dpBinary } CDRMediaType;
    typedef enum { dpSMS = 0, dpUSSD } CDRBearerType;

    //basic info:
    uint64_t        _msgId;         //MSG_ID: system message identifier
    CDRRecordType   _cdrType;       //RECORD_TYPE:
    CDRMediaType    _mediaType;     //MEDIA_TYPE:
    CDRBearerType   _bearer;        //BEARER_TYPE:
    std::string     _routeId;       //ROUTE_ID:
    int32_t         _serviceId;     //SERVICE_ID: set on route
    uint32_t        _userMsgRef;    //USER_MSG_REF: system identifier for dialog tracing
    //sender info
    time_t          _submitTime;    //SUBMIT: sms submit time
    std::string     _srcAdr;        //SRC_ADDR: sender number
    std::string     _srcIMSI;       //SRC_IMSI: sender IMSI
    std::string     _srcMSC;        //SRC_MSC: sender MSC
    std::string     _srcSMEid;      //SRC_SME_ID: sender SME identifier
    //recipient info
    time_t          _finalTime;     //FINALIZED: sms delivery time
    std::string     _dstAdr;        //DST_ADDR: final recipient number
    std::string     _dstIMSI;       //DST_IMSI: recipient IMSI
    std::string     _dstMSC;        //DST_MSC:	recipient MSC
    std::string     _dstSMEid;      //DST_SME_ID: recipient SME identifier
    DeliverySmsResult_t _dlvrRes;   //STATUS: delivery status

//not processed for now:
//DIVERTED_FOR  Ќомер оригинального получател€, но доставлено на DST_ADDR                                    
//DATA_LENGTH   ƒлина сообщени€, в символах дл€ текстовых сообщений и в октетах дл€ бинарных сообщений
};

class ChargeSms : public InmanCommand
{
public:
    ChargeSms();
    virtual ~ChargeSms();

    void setDestinationSubscriberNumber(const std::string& imsi);
    void setCallingPartyNumber(const std::string& imsi);
    void setCallingIMSI(const std::string& imsi);
    void setSMSCAddress(const std::string& imsi);
    void setSubmitTimeTZ(time_t tmVal);
    void setTPShortMessageSpecificInfo(unsigned char );
    void setTPProtocolIdentifier(unsigned char );
    void setTPDataCodingScheme(unsigned char );
    void setTPValidityPeriod(time_t vpVal);
    void setLocationInformationMSC(const std::string& imsi);
    //data for CDR generation
    void setCallingSMEid(const std::string & sme_id);
    void setRouteId(const std::string & route_id);
    void setServiceId(int32_t service_id);
    void setUserMsgRef(uint32_t msg_ref);
    void setMsgId(uint64_t msg_id);
    void setServiceOp(int32_t service_op);
    //data for InitialDP OPERATION
    const std::string & getDestinationSubscriberNumber() const;
    const std::string & getCallingPartyNumber() const;
    const std::string & getCallingIMSI() const;
    const std::string & getSMSCAddress() const;
    const std::string & getLocationInformationMSC() const;
    time_t              getSubmitTimeTZ() const;
    time_t              getTPValidityPeriod() const;
    unsigned char       getTPShortMessageSpecificInfo() const;
    unsigned char       getTPProtocolIdentifier() const;
    unsigned char       getTPDataCodingScheme() const;

    void export2CDR(CDRRecord & cdr);
    //InmanCommand interface
    void handle(InmanHandler*);

protected:
    //SerializableObject interface
    void load(ObjectBuffer& in);
    void save(ObjectBuffer& out);

private:
    std::string   destinationSubscriberNumber;
    std::string   callingPartyNumber;
    std::string   callingImsi;
    std::string   smscAddress;
    time_t        submitTimeTZ;
    unsigned char tpShortMessageSpecificInfo;
    unsigned char tpProtocolIdentifier;
    unsigned char tpDataCodingScheme;
    time_t        tpValidityPeriod;
    std::string   locationInformationMSC; //keeps SRC_MSC
    //data for CDR generation
    std::string   callingSMEid; //"MAP_PROXY"
    std::string   routeId;      //"sibinco.sms > plmn.kem"
    int32_t       serviceId;    // ???
    uint32_t      userMsgRef;  // ???
    uint64_t      msgId;       // ?????
    int32_t       ussdServiceOp; //see sms_const.h
};

//NOTE: in case of CAP3 error, this command ends the TCP dialog.
class ChargeSmsResult : public SmscCommand
{
public:
    ChargeSmsResult();                //positive result, no error
    ChargeSmsResult(uint32_t rPCcode); //negative result by RP cause or CAP3 error
    virtual ~ChargeSmsResult();

    ChargeSmsResult_t GetValue() const;
    //return general nonzero code holding RP cause or CAP3 error, or protocol error
    uint32_t          GetErrorCode() const;
    //return nonzero RP cause MO SM transfer
    uint8_t           GetRPCause() const;
    //return nonzero CAP3 error code
    uint32_t          GetCAP3Error() const;
    virtual void handle(SmscHandler*);

protected:
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);

private:
    ChargeSmsResult_t   value;
    uint32_t            rPC;  //RP cause MO SM transfer [1.255], or CAP3 error
};

class DeliverySmsResult : public InmanCommand
{
public:
    DeliverySmsResult();
    DeliverySmsResult(DeliverySmsResult_t);
    virtual ~DeliverySmsResult();

    DeliverySmsResult_t GetValue() const;

    void export2CDR(CDRRecord & cdr);
    //InmanCommand interface
    void handle(InmanHandler*);

protected:
    //SerializableObject interface
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);

private:
    DeliverySmsResult_t value;
    //optional data for CDR generation (on successfull delivery)
    std::string   destImsi;
    std::string   destMSC;
    std::string   destSMEid;
    time_t        finalTimeTZ;
};

class InmanHandler
{
public:
    virtual void onChargeSms(ChargeSms* sms) = 0;
    virtual void onDeliverySmsResult(DeliverySmsResult* sms) = 0;
};

class SmscHandler
{
public:
    virtual void onChargeSmsResult(ChargeSmsResult* sms) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_MESSAGES__ */

