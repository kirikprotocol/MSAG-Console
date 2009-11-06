#ifndef _SMSC_INFOSME2_REGIONSENDER_H
#define _SMSC_INFOSME2_REGIONSENDER_H

#include "util/int.h"
#include "logger/Logger.h"
#include "util/config/region/Region.hpp"
#include "SpeedControl.h"

namespace smsc {
namespace infosme2 {

class SmscConnector;
class Task;
class Message;

class RegionSender 
{
public:
    RegionSender( int regionId,
                  const smsc::util::config::region::Region& region,
                  SmscConnector& conn );
    ~RegionSender();
    
    inline int getId() const { return id_; }
    unsigned getBandwidth() const { return speedControl_.getSpeed(); }
    
    inline unsigned isReady( unsigned curTime ) {
        return speedControl_.isReady(curTime);
    }
    inline void suspend( unsigned nextTime ) {
        return speedControl_.suspend(nextTime);
    }

    uint8_t send( unsigned curTime, const Task& task, const Message& msg );

private:
    RegionSender( const RegionSender& );
    RegionSender& operator = ( const RegionSender& );

private:
    smsc::logger::Logger*                     log_;
    const smsc::util::config::region::Region& region_;
    SmscConnector&                            conn_;
    SpeedControl                              speedControl_;
    uint64_t                                  sent_;
    int                                       id_;
};

}
}

#endif
