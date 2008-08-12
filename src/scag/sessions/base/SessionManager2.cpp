#include <cassert>
#include "SessionManager2.h"

namespace {

scag2::sessions::SessionManager* inst_ = 0;
smsc::core::synchronization::Mutex mtx;

}

namespace scag2 {
namespace sessions {

SessionManager& SessionManager::Instance()
{
    MutexGuard mg(mtx);
    assert(inst_);
    return *inst_;
}

SessionManager::SessionManager()
{
    MutexGuard mg(mtx);
    assert( ! inst_ );
    inst_ = this;
}

SessionManager::~SessionManager()
{
    MutexGuard mg(mtx);
    assert( inst_ == this );
    inst_ = 0;
}

} // namespace sessions
} // namespace scag2
