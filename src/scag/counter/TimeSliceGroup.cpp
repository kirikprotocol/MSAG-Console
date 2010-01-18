#include "TimeSliceGroup.h"

namespace scag2 {
namespace counter {

smsc::logger::Logger* TimeSliceGroup::log_ = 0;

void TimeSliceItem::setTimeSliceGroup( TimeSliceGroup* grp )
{
    TimeSliceGroup* oldgrp = getTimeSliceGroup();
    if ( oldgrp ) oldgrp->remItem(*this);
    doSetTimeSliceGroup(grp);
    if ( grp ) grp->addItem(*this);
}
    

TimeSliceGroup* TimeSliceManager::createGroup( usec_type slice )
{
    return new TimeSliceGroup( slice );
}

}
}
