
#include "util/csv/CSVFileEncoder.h"
#include "TrapRecord.hpp"
#include <stdio.h>
using smsc::util::csv::CSVFileEncoder;

static char const ident[] = "$Id$";

namespace smsc  {
namespace snmp {

using smsc::snmp::TrapRecord;

const char    _TrapRecordHeader_TEXT[] =
    "SUBMIT_TIME,ALARM_ID,ALARMCATEGORY,SEVERITY,TEXT\n";

const char* TrapRecord::headerText() { return _TrapRecordHeader_TEXT;}

TrapRecord::TrapRecord() {}

void TrapRecord::csvEncode(const TrapRecord & cdr, std::string & rec)
{
    rec = "";
    CSVFileEncoder::addDateTime(rec, cdr.submitTime);
    CSVFileEncoder::addString(rec, cdr.alarmId.c_str());
    CSVFileEncoder::addString(rec, cdr.alarmObjCategory.c_str());
    CSVFileEncoder::addUint8 (rec, cdr.severity);
    CSVFileEncoder::addString(rec, cdr.text.c_str(),true);
}


} //snmp
} //smsc
