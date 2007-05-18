#ident "$Id$"
#ifndef _SMSC_INMAN_CDR_UTIL_H
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

    enum ChargingMode { ON_SUBMIT = 0, ON_DELIVERY, ON_DATA_COLLECTED };
    typedef enum { dpOrdinary = 0, dpDiverted } CDRRecordType;
    typedef enum { dpText = 0, dpBinary } CDRMediaType;
    typedef enum { dpSMS = 0, dpUSSD } CDRBearerType;
    typedef enum { dpDeliveryOk = 0, dpDeliveryFailed = 1 } CDRDeliveryStatus;
    //this one is the same as smsc::inman::AbonentContractInfo::ContractType
    enum ContractType { abtUnknown = 0, abtPostpaid = 1, abtPrepaid = 2 };

    std::string dpType(void) const;

    //basic info:
    uint64_t        _msgId;         //MSG_ID: system message identifier
    CDRRecordType   _cdrType;       //RECORD_TYPE: 
    CDRMediaType    _mediaType;     //MEDIA_TYPE: text or binary
    CDRBearerType   _bearer;        //BEARER_TYPE: sms or ussd
    std::string     _routeId;       //ROUTE_ID:
    int32_t         _serviceId;     //SERVICE_ID: set on route
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
                                    //original destination number that differs from
                                    //address to which delivery was made.

    uint32_t        _dpLength;      //DATA_LENGTH: message length: in chars for dpText,
                                    //in bytes for dpBinary.
    uint32_t        _smsXSrvs;      //SMS Extra serviceCode
    ContractType    _contract;      // 
    ChargingMode    _charge;        //
    bool            _inBilled;      //message was billed by IN platform                                    
//private: not written to CSV
    enum {              //indicates which fields are fullfilled;
        dpEmpty = 0,
        dpSubmitted,    //DP was submitted,
        dpDelivered,    //there was an attempt to delivery DP 
        dpCollected     //DP data collected (final delivery attempt)
    }               _finalized;
    uint32_t        _smsXMask;      //SMS Extra services mask
};


} //cdr
} //inman
} //smsc

#endif /* _SMSC_INMAN_CDR_UTIL_H */

