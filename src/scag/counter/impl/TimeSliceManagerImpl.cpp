#include <map>
#include "TimeSliceManagerImpl.h"
#include "NotificationManager.h"

namespace scag2 {
namespace counter {
namespace impl {

using smsc::core::synchronization::MutexGuard;

TimeSliceManagerImpl::TimeSliceManagerImpl( smsc::logger::Logger* logger,
                                            TimeSliceItem*        observer,
                                            unsigned              slices ) :
log_(logger),
observer_(observer),
lastTime_(TSource::getUSec()),
observeSlice_(sliceToTime(slices)),
stopping_(true)
{
    smsc_log_info(log_,"ctor");
}


TimeSliceManagerImpl::~TimeSliceManagerImpl()
{
    stop();
    // FIXME: destruction of groups in hash
    smsc_log_info(log_,"dtor");
}


void TimeSliceManagerImpl::start()
{
    if (!stopping_) return;
    MutexGuard mg(mon_);
    if (!stopping_) return;
    stopping_ = false;
    this->Start();
}


void TimeSliceManagerImpl::stop()
{
    if (stopping_) return;
    {
        MutexGuard mg(mon_);
        if (stopping_) return;
        stopping_ = true;
        mon_.notifyAll();
    }
    this->WaitFor();
}


int TimeSliceManagerImpl::Execute()
{
    smsc_log_info(log_,"started");

    usec_type wakeTime = lastTime_ + minUSlice;
    usec_type observeTime = lastTime_ + observeSlice_;
    typedef std::multimap< usec_type, TimeSliceGroup* > GroupMap;

    GroupMap groupMap;
    while ( ! stopping_ ) {

        usec_type curTime;
        {
            MutexGuard mg(mon_);
            if ( stopping_ ) break;
            curTime = TSource::getUSec();
            smsc_log_debug(log_,"new pass at %llu", curTime );

            // adding incoming groups
            if ( !groupVector_.empty() ) {
                smsc_log_debug(log_,"adding %u new groups to the processing",unsigned(groupVector_.size()));
                for ( GroupVector::iterator i = groupVector_.begin();
                      i != groupVector_.end(); ++i ) {
                    usec_type grpSlice = (*i)->getSlice();
                    if ( grpSlice == observeSlice_ ) {
                        // the group should be synchronized w/ observer
                        grpSlice = observeTime - lastTime_;
                    }
                    const usec_type grpTime = lastTime_ + grpSlice;
                    groupMap.insert(std::make_pair(grpTime,*i));
                    if ( grpTime < wakeTime ) wakeTime = grpTime;
                }
                groupVector_.clear();
            }

            if ( curTime < wakeTime ) {
                const int sleepTime = int((wakeTime-curTime)/1000);
                if ( sleepTime > 0 ) {
                    smsc_log_debug(log_,"sleeping %u msec",sleepTime);
                    mon_.wait(sleepTime);
                    if ( stopping_ ) break;
                    continue;
                }
            }

            lastTime_ = wakeTime;
            wakeTime += minUSlice;

        }

        // process all selected groups
        GroupMap::iterator ilast = groupMap.upper_bound( curTime );
        GroupMap newMap;
        for ( GroupMap::iterator i = groupMap.begin(); i != ilast; ++i ) {
            smsc_log_debug(log_,"advancing group %u",timeToSlice(i->second->getSlice()));
            const usec_type nextTime = i->second->advanceTime(curTime);
            newMap.insert(std::make_pair(nextTime,i->second));
        }
        groupMap.erase(groupMap.begin(),ilast);
        groupMap.insert(newMap.begin(),newMap.end());

        // send notification if needed
        if ( observer_ && curTime >= observeTime ) {
            observeTime += observeSlice_;
            observer_->advanceTime(curTime);
        }
    }
    smsc_log_info(log_,"stopped");
    return 0;
}


TimeSliceGroup* TimeSliceManagerImpl::addItem( TimeSliceItem& item, usec_type theSlice )
{
    const unsigned slices = roundSlice(theSlice);
    const usec_type slice = sliceToTime(slices);
    TimeSliceGroup* grp;
    {
        MutexGuard mg(mon_);
        TimeSliceGroup** ptr = groupHash_.GetPtr(slice);
        if ( ptr ) {
            grp = *ptr;
        } else {
            // no such group
            smsc_log_debug(log_,"group %u is created",slices);
            grp = createGroup(slice);
            groupHash_.Insert(slice,grp);
            groupVector_.push_back(grp);
            mon_.notify();
        }
    }
    grp->addItem( item );
    return grp;
}

}
}
}
