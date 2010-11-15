/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef _INMAN_TEST_FACADE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _INMAN_TEST_FACADE_HPP

#include "core/synchronization/Event.hpp"

#include "inman/interaction/ConnSrv.hpp"
#include "inman/interaction/connect.hpp"
#include "inman/interaction/messages.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::logger::Logger;

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using smsc::inman::interaction::ConnectSrv;
using smsc::inman::interaction::ConnectSupervisorITF;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectAC;
using smsc::inman::interaction::ConnectListenerITF;

using smsc::inman::interaction::SerializablePacketAC;
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPPacketAC;


/* ************************************************************************** *
 * class TSTFacadeAC: 
 * ************************************************************************** */
#define MAX_FACADE_NAME 50
class TSTFacadeAC : public ConnectSupervisorITF, public ConnectListenerITF {
protected:
  mutable Mutex   _sync;
  Connect *       _pipe;
  ConnectSrv *    _connSrv;
  Logger *        logger;
    //logging prefix, f.ex: "TSTFacadeAC[%u]"
  char            _logId[MAX_FACADE_NAME + sizeof("[%u]") + sizeof(unsigned)*3 + 1];

  void do_disconnect(void);

  bool is_active(void) const
  {
    return (bool)(_pipe && (_pipe->getId() != (unsigned)(INVALID_SOCKET)));
  }

public:
  explicit TSTFacadeAC(ConnectSrv * conn_srv, Logger * use_log = NULL)
    : _pipe(NULL), _connSrv(conn_srv)
  {
    logger = use_log ? use_log : Logger::getInstance("smsc.InTST");
  }
  virtual ~TSTFacadeAC()
  {
    Disconnect();
  }

  void Prompt(Logger::LogLevel dlvl, const char * zstr);
  void Prompt(Logger::LogLevel dlvl, const std::string & str)
  {
    Prompt(dlvl,str.c_str());
  }

  void Disconnect(void);

  bool isActive(void) const;

  unsigned initConnect(const char* use_host, int use_port);

  //Customized variant of Connect::sendPck(): it sends specified
  //number of bytes from ObjectBuffer
  int  sendPckPart(INPPacketAC * pck, uint32_t num_bytes = 0);

  // ---------------------------------------------------
  // -- ConnectSupervisorITF interface implementation
  // ---------------------------------------------------
  virtual bool onConnectClosed(ConnectAC * conn);

  // ---------------------------------------------------
  // -- ConnectListenerITF interface implementation
  // ---------------------------------------------------
  //virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd) = 0;
  virtual void onConnectError(Connect * conn,
                              std::auto_ptr<CustomException> & p_exc);
};


} // namespace test
} // namespace inman
} // namespace smsc
#endif /* _INMAN_TEST_FACADE_HPP */

