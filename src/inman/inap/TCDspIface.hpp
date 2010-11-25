/* ************************************************************************* *
 * TCAP Dispatcher: interface definitions
 * ************************************************************************* */
#ifndef __INMAN_TCAP_DISPATCHER_IFACE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_DISPATCHER_IFACE_HPP

#include "logger/Logger.h"
#include "inman/inap/ACRegDefs.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

using smsc::logger::Logger;

class SSNSession;

class TCAPDispatcherITF {
protected:
  virtual ~TCAPDispatcherITF() //forbid interface destruction
  { }

public:
  enum DSPState_e {
    dspStopped = 0, dspStopping, dspRunning
  };
  enum SS7State_e { ss7None = 0
      , ss7INITED     = 1 //CP static data is initialized
      , ss7REGISTERED = 2 //remote CP manager and message port owner are registered,
                          //SS7 communication facility is initialized
      , ss7OPENED     = 3 //user input message queue is opened
      , ss7CONNECTED  = 4 //user is connected to at least one TCAP BE unit
  };

  //Returns dispatcher state
  virtual DSPState_e  dspState(void) const = 0;
  //Returns state of TCAP BE unit(s) connection
  virtual SS7State_e  ss7State(void) const = 0;
  //Binds SSN and initializes SSNSession (TCAP dialogs registry/factory)
  virtual SSNSession *
      openSSN(uint8_t ssn_id, uint16_t max_dlg_id = 2000,
                                      Logger * uselog = NULL) = 0;
  //
  virtual SSNSession * findSession(uint8_t ssn) const = 0;
  //
  virtual ApplicationContextRegistryITF * acRegistry(void) const = 0;
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_DISPATCHER_IFACE_HPP */

