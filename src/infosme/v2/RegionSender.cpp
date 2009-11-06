#include "RegionSender.h"
#include "SmscConnector.h"
#include "Types.h"

namespace smsc {
namespace infosme2 {

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


uint8_t RegionSender::send( unsigned curTime, const Task& task, const Message& msg )
{
    if ( conn_.send(task,msg) ) {
        speedControl_.consumeQuant();
        ++sent_;
        return MessageState::ENROUTE;
    } else {
        speedControl_.suspend( curTime + 1000 );
        return MessageState::RETRY;
    }
}

}
}
