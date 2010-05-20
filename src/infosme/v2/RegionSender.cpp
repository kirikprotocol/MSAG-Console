#include "RegionSender.h"
#include "SmscConnector.h"
#include "TaskTypes.hpp"

namespace smsc {
namespace infosme {

RegionSender::RegionSender( int regionId,
                            const smsc::util::config::region::Region& region,
                            SmscConnector& conn ) :
log_(smsc::logger::Logger::getInstance("is2.rsend")),
region_(region),
conn_(conn),
speedControl_(unsigned(region.getBandwidth())),
sent_(0),
id_(regionId)
{
    // smsc_log_debug(log_,"ctor %d/'%s'",id_,region_.getName().c_str());
}


RegionSender::~RegionSender()
{
    // smsc_log_debug(log_,"dtor %d",id_);
}


int RegionSender::send( unsigned curTime, Task& task, Message& msg )
{
    int nchunks = conn_.send(task,msg);
    if ( nchunks > 0 ) {
        speedControl_.consumeQuant(nchunks);
        ++sent_;
        return nchunks;
    }
    speedControl_.suspend( curTime + 5000 );
    return 0;
}

}
}
