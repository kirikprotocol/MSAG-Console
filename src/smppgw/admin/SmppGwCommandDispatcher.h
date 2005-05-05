//
// File:   SmppGwCommandDispatcher.h
// Author: igork
//
// Created on 26 ��� 2004 �., 19:37
//

#ifndef _SMPPGW_ADMIN_SmppGwCommandDispatcher_H
#define _SMPPGW_ADMIN_SmppGwCommandDispatcher_H

#include "CommandActions.h"
#include "admin/AdminException.h"
#include "admin/util/CommandDispatcher.h"
#include "admin/protocol/ResponseWriter.h"
#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include "core/buffers/Hash.hpp"


#include "smppgw/smsc.hpp"
#include "smppgw/admin/SmppGwCommandReader.h"
#include "smppgw/admin/CommandApply.h"
#include "CommandUpdateSmeInfo.h"
#include "CommandAddSme.h"
#include "CommandStatusSme.h"
#include "CommandDeleteSme.h"
#include "CommandTraceRoute.h"
#include "CommandLoadRoutes.h"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::admin::protocol;
using namespace smsc::admin::util;
using namespace smsc::smppgw;
using smsc::admin::AdminException;
using smsc::core::network::Socket;
using namespace smsc::smppgw::admin::Actions;

class SmppGwCommandDispatcher : public CommandDispatcherTempl<SmppGwCommandReader, ResponseWriter>
{
public:
  SmppGwCommandDispatcher(Socket * admSocket);
  virtual ~SmppGwCommandDispatcher();
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
}

#endif  /* _SMPPGW_ADMIN_SmppGwCommandDispatcher_H */
