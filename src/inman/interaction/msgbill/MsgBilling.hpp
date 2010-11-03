/* ************************************************************************* *
 * INMan SMS/USSD messages charging protocol PDUs definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_BILLING_MESSAGES__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_BILLING_MESSAGES__

#include "inman/interaction/messages.hpp"

#include "inman/storage/cdrutil.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::util::TonNpiAddressString;

using smsc::util::IMSIString;
using smsc::inman::SMESysId;
using smsc::inman::SMRouteId;

using smsc::inman::cdr::CDRRecord;

// -------------------------------------------------------------------- //
// Billing CommandSet: factory of billing commands and their subobjects
// -------------------------------------------------------------------- //
class INPCSBilling : public INPCommandSetAC { //singleton
protected:
  INPCSBilling();
  ~INPCSBilling()
  { }

public:
  enum CommandTag {
    CHARGE_SMS_TAG          = 1,    // 1. ChargeSms         ( SMSC --> INMAN )
    CHARGE_SMS_RESULT_TAG   = 2,    // 2. ChargeSmsResult   ( SMSC <-- INMAN )
    DELIVERY_SMS_RESULT_TAG = 3,    // 3. DeliverySmsResult ( SMSC --> INMAN )
    DELIVERED_SMS_DATA_TAG  = 4     // 1. DeliveredSmsData  ( SMSC --> INMAN )
  };
  enum HeaderFrm {
    HDR_DIALOG = 1, HDR_SESSIONED_DLG = 2
  };

  static INPCSBilling * getInstance(void);

  // ----------------------------------------
  // -- INPCommandSetAC interface methods
  // ----------------------------------------
  virtual INPLoadMode loadMode(unsigned short obj_id) const
  {
    return INPCommandSetAC::lmHeader;
  }
};

class INPBillingCmd : public INPCommandAC {
protected:
  INPBillingCmd(INPCSBilling::CommandTag cmd_tag) : INPCommandAC(cmd_tag)
  { }

public:
  virtual ~INPBillingCmd()
  { }

  // ----------------------------------
  // -- INPCommandAC interface methods
  // ----------------------------------
  virtual const INPCommandSetAC * commandSet(void) const
  {
    return INPCSBilling::getInstance();
  }
};

// --------------------------------------------------------- //
// Billing commands headers: 
// --------------------------------------------------------- // 
class CsBillingHdr_dlg : public INPHeaderAC {
public:
    uint32_t dlgId;

    CsBillingHdr_dlg();

    virtual void load(ObjectBuffer &in) throw(SerializerException)  { in >> dlgId; }
    virtual void save(ObjectBuffer &out) const                      { out << dlgId; }
};

class CsBillingHdr_sess : public INPHeaderAC {
public:
    uint32_t dlgId;
    uint32_t sessId;

    CsBillingHdr_sess();

    void load(ObjectBuffer &in) throw(SerializerException)  { in >> sessId; in >> dlgId; }
    void save(ObjectBuffer &out) const                      { out << sessId; out << dlgId; }
};

// --------------------------------------------------------- //
// Billing commands: 
// --------------------------------------------------------- // 

//Short message data specific for CAP3 interaction
struct SMCAPSpecificInfo {
  TonNpiAddressString smscAddress;
  unsigned char       tpShortMessageSpecificInfo;
  unsigned char       tpProtocolIdentifier;
  unsigned char       tpDataCodingScheme;
  time_t              tpValidityPeriod;
};

class ChargeSms : public INPBillingCmd {
protected:
  //SerializableObject interface
  void load(ObjectBuffer& in) throw(SerializerException);
  void save(ObjectBuffer& out) const;

private:
  CDRRecord::ChargingPolicy chrgPolicy; //
  //data for CDR generation
  TonNpiAddressString dstSubscriberNumber;
  TonNpiAddressString callingPartyNumber;
  IMSIString          callingImsi;
  time_t              submitTimeTZ;
  TonNpiAddressString locationInformationMSC; //keeps SRC_MSC
  SMESysId            callingSMEid; //"MAP_PROXY"
  SMRouteId           routeId;      //"sibinco.sms > plmn.kem"
  int32_t             serviceId;    //
  int32_t             userMsgRef;   //negative if absent
  uint64_t            msgId;        //
  int32_t             ussdServiceOp; //see sms_const.h
  uint8_t             partsNum;     //number of parts if packet was conjoined.
  uint16_t            msgLen;       //total length of message(including multipart case)
  SMCAPSpecificInfo   csInfo;
  //
  uint32_t            smsXSrvsId;
  uint8_t             chrgFlags;    //flags which customize billing settings
  SMPPServiceType     dsmSrvType;   //SMPP DATA_SM service type

public:
    enum ChargingFlags_e {
        chrgMT = 0x01   //charge the dstSubscriber instead of calling one
      , chrgCDR = 0x02  //force charging via CDR despite of subscriber contract
    };
    ChargeSms(); //by default: charging policy is ON_DELIVERY, charging type is MO
    virtual ~ChargeSms() { }

    void setMTcharge(void)
        { chrgFlags |= chrgMT; }
    void setForcedCDR(void)
        { chrgFlags |= chrgCDR; }
    void setChargeOnSubmit(void)
        { chrgPolicy = CDRRecord::ON_SUBMIT; }
    void setSmsXSrvs(uint32_t srv_ids)
        { smsXSrvsId = srv_ids; }
    //data for CDR generation & CAP interaction
    void setDestinationSubscriberNumber(const char * dst_adr)
        { dstSubscriberNumber = dst_adr; }
    void setCallingPartyNumber(const char * src_adr)
        { callingPartyNumber = src_adr; }
    void setCallingIMSI(const char * imsi)
        { callingImsi = imsi; }
    void setSubmitTimeTZ(time_t tmVal)
        { submitTimeTZ = tmVal; }
    //sets SRC_MSC address (either signals (optionally preceeded by ToN & NPi = ".1.1.")
    //or alphanumeric string)
    void setLocationInformationMSC(const char * src_msc)
        { locationInformationMSC = src_msc; }
    void setCallingSMEid(const char * sme_id)
        { callingSMEid = sme_id; }
    void setRouteId(const char * route_id)
        { routeId = route_id; }
    void setServiceId(int32_t service_id)   { serviceId = service_id; }
    //sets SMPP DATA_SM service type
    void setServiceType(const char * service_type)
                                            { dsmSrvType = service_type; }
    void setUserMsgRef(uint32_t msg_ref)    { userMsgRef = msg_ref; }
    void setMsgId(uint64_t msg_id)          { msgId = msg_id; }
    void setServiceOp(int32_t service_op)   { ussdServiceOp = service_op; }
    void setPartsNum(uint8_t parts_num)     { partsNum = parts_num; }
    void setMsgLength(uint16_t msg_len)     { msgLen = msg_len; }
    //data for CAP3 InitialDP OPERATION
    void setSMSCAddress(const char * smsc_adr)
        { csInfo.smscAddress = smsc_adr; }
    void setTPShortMessageSpecificInfo(unsigned char sm_info)
        { csInfo.tpShortMessageSpecificInfo = sm_info; }
    void setTPProtocolIdentifier(unsigned char prot_id)
        { csInfo.tpProtocolIdentifier = prot_id; }
    void setTPDataCodingScheme(unsigned char dcs)
        { csInfo.tpDataCodingScheme = dcs; }
    void setTPValidityPeriod(time_t vpVal)
        { csInfo.tpValidityPeriod = vpVal; }

    void export2CDR(CDRRecord & cdr) const;
    void exportCAPInfo(SMCAPSpecificInfo & csi) const { csi = csInfo; }
    uint32_t getSmsXSrvs(void) const { return smsXSrvsId; }
    uint8_t  getChargingFlags(void) const { return chrgFlags; }
};

//NOTE: in case of CAP3 error, this command ends the TCP dialog.
class ChargeSmsResult : public INPBillingCmd {
public:
    enum ChargeSmsResult_e { CHARGING_POSSIBLE = 0, CHARGING_NOT_POSSIBLE };

    ChargeSmsResult();                //positive result, no error
    ChargeSmsResult(ChargeSmsResult_e res, uint32_t err_code = 0,
                    const char * err_msg = NULL);
    
    virtual ~ChargeSmsResult() { }

    ChargeSmsResult_e GetValue(void) const { return value; }
    CDRRecord::ContractType getContract(void) const { return contract; }
    uint32_t          getError(void) const { return errCode; }
    const char *      getMsg(void)   const { return errMsg.c_str(); }

    void   setValue(ChargeSmsResult_e res = CHARGING_NOT_POSSIBLE) { value = res; }
    void   setContract(CDRRecord::ContractType abn_contract) { contract = abn_contract; }
    void   setError(uint32_t err_code, const char * err_msg = NULL)
    {
        errCode = err_code;
        if (err_msg)
            errMsg = err_msg;
    }

protected:
    virtual void load(ObjectBuffer& in) throw(SerializerException);
    virtual void save(ObjectBuffer& out) const;

private:
    ChargeSmsResult_e   value;
    uint32_t            errCode;
    std::string         errMsg;
    CDRRecord::ContractType contract;
};

class DeliverySmsResult : public INPBillingCmd {
protected:
    //SerializableObject interface
    virtual void load(ObjectBuffer& in) throw(SerializerException);
    virtual void save(ObjectBuffer& out) const;

private:
    uint32_t      value;    //0, or errorcode
    bool          final;    //successfull delivery or last delivery attempt,
                            //enforces CDR generation
    //optional data for CDR generation (on successfull delivery)
    IMSIString          destImsi;
    TonNpiAddressString destMSC;
    SMESysId            destSMEid;
    TonNpiAddressString divertedAdr;
    time_t              finalTimeTZ;

public:
    //default constructor for successfull delivery 
    DeliverySmsResult(uint32_t res = 0, bool finalAttemp = true);
    virtual ~DeliverySmsResult() { }

    void setResultValue(uint32_t res)           { value = res; }
    void setFinal(bool final_attemp = true)     { final = final_attemp; }
    void setDestIMSI(const char * imsi)         { destImsi = imsi; }
    //sets DST_MSC address (either signals (optionally preceeded by ToN & NPi = ".1.1.")
    //or alphanumeric string)
    void setDestMSC(const char * msc)           { destMSC = msc; }
    void setDestSMEid(const char * sme_id)      { destSMEid = sme_id; }
    void setDivertedAdr(const char * dvrt_adr)  { divertedAdr = dvrt_adr; }
    void setDeliveryTime(time_t final_tm)       { finalTimeTZ = final_tm; }

    uint32_t GetValue() const { return value; }

    void export2CDR(CDRRecord & cdr) const;
};

//DP collected data: successfull delivery or last delivery attempt
//Charging mode ON_DATA_COLLECTED is assumed
class DeliveredSmsData : public INPBillingCmd {
protected:
    //SerializableObject interface
    void load(ObjectBuffer& in) throw(SerializerException);
    void save(ObjectBuffer& out) const;

private:
    //Charging request data ..
    CDRRecord::ChargingPolicy chrgPolicy; //
    TonNpiAddressString dstSubscriberNumber;
    TonNpiAddressString callingPartyNumber;
    IMSIString          callingImsi;
    time_t              submitTimeTZ;
    TonNpiAddressString locationInformationMSC; //keeps SRC_MSC
    SMESysId            callingSMEid; //"MAP_PROXY"
    SMRouteId           routeId;      //"sibinco.sms > plmn.kem"
    int32_t             serviceId;    //
    int32_t             userMsgRef;   //negative if absent
    uint64_t            msgId;        //
    int32_t             ussdServiceOp; //see sms_const.h
    uint8_t             partsNum;     //number of parts if packet was conjoined.
    uint16_t            msgLen;       //total length of message(including multipart case)
    SMCAPSpecificInfo   csInfo;
    //
    unsigned char       extCode;      //extension fields are present
    uint32_t            smsXSrvsId;
    uint8_t             chrgFlags;    //flags which customize billing settings, see ChargeSms::ChargingFlags_e
    SMPPServiceType     dsmSrvType;   //SMPP DATA_SM service type

    //Delivery report data ..
    uint32_t            dlvrRes;    //0, or errorcode
    //optional data for CDR generation (on successfull delivery)
    IMSIString          destImsi;
    TonNpiAddressString destMSC;
    SMESysId            destSMEid;
    TonNpiAddressString divertedAdr;
    time_t              finalTimeTZ;

public:
    DeliveredSmsData(uint32_t res = 0); //by default: charging type is MO
    virtual ~DeliveredSmsData(void) { }

    void setMTcharge(void)
        { chrgFlags |= ChargeSms::chrgMT; }
    void setForcedCDR(void)
        { chrgFlags |= ChargeSms::chrgCDR; }
    void setChargeOnSubmit(void)
        { chrgPolicy = CDRRecord::ON_SUBMIT_COLLECTED; }
    //data for CDR generation & CAP interaction
    void setDestinationSubscriberNumber(const char * dst_adr)
        { dstSubscriberNumber = dst_adr; }
    void setCallingPartyNumber(const char * src_adr)
        { callingPartyNumber = src_adr; }
    void setCallingIMSI(const char * imsi)
        { callingImsi = imsi; }
    void setSubmitTimeTZ(time_t tmVal)
        { submitTimeTZ = tmVal; }
    //sets SRC_MSC address (either signals (optionally preceeded by ToN & NPi = ".1.1.")
    //or alphanumeric string)
    void setLocationInformationMSC(const char * src_msc)
        { locationInformationMSC = src_msc; }
    void setCallingSMEid(const char * sme_id)
        { callingSMEid = sme_id; }
    void setRouteId(const char * route_id)
        { routeId = route_id; }
    void setServiceId(int32_t service_id)   { serviceId = service_id; }
    //sets SMPP DATA_SM service type
    void setServiceType(const char * service_type)
                                            { dsmSrvType = service_type; }
    void setUserMsgRef(uint32_t msg_ref)    { userMsgRef = msg_ref; }
    void setMsgId(uint64_t msg_id)          { msgId = msg_id; }
    void setServiceOp(int32_t service_op)   { ussdServiceOp = service_op; }
    void setPartsNum(uint8_t parts_num)     { partsNum = parts_num; }
    void setMsgLength(uint16_t msg_len)     { msgLen = msg_len; }
    //data for CAP3 InitialDP OPERATION
    void setSMSCAddress(const char * smsc_adr)
        { csInfo.smscAddress = smsc_adr; }
    void setTPShortMessageSpecificInfo(unsigned char sm_info)
        { csInfo.tpShortMessageSpecificInfo = sm_info; }
    void setTPProtocolIdentifier(unsigned char prot_id)
        { csInfo.tpProtocolIdentifier = prot_id; }
    void setTPDataCodingScheme(unsigned char dcs)
        { csInfo.tpDataCodingScheme = dcs; }
    void setTPValidityPeriod(time_t vpVal)
        { csInfo.tpValidityPeriod = vpVal; }

    void setSmsXSrvs(uint32_t srv_ids)     { extCode |= 0x80; smsXSrvsId = srv_ids; }

    //Delivery report data setters  ..
    void setResultValue(uint32_t res)           { dlvrRes = res; }
    void setDestIMSI(const char * imsi)         { destImsi = imsi; }
    //sets DST_MSC address (either signals (optionally preceeded by ToN & NPi = ".1.1.")
    //or alphanumeric string)
    void setDestMSC(const char * msc)           { destMSC = msc; }
    void setDestSMEid(const char * sme_id)      { destSMEid = sme_id; }
    void setDivertedAdr(const char * dvrt_adr)  { divertedAdr = dvrt_adr; }
    void setDeliveryTime(time_t final_tm)       { finalTimeTZ = final_tm; }

    uint32_t getResult(void) const { return dlvrRes; }
    uint8_t  getChargingFlags(void) const { return chrgFlags; }

    void export2CDR(CDRRecord & cdr) const;
    void exportCAPInfo(SMCAPSpecificInfo & csi) const { csi = csInfo; }
};


// --------------------------------------------------------- //
// Solid instances of Billing packets:
// --------------------------------------------------------- //
typedef INPSolidPacketT<CsBillingHdr_dlg, ChargeSms>         SPckChargeSms;
typedef INPSolidPacketT<CsBillingHdr_dlg, ChargeSmsResult>   SPckChargeSmsResult;
typedef INPSolidPacketT<CsBillingHdr_dlg, DeliverySmsResult> SPckDeliverySmsResult;
typedef INPSolidPacketT<CsBillingHdr_dlg, DeliveredSmsData>  SPckDeliveredSmsData;

// --------------------------------------------------------- //
// Billing command handlers:
// --------------------------------------------------------- //
class INPBillingHandlerITF {
protected:
    virtual ~INPBillingHandlerITF() //forbid interface destruction
    { }

public:
    virtual bool onChargeSms(ChargeSms* sms, CsBillingHdr_dlg *hdr) = 0;
    virtual void onDeliverySmsResult(DeliverySmsResult* sms_res, CsBillingHdr_dlg *hdr) = 0;
    virtual void onDeliveredSmsData(DeliveredSmsData* sms_dat, CsBillingHdr_dlg *hdr) = 0;
};

class SMSCBillingHandlerITF {
protected:
    virtual ~SMSCBillingHandlerITF() //forbid interface destruction
    { }

public:
    virtual void onChargeSmsResult(ChargeSmsResult* chg_res, CsBillingHdr_dlg *hdr) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_BILLING_MESSAGES__ */

