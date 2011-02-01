#ifndef _INFORMER_CORESMSCSTATS_H
#define _INFORMER_CORESMSCSTATS_H

#include <string>
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

/// a statistical data for an SMSC
struct CoreSmscStats
{
    std::string   smscId;   // smsc id
    timediff_type liveTime; // time since connection, or -1 (seconds)
    unsigned      nRegions; // number of regions attached
    unsigned      maxBandwidth; // a sum of bw of all regions (sms/seconds)
    msgtime_type  avgInterval;  // an interval of integration (seconds)
    unsigned      currentLoad;  // a number of sms counted during avgInterval
    unsigned      nResponses;   // a number of response wait timers
    unsigned      nReceipts;    // a number of receipt wait timers
};

}
}

#endif
