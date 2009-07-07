#include "SMSCLinkSet.hpp"
#include "util/Exception.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "core/synchronization/RWLockGuard.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace balancers {

io_subsystem::LinkId
SMSCLinkSet::send(const io_subsystem::Message& message)
{
  smsc::core::synchronization::MutexGuard synchronize(_sendLock);
  smsc::core::synchronization::ReadLockGuard disableLinksetModification(_lock);
  io_subsystem::LinkId linkIdForUse;

  do {
    linkIdForUse = _arrayOfLinks[_linkNumInSet++];

    if ( linkIdForUse != EMPTY_LINK_ID ) {
      try {
        _links[linkIdForUse]->send(message);
        if ( _linkNumInSet == _actualNumOfLinks )
          _linkNumInSet = 0;
        return linkIdForUse;
      } catch (utilx::ProtocolException& ex) {
        smsc_log_error(_logger, "SMSCLinkSet::send::: caught ProtocolException='%s'",
                       ex.what());
      }
    }
  } while( _linkNumInSet < _actualNumOfLinks );
  _linkNumInSet = 0;
  throw io_subsystem::LinkSetSendException("SMSCLinkSet::send::: can't send message - there isn't active link");
}

}}}
