#ident "$Id$"
#ifndef _SMSC_INMAN_CDR_UTIL_H
#define _SMSC_INMAN_CDR_UTIL_H

namespace smsc  {
namespace inman {
namespace cdr {

struct CDRRecord {
    CDRRecord();

    //encodes CDR to CSV format
    static void csvEncode(const CDRRecord & cdr, std::string & rec);

    typedef enum { dpOrdinary = 0, dpDiverted } CDRRecordType;
    typedef enum { dpText = 0, dpBinary } CDRMediaType;
    typedef enum { dpSMS = 0, dpUSSD } CDRBearerType;
    typedef enum { dpDeliveryOk = 0, dpDeliveryFailed = 1 } CDRDeliveryStatus;

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

    //not processed by Inman for now:                                    
    uint32_t        _dpLength;      //DATA_LENGTH: message length: in chars for dpText,
                                    //in bytes for dpBinary.
//private:
    bool            _finalized;  //all fields are fullfilled;
};


} //cdr
} //inman
} //smsc

#endif /* _SMSC_INMAN_CDR_UTIL_H */

