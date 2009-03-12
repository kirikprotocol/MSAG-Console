#ifndef _SCAG_PVSS_FLOODER_FLOODERCONFIG_H
#define _SCAG_PVSS_FLOODER_FLOODERCONFIG_H

#include <string>

namespace scag2 {
namespace pvss {
namespace flooder {

class FlooderConfig
{
public:
    FlooderConfig() :
    asyncMode_(false), speed_(10), getSetCount_(100), addressesCount_(100), flooderThreads_(1) {}

    bool getAsyncMode() const { return asyncMode_; }
    void setAsyncMode( bool async ) { asyncMode_ = async; }
    int getSpeed() const { return speed_; } // in calls/sec
    void setSpeed( int speed ) { speed_ = speed; }
    int getGetSetCount() const { return getSetCount_;}
    void setGetSetCount( int gsCount ) { getSetCount_ = gsCount; }
    int getAddressesCount() const { return addressesCount_; }
    void setAddressesCount( int ac ) { addressesCount_ = ac; } 
    int getFlooderThreadCount() const { return flooderThreads_; }
    void setFlooderThreadCount( int ftc ) { flooderThreads_ = ftc; }
    
private:
    bool    asyncMode_;
    int     speed_;
    int     getSetCount_;
    int     addressesCount_;
    int     flooderThreads_;
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_FLOODERCONFIG_H */
