#ifndef _SCAG_PVSS_FLOODER_FLOODERCONFIG_H
#define _SCAG_PVSS_FLOODER_FLOODERCONFIG_H

#include <string>
#include <vector>

#include "scag/pvss/common/ScopeType.h"

namespace scag2 {
namespace pvss {
namespace flooder {

class FlooderConfig
{
public:
    FlooderConfig() :
    asyncMode_(false), speed_(10), addressesCount_(100), addressOffset_(0), flooderThreads_(1), commands_("s0g0"),
    oneCommandPerAbonent_(false), maxSpeed_(false), scopeType_(SCOPE_ABONENT) {
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
    int getAddressOffset() const { return addressOffset_; }
    void setAddressOffset( int off ) { addressOffset_ = off; }
    int getFlooderThreadCount() const { return flooderThreads_; }
    void setFlooderThreadCount( int ftc ) { flooderThreads_ = ftc; }
    bool getOneCommandPerAbonent() const { return oneCommandPerAbonent_; }
    void setOneCommandPerAbonent( bool v ) { oneCommandPerAbonent_ = v; }

    const std::vector< std::string >& getPropertyPatterns() const { return patterns_; }
    void setPropertyPatterns( const std::vector< std::string >& pat ) { patterns_ = pat; }

    const std::string& getCommands() const { return commands_; }
    void setCommands( const std::string& cmds ) { commands_ = cmds; }

    const std::string& getAddressFormat() const { return addressFormat_; }
    void setAddressFormat( const std::string& af ) { addressFormat_ = af; }

    ScopeType getScopeType() const { return scopeType_; }
    void setScopeType( ScopeType st ) { scopeType_ = st; }

    int getRequested() const { return 0; /* unlimited */ }

    bool getMaxSpeed() const { return maxSpeed_;  }
    void setMaxSpeed(bool maxSpeed) { maxSpeed_ = maxSpeed; }

private:
    bool    asyncMode_;
    int     speed_;
    int     getSetCount_;
    int     addressesCount_;
    int     addressOffset_;
    int     flooderThreads_;
    std::vector< std::string > patterns_;
    std::string                commands_;
    std::string                addressFormat_;
    bool    oneCommandPerAbonent_;
    bool    maxSpeed_;
    ScopeType scopeType_;
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_FLOODERCONFIG_H */
