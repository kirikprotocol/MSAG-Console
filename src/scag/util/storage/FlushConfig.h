#ifndef _SCAG_UTIL_STORAGE_FLUSHCONFIG_H
#define _SCAG_UTIL_STORAGE_FLUSHCONFIG_H

namespace scag2 {
namespace util {
namespace storage {

struct FlushConfig 
{
    FlushConfig() :
    flushSpeed(1000),
    flushCount(10),
    flushLowMark(20),
    flushHighMark(200),
    flushInterval(10000) {}

    void postLoadFix() {
        if ( flushCount <= 0 ) flushCount = 1;
        if ( flushCount > flushLowMark ) flushLowMark = flushCount;
        if ( flushLowMark > flushHighMark ) flushHighMark = flushLowMark;
        if ( flushInterval > 100000 ) flushInterval = 100000;
        if ( flushInterval < 100 ) flushInterval = 100;
        if ( flushSpeed > 100000 ) flushSpeed = 100000;
    }

    std::string toString() const {
        char buf[200];
        sprintf(buf,"flushSpeed=%ukb/s flushCount=%u flushLowMark=%u flushHighMark=%u flushInterval=%ums",
                flushSpeed, flushCount, flushLowMark, flushHighMark, flushInterval );
        return buf;
    }

public:
    unsigned flushSpeed;
    unsigned flushCount;
    unsigned flushLowMark;
    unsigned flushHighMark;
    unsigned flushInterval;
};

}
}
}

#endif
