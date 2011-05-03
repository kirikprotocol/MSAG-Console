/* ************************************************************************** *
 * USSMan service connect: handles USS requests coming from assigned TCP
 * connect in asynchronous mode, manages corresponding USSD dialogs using
 * TCAP layer.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_USS_EMULATOR_USSMANEMULCONNECT_HPP__
#ident "@(#)$Id$"
# define __SMSC_INMAN_USS_EMULATOR_USSMANEMULCONNECT_HPP__

# include <memory>
# include <set>

# include "inman/interaction/connect.hpp"
# include "inman/interaction/serializer.hpp"
# include "inman/uss/emulator/MessageHandler.hpp"
# include "util/Singleton.hpp"

namespace smsc {
namespace inman {
namespace uss {

using smsc::util::CustomException;

class USSManEmulConnect : public interaction::ConnectListenerITF {
public:
  explicit USSManEmulConnect(logger::Logger* logger)
    : _logger(logger), _requestProcessor(NULL) {}
  ~USSManEmulConnect() { delete _requestProcessor; }

  virtual void onPacketReceived(interaction::Connect* conn,
                                std::auto_ptr<interaction::SerializablePacketAC>& recv_cmd)
    throw(std::exception);
  virtual void onConnectError(interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc);

  void close(interaction::ConnectAC* conn);
private:
  logger::Logger *_logger;
  MessageHandler* _requestProcessor;
};

} //uss
} //inman
} //smsc

#endif /* __SMSC_INMAN_USSMAN_CONNECT_HPP__ */
