/* ************************************************************************* *
 * INMan CSV file record definition. Contains all required info for CDR
 * generation (external postprocessing, out of INMan scope).
 * ************************************************************************* */
#ifndef _SMSC_INMAN_CDR_UTIL_H
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _SMSC_INMAN_CDR_UTIL_H

#include "util/TonNpiAddress.hpp"
#include "inman/CDRTypes.hpp"

namespace smsc  {
namespace inman {
namespace cdr {

using smsc::util::TonNpiAddressString;
using smsc::util::IMSIString;
using smsc::inman::SMESysId;
using smsc::inman::SMPPServiceType;

extern const char    _CDRRecordHeader_TEXT[];

const unsigned MAX_DPTypeStringLength = (sizeof("dpUSSD(X:%Xh)") + sizeof("FFFFFFFF") + 2);

typedef smsc::core::buffers::FixedLengthString<MAX_DPTypeStringLength> DPTypeString;

struct CDRRecord {
    CDRRecord();
    
    //encodes CDR to CSV format
    static void csvEncode(const CDRRecord & cdr, std::string & rec);

    enum ChargingPolicy {
        ON_SUBMIT = 0           //
      , ON_DELIVERY = 1         //
      , ON_DATA_COLLECTED = 2   //
      , ON_SUBMIT_COLLECTED = 3 //
    };

    static const char * nmPolicy(ChargingPolicy use_val);

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

    DPTypeString dpType(void) const;
    const char * nmPolicy(void) const { return nmPolicy(_chargePolicy); }

    //basic info:
    uint64_t            _msgId;         //MSG_ID: system message identifier
    CDRRecordType       _cdrType;       //RECORD_TYPE: 
    CDRMediaType        _mediaType;     //MEDIA_TYPE: text or binary
    CDRBearerType       _bearer;        //BEARER_TYPE: sms or ussd
    SMRouteId           _routeId;       //ROUTE_ID:
    int32_t             _serviceId;     //SERVICE_ID: id of service on route,
                                        //in case of SMS Extra - id of pack of extra services
    int32_t             _userMsgRef;    //USER_MSG_REF: system identifier for dialog tracing
    uint8_t             _partsNum;      //number of parts if packet was conjoined.
    //sender info
    time_t              _submitTime;    //SUBMIT: sms submit time
    TonNpiAddressString _srcAdr;        //SRC_ADDR: sender number
    IMSIString          _srcIMSI;       //SRC_IMSI: sender IMSI
    TonNpiAddressString _srcMSC;        //SRC_MSC: sender MSC: either signals (optionally
                                        //  preceeded by ToN & NPi = ".1.1.")
                                        //  or alphanumeric string)
    SMESysId            _srcSMEid;      //SRC_SME_ID: sender SME identifier
    //recipient info
    time_t              _finalTime;     //FINALIZED: sms delivery time
    TonNpiAddressString _dstAdr;        //DST_ADDR: final recipient number
    IMSIString          _dstIMSI;       //DST_IMSI: recipient IMSI
    TonNpiAddressString _dstMSC;        //DST_MSC: recipient MSC: either signals (optionally
                                        //  preceeded by ToN & NPi = ".1.1.")
                                        //  or alphanumeric string)
    SMESysId            _dstSMEid;      //DST_SME_ID: recipient SME identifier
    uint32_t            _dlvrRes;       //STATUS: delivery status: 0 or error code
    TonNpiAddressString _divertedAdr;   //DIVERTED_FOR: if cdrType == dpDiverted, keeps
                                        //destination number to which delivery was made 
                                        //that differs from original destination address.

    uint32_t            _dpLength;      //DATA_LENGTH: message length: in chars for dpText,
                                        //in bytes for dpBinary.
    uint32_t            _smsXMask;      //mask of pack of SMS Extra services
    ContractType        _contract;      //sender contract type
    ChargingPolicy      _chargePolicy;  //
    bool                _inBilled;      //message was billed by IN platform
    SMPPServiceType     _dsmSrvType;    //SMPP DATA_SM service type
    ChargingType        _chargeType;    //MO or MT charging

//private: not written to CSV
    enum Phase_e {      //indicates which fields are fullfilled;
        dpEmpty = 0,
        dpSubmitted,    //DP was submitted, sender description present
        dpDelivered,    //there was an attempt to delivery DP, recipient
                        //description may present
        dpCollected     //DP data collected (final delivery attempt)
    }                   _finalized;
};


} //cdr
} //inman
} //smsc

#endif /* _SMSC_INMAN_CDR_UTIL_H */

