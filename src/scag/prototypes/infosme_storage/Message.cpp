#include <cerrno>
#include "Message.h"
#include "scag/util/io/EndianConverter.h"
#include "core/synchronization/Mutex.hpp"
#include "util/Exception.hpp"

using scag2::util::io::EndianConverter;

namespace {
smsc::core::synchronization::Mutex timeMutex;
}

namespace scag2 {
namespace prototypes {
namespace infosme {

namespace MsgState {
const char* toString( uint8_t state )
{
    switch (state) {
    case input : return "inpt";
    case process : return "proc";
    case sent : return "sent";
    case delivered : return "dlvr";
    case expired : return "expd";
    case failed : return "fail";
    case retry : return "rtry";
    default : return "????";
    }
}
}

time_t gmt20100101_ = 0;

void initgmt20100101()
{
    if (gmt20100101_ == 0) {
        smsc::core::synchronization::MutexGuard mg(timeMutex);
        if (gmt20100101_==0) {
            struct tm t0 = {0};
            t0.tm_mday = 1;
            t0.tm_year = 110;  // 2010
            const time_t loc = mktime(&t0);
            const time_t gmt = mktime(gmtime_r(&loc,&t0));
            if (loc == time_t(-1) || gmt == time_t(-1)) {
                throw smsc::util::Exception("gmt20100101: cannot mktime",errno);
            }
            gmt20100101_ = loc - gmt + loc;
        }
    }
}


char* formatMsgTime( char* buf, msgtime_type theTime )
{
    const time_t t = gmt20100101_ + theTime;
    struct tm tx = {0};
    if ( !gmtime_r(&t,&tx) ) {
        throw smsc::util::Exception("formatMsgTime: cannot gmtime_r");
    }
    sprintf(buf,"%02u-%02u-%02u+%02u:%02u:%02u",
            tx.tm_year%100, tx.tm_mon+1, tx.tm_mday, tx.tm_hour, tx.tm_min, tx.tm_sec );
    return buf;
}

unsigned char* Message::toBuf( uint16_t version, unsigned char* buf )
{
    EndianConverter::set64(buf,subscriber);
    buf += 8;
    EndianConverter::set32(buf,uint32_t(msgId));
    buf += 4;
    EndianConverter::set32(buf,uint32_t(lastTime));
    buf += 4;
    EndianConverter::set32(buf,uint32_t(timeLeft));
    buf += 4;
    EndianConverter::set32(buf,uint32_t(textId));
    buf += 4;
    memcpy(buf,userData.c_str(),USERDATA_LENGTH);
    buf += USERDATA_LENGTH;
    *buf = state;
    ++buf;
    return buf;
}


char* Message::printToBuf( uint16_t version, char* buf )
{
    char lbuf[20];
    char tbuf[20];
    return buf + sprintf(buf,
                         "%llu,%u,%s,%s,%u,%s,%s",subscriber,msgId,
                         formatMsgTime(lbuf,lastTime),
                         formatMsgTime(tbuf,timeLeft),
                         textId,userData.c_str(),
                         MsgState::toString(state));
}

}
}
}
