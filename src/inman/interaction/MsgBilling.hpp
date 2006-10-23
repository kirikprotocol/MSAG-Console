#ident "$Id$"

#ifndef __SMSC_INMAN_BILLING_MESSAGES__
#define __SMSC_INMAN_BILLING_MESSAGES__

#include "inman/interaction/messages.hpp"

#include "inman/storage/cdrutil.hpp"
using smsc::inman::cdr::CDRRecord;

#include "inman/inerrcodes.hpp"
using smsc::inman::InmanErrorCode;
using smsc::inman::InmanErrorType;

namespace smsc  {
namespace inman {
namespace interaction {
// -------------------------------------------------------------------- //
// Billing CommandSet: factory of billing commands and their subobjects
// -------------------------------------------------------------------- //
class INPCSBilling : public INPCommandSetAC {
public:
    typedef enum {
        CHARGE_SMS_TAG          = 1,    // 1. ChargeSms         ( SMSC --> INMAN ) 
        CHARGE_SMS_RESULT_TAG   = 2,    // 2. ChargeSmsResult   ( SMSC <-- INMAN ) 
        DELIVERY_SMS_RESULT_TAG = 3     // 3. DeliverySmsResult ( SMSC --> INMAN ) 
    } CommandTag;

    typedef enum { HDR_DIALOG = 1, HDR_SESSIONED_DLG = 2 } HeaderFrm;

    INPCSBilling();

    static INPCSBilling * getInstance(void)
    {
        static INPCSBilling  cmdSet;
        return &cmdSet;
    }

    INPCommandSetId CsId(void) const { return csBilling; }
    INPLoadMode loadMode(unsigned short obj_id) const { return INPCommandSetAC::lmHeader; }
};

class INPBillingCmd : public INPCommandAC {
public:
    INPBillingCmd(INPCSBilling::CommandTag cmd_tag) : INPCommandAC(cmd_tag)
    { }
    INPCommandSetAC * commandSet(void) const { return INPCSBilling::getInstance(); }
};

// --------------------------------------------------------- //
// Billing commands headers: 
// --------------------------------------------------------- // 
class CsBillingHdr_dlg : public INPHeaderAC {
public:
    uint32_t dlgId;

    CsBillingHdr_dlg();

    virtual void load(ObjectBuffer &in) throw(CustomException)  { in >> dlgId; }
    virtual void save(ObjectBuffer &out) const                  { out << dlgId; }
};

class CsBillingHdr_sess : public INPHeaderAC {
public:
    uint32_t dlgId;
    uint32_t sessId;

    CsBillingHdr_sess();

    void load(ObjectBuffer &in) throw(CustomException)  { in >> sessId; in >> dlgId; }
    void save(ObjectBuffer &out) const                  { out << sessId; out << dlgId; }
};

// --------------------------------------------------------- //
// Billing commands: 
// --------------------------------------------------------- // 

//Short message data specific for CAP3 interaction
struct SMCAPSpecificInfo {
    std::string   smscAddress;
    unsigned char tpShortMessageSpecificInfo;
    unsigned char tpProtocolIdentifier;
    unsigned char tpDataCodingScheme;
    time_t        tpValidityPeriod;
};

class ChargeSms : public INPBillingCmd {
public:
    ChargeSms();
    virtual ~ChargeSms() { }

    void setForwarded(bool isFwd = true)
        { forwarded = isFwd; }
    //data for CDR generation & CAP interaction
    void setDestinationSubscriberNumber(const std::string& dst_adr)
        { dstSubscriberNumber = dst_adr; }
    void setCallingPartyNumber(const std::string& src_adr)
        { callingPartyNumber = src_adr; }
    void setCallingIMSI(const std::string& imsi)
        { callingImsi = imsi; }
    void setSubmitTimeTZ(time_t tmVal)
        { submitTimeTZ = tmVal; }
    void setLocationInformationMSC(const std::string& src_msc)
        { locationInformationMSC = src_msc; }
    void setCallingSMEid(const std::string & sme_id)
        { callingSMEid = sme_id; }
    void setRouteId(const std::string & route_id)
        { routeId = route_id; }
    void setServiceId(int32_t service_id)   { serviceId = service_id; }
    void setUserMsgRef(uint32_t msg_ref)    { userMsgRef = msg_ref; }
    void setMsgId(uint64_t msg_id)          { msgId = msg_id; }
    void setServiceOp(int32_t service_op)   { ussdServiceOp = service_op; }
    void setPartsNum(uint8_t parts_num)     { partsNum = parts_num; }
    void setMsgLength(uint16_t msg_len)     { msgLen = msg_len; }
    //data for CAP3 InitialDP OPERATION
    void setSMSCAddress(const std::string& smsc_adr)
        { csInfo.smscAddress = smsc_adr; }
    void setTPShortMessageSpecificInfo(unsigned char sm_info)
        { csInfo.tpShortMessageSpecificInfo = sm_info; }
    void setTPProtocolIdentifier(unsigned char prot_id)
        { csInfo.tpProtocolIdentifier = prot_id; }
    void setTPDataCodingScheme(unsigned char dcs)
        { csInfo.tpDataCodingScheme = dcs; }
    void setTPValidityPeriod(time_t vpVal)
        { csInfo.tpValidityPeriod = vpVal; }

#ifdef SMSEXTRA
    void setSmsXSrvs(uint32_t srv_ids)     { extCode |= 0x80; smsXSrvsId = srv_ids; }
#endif /* SMSEXTRA */

    void export2CDR(CDRRecord & cdr) const;
    void exportCAPInfo(SMCAPSpecificInfo & csi) const { csi = csInfo; }
    uint32_t getSmsXSrvs(void) const { return smsXSrvsId; }

protected:
    //SerializableObject interface
    void load(ObjectBuffer& in) throw(CustomException);
    void save(ObjectBuffer& out) const;

private:
    bool          forwarded;    //rescheduled delivery attempt
    //data for CDR generation
    std::string   dstSubscriberNumber;
    std::string   callingPartyNumber;
    std::string   callingImsi;
    time_t        submitTimeTZ;
    std::string   locationInformationMSC; //keeps SRC_MSC
    std::string   callingSMEid; //"MAP_PROXY"
    std::string   routeId;      //"sibinco.sms > plmn.kem"
    int32_t       serviceId;    //
    int32_t       userMsgRef;   //negative if absent
    uint64_t      msgId;        //
    int32_t       ussdServiceOp; //see sms_const.h
    uint8_t       partsNum;     //number of parts if packet was conjoined.
    uint16_t      msgLen;       //total length of message(including multipart case)
    SMCAPSpecificInfo csInfo;
    //
    unsigned char extCode;      //extension fields are present
    uint32_t      smsXSrvsId;
};

//NOTE: in case of CAP3 error, this command ends the TCP dialog.
class ChargeSmsResult : public INPBillingCmd, public InmanErrorCode {
public:
    typedef enum { CHARGING_POSSIBLE = 0, CHARGING_NOT_POSSIBLE } ChargeSmsResult_t;

    ChargeSmsResult();                //positive result, no error
    ChargeSmsResult(uint32_t errCode, ChargeSmsResult_t res = CHARGING_NOT_POSSIBLE);
    ChargeSmsResult(InmanErrorType errType, uint16_t errCode,
                    ChargeSmsResult_t res = CHARGING_NOT_POSSIBLE);
    
    virtual ~ChargeSmsResult() { }

    ChargeSmsResult_t GetValue() const { return value; }
    void    SetValue(ChargeSmsResult_t res = CHARGING_NOT_POSSIBLE) { value = res; }

protected:
    virtual void load(ObjectBuffer& in) throw(CustomException);
    virtual void save(ObjectBuffer& out) const;

private:
    ChargeSmsResult_t   value;
};

class DeliverySmsResult : public INPBillingCmd {
public:
    DeliverySmsResult();    //constructor for successfull delivery 
    DeliverySmsResult(uint32_t, bool finalAttemp = true);
    virtual ~DeliverySmsResult() { }

    void setResultValue(uint32_t res)               { value = res; }
    void setFinal(bool final_attemp = true)         { final = final_attemp; }
    void setDestIMSI(const std::string& imsi)       { destImsi = imsi; }
    void setDestMSC(const std::string& msc)         { destMSC = msc; }
    void setDestSMEid(const std::string& sme_id)    { destSMEid = sme_id; }
    void setDivertedAdr(const std::string& dvrt_adr) { divertedAdr = dvrt_adr; }
    void setDeliveryTime(time_t final_tm)           { finalTimeTZ = final_tm; }

    uint32_t GetValue() const { return value; }

    void export2CDR(CDRRecord & cdr) const;

protected:
    //SerializableObject interface
    virtual void load(ObjectBuffer& in) throw(CustomException);
    virtual void save(ObjectBuffer& out) const;

private:
    uint32_t      value;    //0, or errorcode
    bool          final;    //successfull delivery or last delivery attempt,
                            //enforces CDR generation
    //optional data for CDR generation (on successfull delivery)
    std::string   destImsi;
    std::string   destMSC;
    std::string   destSMEid;
    std::string   divertedAdr;
    time_t        finalTimeTZ;
};

// --------------------------------------------------------- //
// Solid instances of Billing packets:
// --------------------------------------------------------- //
typedef INPSolidPacketT<CsBillingHdr_dlg, ChargeSms>         SPckChargeSms;
typedef INPSolidPacketT<CsBillingHdr_dlg, ChargeSmsResult>   SPckChargeSmsResult;
typedef INPSolidPacketT<CsBillingHdr_dlg, DeliverySmsResult> SPckDeliverySmsResult;

// --------------------------------------------------------- //
// Billing command handlers:
// --------------------------------------------------------- //
class INPBillingHandlerITF {
public:
    virtual bool onChargeSms(ChargeSms* sms, CsBillingHdr_dlg *hdr) = 0;
    virtual void onDeliverySmsResult(DeliverySmsResult* sms_res, CsBillingHdr_dlg *hdr) = 0;
};

class SMSCBillingHandlerITF {
public:
    virtual void onChargeSmsResult(ChargeSmsResult* chg_res, CsBillingHdr_dlg *hdr) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_BILLING_MESSAGES__ */

