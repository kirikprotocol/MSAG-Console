#ifndef _SCAG_PVSS_FLOODER_FLOODERCONFIG_H
#define _SCAG_PVSS_FLOODER_FLOODERCONFIG_H

#include <string>
#include <vector>

namespace scag2 {
namespace pvss {
namespace flooder {

class FlooderConfig
{
public:
    FlooderConfig() :
    asyncMode_(false), speed_(10), addressesCount_(100), flooderThreads_(1), commands_("s0g0") {
        patterns_.push_back("\"test0\" INT: 1 TIME_POLICY: FIXED FINAL_DATE: 2009/03/16 09:52:55 LIFE_TIME: 123");
    }

    bool getAsyncMode() const { return asyncMode_; }
    void setAsyncMode( bool async ) { asyncMode_ = async; }
    int getSpeed() const { return speed_; } // in calls/sec
    void setSpeed( int speed ) { speed_ = speed; }
    // int getGetSetCount() const { return getSetCount_;}
    // void setGetSetCount( int gsCount ) { getSetCount_ = gsCount; }
    int getAddressesCount() const { return addressesCount_; }
    void setAddressesCount( int ac ) { addressesCount_ = ac; } 
    int getFlooderThreadCount() const { return flooderThreads_; }
    void setFlooderThreadCount( int ftc ) { flooderThreads_ = ftc; }

    const std::string& getProfileKeyFormat() const { static const std::string s(".1.1.791%08d"); return s; }

    const std::vector< std::string >& getPropertyPatterns() const { return patterns_; }
    void setPropertyPatterns( const std::vector< std::string >& pat ) { patterns_ = pat; }

    const std::string& getCommands() const { return commands_; }
    void setCommands( const std::string& cmds ) { commands_ = cmds; }

    int getRequested() const { return 0; /* unlimited */ }

private:
    bool    asyncMode_;
    int     speed_;
    int     getSetCount_;
    int     addressesCount_;
    int     flooderThreads_;
    std::vector< std::string > patterns_;
    std::string                commands_;
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_FLOODERCONFIG_H */
