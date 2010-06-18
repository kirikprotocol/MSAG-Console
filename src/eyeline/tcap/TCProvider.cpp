#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/TCProvider.hpp"
#include "eyeline/tcap/provd/TCServiceAllocator.hpp"

namespace eyeline {
namespace tcap {

using smsc::core::synchronization::MutexGuard;

/* ************************************************************************* *
 * class TCProvider implementation
 * ************************************************************************* */
smsc::core::synchronization::Mutex  TCProvider::_TCPrvdSync;

TCProviderIface & TCProvider::initService(void) /*throw(std::exception)*/
{
  if (_tcSrv.get())
    throw smsc::util::Exception("TCProvider::Init(): TCProvider duplicate initialization");
  _tcSrv.reset(provd::allocTCService());
  return *_tcSrv.get();
}

TCProviderIface & TCProvider::getService(void) /*throw(std::exception)*/
{
  if (!_tcSrv.get())
    throw smsc::util::Exception("TCProvider::get(): TCProvider wasn't initialized");
  return *_tcSrv.get();
}

TCProvider &  TCProvider::getInstance(void) /*throw(std::exception)*/
{
  static TCProvider _instance;
  return _instance;
}

// ------------------------------------------------------
// -- Static methods
// ------------------------------------------------------
TCProviderIface & TCProvider::Init(void) /*throw(std::exception)*/
{
  MutexGuard  grd(_TCPrvdSync);
  return getInstance().initService();
}

TCProviderIface & TCProvider::get(void) /*throw(std::exception)*/
{
  MutexGuard  grd(_TCPrvdSync);
  return getInstance().getService();
}

void TCProvider::Shutdown(void)
{
  MutexGuard  grd(_TCPrvdSync);
  return getInstance().rlsService();
}

} //tcap
} //eyeline

