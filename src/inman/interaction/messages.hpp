#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "inman/interaction/serializer.hpp"
#include "inman/common/factory.hpp"
#include "inman/storage/cdrutil.hpp"

using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::SerializerITF;
using smsc::inman::common::FactoryT;
using smsc::inman::cdr::CDRRecord;

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


class ChargeSms : public InmanCommand
{
public:
    ChargeSms();
    virtual ~ChargeSms();

    void setDestinationSubscriberNumber(const std::string& dst_adr);
    void setCallingPartyNumber(const std::string& src_adr);
    void setCallingIMSI(const std::string& imsi);
    void setSMSCAddress(const std::string& smsc_adr);
    void setSubmitTimeTZ(time_t tmVal);
    void setTPShortMessageSpecificInfo(unsigned char sm_info);
    void setTPProtocolIdentifier(unsigned char prot_id);
    void setTPDataCodingScheme(unsigned char dcs);
    void setTPValidityPeriod(time_t vpVal);
    void setLocationInformationMSC(const std::string& src_msc);
    //data for CDR generation
    void setCallingSMEid(const std::string & sme_id);
    void setRouteId(const std::string & route_id);
    void setServiceId(int32_t service_id);
    void setUserMsgRef(uint32_t msg_ref);
    void setMsgId(uint64_t msg_id);
    void setServiceOp(int32_t service_op);
    //data for InitialDP OPERATION
    const std::string & getDestinationSubscriberNumber(void) const;
    const std::string & getCallingPartyNumber(void) const;
    const std::string & getCallingIMSI(void) const;
    const std::string & getSMSCAddress(void) const;
    const std::string & getLocationInformationMSC(void) const;
    time_t              getSubmitTimeTZ(void) const;
    time_t              getTPValidityPeriod(void) const;
    unsigned char       getTPShortMessageSpecificInfo(void) const;
    unsigned char       getTPProtocolIdentifier(void) const;
    unsigned char       getTPDataCodingScheme(void) const;

    void export2CDR(CDRRecord & cdr) const;
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
    int32_t       serviceId;    //
    int32_t       userMsgRef;   //negative if absent
    uint64_t      msgId;        //
    int32_t       ussdServiceOp; //see sms_const.h
};

//NOTE: in case of CAP3 error, this command ends the TCP dialog.
class ChargeSmsResult : public SmscCommand
{
public:
    typedef enum { chgOk = 0, chgRPCause = 1, chgTCPerror, chgCAP3error} ChargeErrorClass;
    ChargeSmsResult();                //positive result, no error
    ChargeSmsResult(uint32_t rPCcode); //negative result by RP cause or CAP3 error
    virtual ~ChargeSmsResult();

    ChargeSmsResult_t GetValue() const;

    ChargeErrorClass  GetErrorClass(void) const;
    //return combined nonzero code holding RP cause or CAP3 error, or protocol error
    uint32_t          GetErrorCode(void) const;
    //return nonzero RP cause MO SM transfer
    uint8_t           GetRPCause(void) const;
    //return nonzero CAP3 error code
    uint32_t          GetCAP3Error(void) const;
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

    void setResultValue(DeliverySmsResult_t res);
    void setDestIMSI(const std::string& imsi);
    void setDestMSC(const std::string& msc);
    void setDestSMEid(const std::string& sme_id);
    void setDeliveryTime(time_t final_tm);

    DeliverySmsResult_t GetValue() const;

    void export2CDR(CDRRecord & cdr) const;
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

