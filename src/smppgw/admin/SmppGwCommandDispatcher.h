// 
// File:   SmppGwCommandDispatcher.h
// Author: igork
//
// Created on 26 Май 2004 г., 19:37
//

#ifndef _SMPPGW_ADMIN_SmppGwCommandDispatcher_H
#define	_SMPPGW_ADMIN_SmppGwCommandDispatcher_H

#include "admin/AdminException.h"
#include "admin/util/CommandDispatcher.h"
#include "admin/protocol/ResponseWriter.h"
#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"

#include "smppgw/smsc.hpp"
#include "smppgw/admin/SmppGwCommandReader.h"
#include "smppgw/admin/CommandApply.h"
#include "CommandUpdateSmeInfo.h"
#include "CommandAddSme.h"
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

class SmppGwCommandDispatcher : public CommandDispatcherTempl<SmppGwCommandReader, ResponseWriter>
{
public:
	SmppGwCommandDispatcher(Socket * admSocket);
	virtual ~SmppGwCommandDispatcher();
	virtual Response *handle(const Command * const command) throw (AdminException);
	static void shutdown();
  
  static void setGwConfigs(const SmscConfigs& cfg) { configs = &cfg; }
  
  static void startGw();
  static void stopGw();
  static void abortGw();
  static void dumpGw();

protected:
  Response * apply(CommandApply*);
  Response * applyConfig();
  Response * applyRoutes();
  Response * applyProviders();
  Response * applySmscs();
  Response * traceRoute(CommandTraceRoute*);
  Response * loadRoutes(CommandLoadRoutes*);

  Response * updateSmeInfo(CommandUpdateSmeInfo*);
  Response * addSme(CommandAddSme*);
  Response * deleteSme(CommandDeleteSme*);
  
private:
  static const SmscConfigs* configs;
};

}
}
}

#endif	/* _SMPPGW_ADMIN_SmppGwCommandDispatcher_H */

