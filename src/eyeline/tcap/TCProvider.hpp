/* ************************************************************************** *
 * Transaction Capabilities Provider.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TC_PROVIDER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TC_PROVIDER_HPP

#include "core/synchronization/Mutex.hpp"
#include "eyeline/tcap/TCProviderIface.hpp"

namespace eyeline {
namespace tcap {

class TCProvider {
private:
  static smsc::core::synchronization::Mutex _TCPrvdSync;
  static std::auto_ptr<TCProviderAC>        _tcSrv;

protected:
  static TCProvider & getInstance(void) /*throw(std::exception)*/;

  //Allocates TCProvider service, returns its interface
  TCProviderIface & initService(void) /*throw(std::exception)*/;
  //Returns interface of TCProvider service
  TCProviderIface & getService(void) /*throw(std::exception)*/;
  //Releases TCProvider service
  void rlsService(void) { _tcSrv.reset(0); }

  //forbid destruction and instantiation
  TCProvider()
  { }
  ~TCProvider()
  { }
  void * operator new(size_t);
  TCProvider & operator=(const TCProvider &);

public:
  static TCProviderIface &  Init(void) /*throw(std::exception)*/;
  static TCProviderIface &  get(void) /*throw(std::exception)*/;
  static void               Shutdown(void);
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TC_PROVIDER_HPP */

