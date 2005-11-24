static char const ident[] = "$Id$";

#include "util/csv/CSVFileEncoder.h"
#include "inman/storage/cdrutil.hpp"

using smsc::util::csv::CSVFileEncoder;

namespace smsc  {
namespace inman {
namespace cdr {

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
    CSVFileEncoder::addUint8 (rec, cdr._partsNum, true);
}


} //cdr
} //inman
} //smsc


