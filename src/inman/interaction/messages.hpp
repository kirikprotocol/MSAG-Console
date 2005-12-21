#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "inman/interaction/serializer.hpp"
#include "inman/common/factory.hpp"
#include "inman/storage/cdrutil.hpp"
#include "inman/interaction/inerrcodes.hpp"

using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::SerializerITF;
using smsc::inman::interaction::SerializerException;
using smsc::inman::common::FactoryT;
using smsc::inman::cdr::CDRRecord;
using smsc::inman::interaction::SerializerITF;

using smsc::inman::InmanErrorCode;
using smsc::inman::InmanErrorType;

namespace smsc  {
namespace inman {
namespace interaction {

//Serializer for Inman commands, transferred over TCP socket
//SerializerInap implements two deserialization modes:
// - complete: both the packet and contained object are deserialized
// - partial:  only packet prefix is parsed(required to identify object), remaining data
//             is set as object data buffer.
class SerializerInap : public SerializerITF, public FactoryT< unsigned short, SerializableObject >
{
public:
    enum { FORMAT_VERSION = 0x0001 };
    typedef enum  { dsrlPartial = 0, dsrlComplete } DeserializationMode;

    virtual ~SerializerInap();

    //SerializerITF interface:
    SerializableObject* deserialize(ObjectBuffer& in) throw(CustomException); 
    SerializableObject* deserializeAndOwn(ObjectBuffer* in, bool ownBuf = true) throw(CustomException); 
    void                serialize(SerializableObject*, ObjectBuffer& out);

    static SerializerInap* getInstance();

protected:
    SerializerInap();
    SerializableObject* deserializePrefix(ObjectBuffer& in) throw(CustomException);

    DeserializationMode _mode;
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
    void setPartsNum(uint8_t parts_num);
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
    void load(ObjectBuffer& in) throw(CustomException);
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
    uint8_t       partsNum;     //number of parts if packet was conjoined.
};

//NOTE: in case of CAP3 error, this command ends the TCP dialog.
class ChargeSmsResult : public SmscCommand, public InmanErrorCode
{
public:
    ChargeSmsResult();                //positive result, no error
    ChargeSmsResult(uint32_t errCode, ChargeSmsResult_t res = CHARGING_NOT_POSSIBLE);
    ChargeSmsResult(InmanErrorType errType, uint16_t errCode,
                    ChargeSmsResult_t res = CHARGING_NOT_POSSIBLE);
    
    virtual ~ChargeSmsResult();

    ChargeSmsResult_t GetValue() const;

    virtual void handle(SmscHandler*);

protected:
    virtual void load(ObjectBuffer& in) throw(CustomException);
    virtual void save(ObjectBuffer& out);

private:
    ChargeSmsResult_t   value;
};

class DeliverySmsResult : public InmanCommand
{
public:
    DeliverySmsResult();    //constructor for successfull delivery 
    DeliverySmsResult(DeliverySmsResult_t, bool finalAttemp = true);
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
    virtual void load(ObjectBuffer& in) throw(CustomException);
    virtual void save(ObjectBuffer& out);

private:
    DeliverySmsResult_t value;
    bool          final;    //successfull delivery or last delivery attempt,
                            //enforces CDR generation
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

