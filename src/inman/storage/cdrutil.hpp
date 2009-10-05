/* ************************************************************************* *
 * INMan CSV file record definition. Contains all required info for CDR
 * generation (external postprocessing, out of INMan scope).
 * ************************************************************************* */
#ifndef _SMSC_INMAN_CDR_UTIL_H
#ident "@(#)$Id$"
#define _SMSC_INMAN_CDR_UTIL_H

#include <string>

namespace smsc  {
namespace inman {
namespace cdr {

extern const char    _CDRRecordHeader_TEXT[];

struct CDRRecord {
    CDRRecord();

    //encodes CDR to CSV format
    static void csvEncode(const CDRRecord & cdr, std::string & rec);

    enum ChargingPolicy {
        ON_SUBMIT = 0       //
      , ON_DELIVERY         //
      , ON_DATA_COLLECTED   //
      , ON_SUBMIT_COLLECTED // 
    };
    enum ChargingType {
        MO_Charge = 0   //DP originator is charged
      , MT_Charge = 1   //DP receiver is charged
    };
    enum CDRRecordType { dpOrdinary = 0, dpDiverted };
    enum CDRMediaType { dpText = 0, dpBinary };
    enum CDRBearerType { dpSMS = 0, dpUSSD };
    enum CDRDeliveryStatus { dpDeliveryOk = 0, dpDeliveryFailed = 1 };
    //this one is the same as smsc::inman::AbonentContractInfo::ContractType
    enum ContractType { abtUnknown = 0, abtPostpaid = 1, abtPrepaid = 2 };

    std::string dpType(void) const;

    //basic info:
    uint64_t        _msgId;         //MSG_ID: system message identifier
    CDRRecordType   _cdrType;       //RECORD_TYPE: 
    CDRMediaType    _mediaType;     //MEDIA_TYPE: text or binary
    CDRBearerType   _bearer;        //BEARER_TYPE: sms or ussd
    std::string     _routeId;       //ROUTE_ID:
    int32_t         _serviceId;     //SERVICE_ID: id of service on route,
                                    //in case of SMS Extra - id of pack of extra services
    int32_t         _userMsgRef;    //USER_MSG_REF: system identifier for dialog tracing
    uint8_t         _partsNum;      //number of parts if packet was conjoined.
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
    uint32_t        _dlvrRes;       //STATUS: delivery status: 0 or error code
    std::string     _divertedAdr;   //DIVERTED_FOR: if cdrType == dpDiverted, keeps
                                    //destination number to which delivery was made 
                                    //that differs from original destination address.

    uint32_t        _dpLength;      //DATA_LENGTH: message length: in chars for dpText,
                                    //in bytes for dpBinary.
    uint32_t        _smsXMask;      //mask of pack of SMS Extra services
    ContractType    _contract;      //sender contract type
    ChargingPolicy  _chargePolicy;  //
    bool            _inBilled;      //message was billed by IN platform
    std::string     _dsmSrvType;    //SMPP DATA_SM service type
//private: not written to CSV
    ChargingType    _chargeType;    //MO or MT charging
    enum Phase_e {      //indicates which fields are fullfilled;
        dpEmpty = 0,
        dpSubmitted,    //DP was submitted, sender description present
        dpDelivered,    //there was an attempt to delivery DP, receiver
                        //description may present
        dpCollected     //DP data collected (final delivery attempt)
    }               _finalized;
};


} //cdr
} //inman
} //smsc

#endif /* _SMSC_INMAN_CDR_UTIL_H */

