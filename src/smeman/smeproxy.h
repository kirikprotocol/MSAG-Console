/*
  $Id$
*/

#include <exception>
#include "util/debug.h"
#include "smsccmd.h"
#include "core/synchronization/Event.hpp"

#if !defined __Cpp_Header__smeman_smeproxy_h__
#define __Cpp_Header__smeman_smeproxy_h__

namespace smsc {
namespace smeman {

enum SmeProxyState
{
  VALID,
  INVALID
};

typedef int SmeProxyPriority;
const int SmeProxyPriorityMin = 0;
const int SmeProxyPriorityMax = 32000;
typedef smsc::core::synchronization::Event ProxyMonitor;

class ProxyQueueLimitException : public std::exception
{
  virtual const char* what() const throw() {return "";}
};

// abstract
class SmeProxy
{
public: 
  //....
  virtual void close() = 0;
  /// ������ ProxyQueueLimitException ���� ��� ��������� ����� 
  /// � ����� ������� ��� �� ����� �� ����������� ��������
  virtual void putCommand(const SmscCommand& command) = 0;
  virtual SmscCommand getCommand() = 0;
  virtual SmeProxyState getState() const = 0;
  virtual void init() = 0;
  virtual SmeProxyPriority getPriority() const = 0;
  virtual bool hasInput() const = 0;
  virtual void attachMonitor(ProxyMonitor* monitor) = 0; // for detach monitor call with NULL
  virtual bool attached() = 0; // check what proxy has attached monitor (proxy is attached to dispatcher)
  virtual uint32_t getNextSequenceNumber() = 0;
};

}; // namespace smeman
}; // namespace smsc

#endif


