#include "HashCountManager.h"
#include "util/Exception.hpp"

namespace scag2 {
namespace counter {
namespace impl {

using namespace smsc::core::synchronization;

HashCountManager::~HashCountManager()
{
    {
        MutexGuard mg(disposeMon_);
        disposeQueue_.clear();
    }
    MutexGuard mg(hashMutex_);
    char* key;
    Counter* ptr;
    for ( smsc::core::buffers::Hash<Counter*>::Iterator iter(&hash_); iter.Next(key,ptr); ) {
        delete ptr;
    }
    hash_.Empty();
}


CounterPtrAny HashCountManager::getAnyCounter( const char* name )
{
    {
        MutexGuard mg(hashMutex_);
        Counter** ptr = hash_.GetPtr(name);
        if (ptr) return CounterPtrAny(*ptr);
    }
    return CounterPtrAny();
}

CounterPtrAny HashCountManager::registerAnyCounter( Counter* ccc )
{
    if (!ccc) throw smsc::util::Exception("CountManager: null counter to register");
    std::auto_ptr<Counter> cnt(ccc);
    MutexGuard mg(hashMutex_);
    Counter** ptr = hash_.GetPtr(ccc->getName().c_str());
    if ( ptr ) {
        if ( ccc->getType() != (*ptr)->getType() ) {
            throw smsc::util::Exception("CountManager: counter '%s' already registered with different type %d != %d",
                                        ccc->getName().c_str(), (*ptr)->getType(), ccc->getType());
        }
        return CounterPtrAny(*ptr);
    }
    hash_.Insert(ccc->getName().c_str(),cnt.release());
    return CounterPtrAny(ccc);
}


void HashCountManager::scheduleDisposal( counttime_type dt, Counter& c )
{
    MutexGuard mg(disposeMon_);
    if ( dt < nextTime_ ) nextTime_ = dt;
    disposeQueue_.push_back(&c);
}


counttime_type HashCountManager::getCurrentTime() const
{
    return counttime_type(time(NULL));
}

}
}
}
