//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#if !defined PROXY_SME_SMACHINE_HEADER
#define PROXY_SME_SMACHINE_HEADER

#include "proxysme.h"
#include "ProxySmeQueue.h"
#include "ProxySmeMixer.h"
#include <stdexcept>
#include <memory>
#include "../smpp/smpp.h"
#include "../core/synchronization/Mutex.hpp"
#include "../core/threads/Thread.hpp"

SMSC_SMEPROXY_BEGIN

using namespace smsc::smpp;
using namespace smsc::core::synchronization;

struct SMachineNotifier {
  virtual bool SMachineBreak_() = 0;
};

enum { END_PROCESSING, BREAK_PROCESSING };

/// стет машина для обработки пакетов
class SMachine //: public smsc::core::threads::Thread
{
  enum THSTATE { IS_STOPPED, IS_RUNNING, IS_STOPPING };
  const ProxyConfig&  config_;
  Queue&              que_;
  Mixer&              mixer_;
  THSTATE             stopIt_;
public:
  void Stop();
//  void Start();
  SMachine(Queue&,Mixer&,const ProxyConfig&);
  virtual ~SMachine();
//  virtual int Execute();
  unsigned ProcessCommands(SMachineNotifier& notifier);
private:
  void TranslateToSubmitAndSend(DIRECTION direct,SmppHeader*);
  void TranslateToDeliverRespAndSend(DIRECTION direct,SmppHeader*);
};


SMSC_SMEPROXY_END

#endif // PROXY_SME_SMACHINE_HEADER
