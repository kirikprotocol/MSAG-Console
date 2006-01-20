//
// File:   SCAGCommandDispatcher.h
// Author: igork
//
// Created on 26 Май 2004 г., 19:37
//

#ifndef _SCAG_ADMIN_SCAGCommandDispatcher_H
#define _SCAG_ADMIN_SCAGCommandDispatcher_H

#include "CommandActions.h"
#include "admin/AdminException.h"
#include "admin/util/CommandDispatcher.h"
#include "admin/protocol/ResponseWriter.h"
#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include "core/buffers/Hash.hpp"

// Its during for a test only
//#include "scag/scag.h"

#include "scag/admin/SCAGCommandReader.h"
#include "scag/admin/SCAGCommand.h"
#include "logger/Logger.h"

namespace scag {
namespace admin {

using namespace smsc::admin::protocol;
using namespace smsc::admin::util;
using namespace scag;
using smsc::admin::AdminException;
using smsc::core::network::Socket;
using namespace scag::admin::Actions;
using smsc::logger::Logger;

class SCAGCommandDispatcher : public CommandDispatcherTempl<SCAGCommandReader, ResponseWriter>
{
public:
  static Logger * getLogger();
  SCAGCommandDispatcher(Socket * admSocket);
  virtual ~SCAGCommandDispatcher();
  virtual Response *handle(const Command * const command) throw (AdminException);
  virtual void DoActions(Actions::CommandActions actions);

  static void shutdown();
  static void startGw();
  static void stopGw();
  static void abortGw();
  static void dumpGw();
};

}
}

#endif  /* _SCAG_ADMIN_SCAGCommandDispatcher_H */
