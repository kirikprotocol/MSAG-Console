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

enum SmeBindMode{
  smeRX,smeTX,smeTRX
};


typedef int SmeProxyPriority;
const int SmeProxyPriorityMinBr = 0;
const int SmeProxyPriorityMin = 1;
const int SmeProxyPriorityDefault = 16000;
const int SmeProxyPriorityMax = 32000;
const int SmeProxyPriorityMaxBr = SmeProxyPriorityMax+1;
typedef smsc::core::synchronization::Event ProxyMonitor;

class ProxyQueueLimitException : public std::exception
{
  virtual const char* what() const throw() {return "Proxy Queue limit";}
};

class InvalidProxyCommandException: public std::exception
{
  virtual const char* what() const throw() {return "Invalid bind state";}
};

static std::string nullstr="";

// abstract
class SmeProxy
{
public:
  //....
  virtual ~SmeProxy() {}
  virtual void close() = 0;
  /// ������ exception ���� ��� ��������� �����
  /// � ����� ������� ��� �� ����� �� ����������� ��������
  virtual void putCommand(const SmscCommand& command) = 0;
  virtual SmscCommand getCommand() = 0;
  virtual SmeProxyState getState() const = 0;
  virtual void init() = 0;
  virtual SmeProxyPriority getPriority() {return SmeProxyPriorityDefault;};
  virtual void setPriority(SmeProxyPriority){};
  virtual bool hasInput() const = 0;
  virtual void attachMonitor(ProxyMonitor* monitor) = 0; // for detach monitor call with NULL
  virtual bool attached() = 0; // check what proxy has attached monitor (proxy is attached to dispatcher)
  virtual uint32_t getNextSequenceNumber() = 0;
  virtual uint32_t getUniqueId() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
  virtual uint32_t getIndex() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
  virtual unsigned long getPreferredTimeout() { return 8; }
  virtual const std::string& getSourceAddressRange(){return nullstr;};
  virtual const char * getSystemId() const = 0;

  virtual int getBindMode(){return smeTRX;}

  // in and out MUST BE AT LEAST 32 BYTES!!!
  // return false if unfilled
  // port of peer is optional
  virtual bool getPeers(char* in,char* out)
  {
    strcpy(in,"127.0.0.1");
    strcpy(out,"127.0.0.1");
    return true;
  };

  virtual void disconnect(){};
};

}; // namespace smeman
}; // namespace smsc

#endif
