#ifndef _SMSC_INFOSME2_REGIONSENDER_H
#define _SMSC_INFOSME2_REGIONSENDER_H

#include "util/int.h"
#include "logger/Logger.h"
#include "util/config/region/Region.hpp"
#include "SpeedControl.h"

namespace smsc {
namespace infosme {

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

    /// send a message
    /// @return number of chunks has been sent, or 0
    int send( unsigned curTime, Task& task, Message& msg );
    SmscConnector& getSmscConnector() { return conn_; }

private:
    RegionSender( const RegionSender& );
    RegionSender& operator = ( const RegionSender& );

private:
    smsc::logger::Logger*                     log_;
    const smsc::util::config::region::Region& region_;
    SmscConnector&                            conn_;
    SpeedControl<>                            speedControl_;
    uint64_t                                  sent_;
    int                                       id_;
};

}
}

#endif
