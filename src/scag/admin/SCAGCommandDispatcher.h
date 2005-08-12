//
// File:   SCAGCommandDispatcher.h
// Author: igork
//
// Created on 26 ��� 2004 �., 19:37
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


#include "scag/smsc.hpp"
#include "scag/admin/SCAGCommandReader.h"
#include "scag/admin/CommandApply.h"
#include "CommandUpdateSmeInfo.h"
#include "CommandAddSme.h"
#include "CommandStatusSme.h"
#include "CommandDeleteSme.h"
#include "CommandTraceRoute.h"
#include "CommandLoadRoutes.h"

namespace scag {
namespace admin {

using namespace smsc::admin::protocol;
using namespace smsc::admin::util;
using namespace smsc::scag;
using smsc::admin::AdminException;
using smsc::core::network::Socket;
using namespace scag::admin::Actions;

class SCAGCommandDispatcher : public CommandDispatcherTempl<SCAGCommandReader, ResponseWriter>
{
public:
  SCAGCommandDispatcher(Socket * admSocket);
  virtual ~SCAGCommandDispatcher();
  virtual Response *handle(const Command * const command) throw (AdminException);
  virtual void DoActions(Actions::CommandActions actions);

  static void shutdown();
  static void setGwConfigs(const SmscConfigs& cfg) { configs = &cfg; }
  static void startGw();
  static void stopGw();
  static void abortGw();
  static void dumpGw();

private:
  static const SmscConfigs* configs;
};

}
}

#endif  /* _SCAG_ADMIN_SCAGCommandDispatcher_H */
