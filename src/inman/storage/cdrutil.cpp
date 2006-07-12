static char const ident[] = "$Id$";

#include "util/csv/CSVFileEncoder.h"
#include "inman/storage/cdrutil.hpp"

using smsc::util::csv::CSVFileEncoder;

namespace smsc  {
namespace inman {
namespace cdr {

extern const char    _CDRRecordHeader_TEXT[] =
    "MSG_ID,RECORD_TYPE,MEDIA_TYPE,BEARER_TYPE,SUBMIT,FINALIZED,STATUS,"
    "SRC_ADDR,SRC_IMSI,SRC_MSC,SRC_SME_ID,DST_ADDR,DST_IMSI,DST_MSC,DST_SME_ID,"
    "DIVERTED_FOR,ROUTE_ID,SERVICE_ID,USER_MSG_REF,DATA_LENGTH,PARTS_NUM,IN_BILL\n";

/* ************************************************************************** *
 * class CDRRecord implementation:
 * ************************************************************************** */
CDRRecord::CDRRecord()
    : _finalized(false), _msgId(0), _partsNum(1)
    , _cdrType(CDRRecord::dpOrdinary), _mediaType(CDRRecord::dpText)
    , _bearer(CDRRecord::dpSMS), _dlvrRes(CDRRecord::dpDeliveryFailed)
{
    _serviceId = _userMsgRef = _dpLength = 0;
    _submitTime = _finalTime = 0;
    _inBilled = false;
}

void CDRRecord::csvEncode(const CDRRecord & cdr, std::string & rec)
{
    rec = "";
    CSVFileEncoder::addUint64(rec, cdr._msgId);
    CSVFileEncoder::addUint8 (rec, cdr._cdrType);
    CSVFileEncoder::addUint8 (rec, cdr._mediaType);
    CSVFileEncoder::addUint8 (rec, cdr._bearer);
    CSVFileEncoder::addDateTime(rec, cdr._submitTime);
    CSVFileEncoder::addDateTime(rec, cdr._finalTime);
    CSVFileEncoder::addUint32(rec, cdr._dlvrRes);
    
    CSVFileEncoder::addString(rec, cdr._srcAdr.c_str());
    CSVFileEncoder::addString(rec, cdr._srcIMSI.c_str());
    CSVFileEncoder::addString(rec, cdr._srcMSC.c_str());
    CSVFileEncoder::addString(rec, cdr._srcSMEid.c_str());
    
    CSVFileEncoder::addString(rec, cdr._dstAdr.c_str());
    CSVFileEncoder::addString(rec, cdr._dstIMSI.c_str());
    CSVFileEncoder::addString(rec, cdr._dstMSC.c_str());
    CSVFileEncoder::addString(rec, cdr._dstSMEid.c_str());

    if (cdr._cdrType == dpDiverted)
        CSVFileEncoder::addString(rec, cdr._divertedAdr.c_str());
    else
        CSVFileEncoder::addString(rec, 0);
    CSVFileEncoder::addString(rec, cdr._routeId.c_str());
    CSVFileEncoder::addInt32 (rec, cdr._serviceId);
    if (cdr._userMsgRef < 0)
        CSVFileEncoder::addSeparator(rec);
    else
        CSVFileEncoder::addUint32(rec, (uint32_t)(cdr._userMsgRef));
        
    CSVFileEncoder::addUint32(rec, cdr._dpLength);
    CSVFileEncoder::addUint8 (rec, cdr._partsNum, false);
    CSVFileEncoder::addUint8 (rec, cdr._inBilled ? 0x01 : 0x00, true);
}


} //cdr
} //inman
} //smsc


