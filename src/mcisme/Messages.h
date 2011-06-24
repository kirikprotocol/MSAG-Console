#ifndef SMSC_MCI_SME_MESSAGES
#define SMSC_MCI_SME_MESSAGES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <sys/types.h>

#include <string>

#include "core/buffers/TmpBuf.hpp"
#include "core/buffers/Array.hpp"
#include "core/buffers/Hash.hpp"

#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "Templates.h"
#include "misscall/callproc.hpp"
#include "mcisme/AbntAddr.hpp"
#include "logger/Logger.h"
#include "mcisme/Profiler.h"

namespace smsc {
namespace mcisme {
using namespace smsc::misscall;

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using smsc::logger::Logger;

struct Message
{
  AbntAddr called_abonent;
  std::string calling_abonent, message, smsc_id;
  bool        notification, skip, data_sm;
  static int  maxRowsPerMessage;

public:
  Message()
  : notification(false), skip(false), data_sm(false)
  {}

  const char* GetMsg(void) const
  {
    return message.c_str();
  }

  int GetMsgLen(void) const
  {
    return static_cast<int>(message.length());
  }

  std::string toString() const {
    char msgBuf[1024];
    snprintf(msgBuf, sizeof(msgBuf), "called_abonent=%s,calling_abonent=%s,message=%s,smsc_id=%s,notification=%d,skip=%d,data_sm=%d",
             called_abonent.getText().c_str(), calling_abonent.c_str(), message.c_str(), smsc_id.c_str(),
             notification, skip, data_sm);
    return msgBuf;
  }
};

struct MCEvent
{
  uint8_t       id;
  time_t        dt;
  AbntAddrValue caller;
  uint16_t      callCount;
  uint8_t       missCallFlags;

  MCEvent(const uint8_t& _id):id(_id), dt(0), callCount(0), missCallFlags(0) {}
  MCEvent():id(0), dt(0), callCount(0), missCallFlags(0)
  {
    memset((void*)&(caller.full_addr), 0xFF, sizeof(caller.full_addr));
  }

  MCEvent(const MCEvent& e): id(e.id), dt(e.dt), callCount(e.callCount), missCallFlags(e.missCallFlags)
  {
    memcpy((void*)&(caller.full_addr), (void*)&(e.caller.full_addr), sizeof(caller.full_addr));
  }
  MCEvent& operator=(const AbntAddr& addr)
  {
    id = 0; dt = 0; callCount = 1; missCallFlags = 0;
    memcpy((void*)&(caller.full_addr), (void*)addr.getAddrSig(), sizeof(caller.full_addr));
    return *this;
  }

  MCEvent& operator=(const MCEvent& e)
  {
    if(&e == this)
      return *this;

    id = e.id; dt = e.dt; callCount = e.callCount; missCallFlags = e.missCallFlags;
    memcpy((void*)&(caller.full_addr), (void*)&(e.caller.full_addr), sizeof(caller.full_addr));
    return *this;
  }

  const char* toString(char* textString,size_t size) const
  {
    snprintf(textString, size, "id=%d,dt=%d,caller=%s,callCount=%d,missCallFlags=0x%02X", id, dt, AbntAddr(&caller).getText().c_str(), callCount, missCallFlags);

    return textString;
  }
};

struct sms_info;

struct MCEventOut {
  std::vector<MCEvent> srcEvents;
  Message msg;
  AbonentProfile abonentA_profile;

  std::string toString() const {
    std::string result;
    result="msg=[";
    result+=msg.toString();
    result+="],srcEvents=[";

    char buf[128];
    for(std::vector<MCEvent>::const_iterator iter = srcEvents.begin(), end_iter = srcEvents.end(); iter != end_iter;)
    {
      result+=iter->toString(buf,sizeof(buf));
      if ( ++iter != end_iter )
      {
        result += ",";
      }
    }
    result += "]";

    return result;
  }
};

/* -------------------------- Параметры для шаблона сообщения -------------------------- */
extern const char* MSG_TEMPLATE_PARAM_ABONENT; // кому посылается сообщение (i|n)
extern const char* MSG_TEMPLATE_PARAM_TOTAL;   // общее количество звонков  (i)
extern const char* MSG_TEMPLATE_PARAM_ROWS;    // строки сообщения          (i)
    
/* -------------------------- Параметры для шаблона строки -------------------------- */
extern const char* MSG_TEMPLATE_PARAM_CALLER;  // номер звонившего абонента (i)
extern const char* MSG_TEMPLATE_PARAM_COUNT;   // количество звонков от абонента, if (single) => 1 (i)
extern const char* MSG_TEMPLATE_PARAM_DATE;    // дата звонка/дата последнего звонка (i)

class TaskProcessor;

class MessageFormatter
{
private:
  smsc::logger::Logger *logger;
  InformTemplateFormatter*    formatter;
  TaskProcessor& _taskProcessor;

  Hash <uint32_t>             counters;
  Array<MissedCallEvent>      events;

  bool isLastFromCaller(int index);
public:

  MessageFormatter(InformTemplateFormatter* _formatter,
                   TaskProcessor& task_processor)
  : logger(Logger::getInstance("mci.msgfmt")), formatter(_formatter), _taskProcessor(task_processor) {};

  bool canAdd(const MissedCallEvent& event);
  void addEvent(const MissedCallEvent& event);
  void addEvent(const AbntAddr& abnt, const MCEvent& event);

  bool formatMessage(const AbntAddr& abnt,
                     const vector<MCEvent>& mc_events,
                     MCEventOut* for_send,
                     const std::string& smscAddress,
                     int timeOffset,
                     bool originatingAddressIsSmscAddress);
};

void addBanner(std::string& message, const string& banner);

void keyIsNotSupported(const char* type) throw(AdapterException);
void typeIsNotSupported(const char* type) throw(AdapterException);
const char* findMessageTemplateKey(const char* key) throw(AdapterException);

class NotifyGetAdapter : public GetAdapter
{
protected:

  std::string abonent, caller;

public:

  NotifyGetAdapter(const std::string& _abonent="", const std::string& _caller="")
    : GetAdapter(), abonent(_abonent), caller(_caller) {}
  virtual ~NotifyGetAdapter() {}

  virtual bool isNull(const char* key)
    throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_ABONENT)     return (abonent.length() <= 0);
    else if (param == MSG_TEMPLATE_PARAM_CALLER) return ( caller.length() <= 0);
    keyIsNotSupported(key);
    return true;
  }
  virtual const char* getString(const char* key)
    throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_ABONENT)
      return abonent.c_str();
    else if (param == MSG_TEMPLATE_PARAM_CALLER)
      return caller.c_str();
    keyIsNotSupported(key);
    return 0;
  }

  virtual int8_t   getInt8  (const char* key) throw(AdapterException) { typeIsNotSupported("int8");   return 0; }
  virtual int16_t  getInt16 (const char* key) throw(AdapterException) { typeIsNotSupported("int16");  return 0; }
  virtual int32_t  getInt32 (const char* key) throw(AdapterException) { typeIsNotSupported("int32");  return 0; }
  virtual int64_t  getInt64 (const char* key) throw(AdapterException) { typeIsNotSupported("int64");  return 0; }
  virtual uint8_t  getUint8 (const char* key) throw(AdapterException) { typeIsNotSupported("uint8");  return 0; }
  virtual uint16_t getUint16(const char* key) throw(AdapterException) { typeIsNotSupported("uint16"); return 0; }
  virtual uint32_t getUint32(const char* key) throw(AdapterException) { typeIsNotSupported("uint32"); return 0; }
  virtual uint64_t getUint64(const char* key) throw(AdapterException) { typeIsNotSupported("uint64"); return 0; }
  virtual time_t getDateTime(const char* key) throw(AdapterException) { typeIsNotSupported("date");   return 0; }
  virtual float  getFloat(const char* key)    throw(AdapterException) { typeIsNotSupported("float");  return 0; }
  virtual double getDouble(const char* key)   throw(AdapterException) { typeIsNotSupported("double"); return 0; }
  virtual long double getLongDouble(const char* key) throw(AdapterException) {
    typeIsNotSupported("long-double"); return 0;
  }
};

class MessageGetAdapter : public NotifyGetAdapter
{
  std::string rows;
  int32_t     total;

public:
  MessageGetAdapter(const std::string& abonent, const std::string& _rows, int32_t _total)
    : NotifyGetAdapter(abonent, ""), rows(_rows), total(_total) {}
  virtual ~MessageGetAdapter() {}

  virtual bool isNull(const char* key)
    throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_ABONENT)    return (abonent.length() <= 0);
    else if (param == MSG_TEMPLATE_PARAM_ROWS)  return (rows.length() <= 0);
    else if (param == MSG_TEMPLATE_PARAM_TOTAL) return (total < 0);
    keyIsNotSupported(key);
    return true;
  }

  virtual const char* getString(const char* key) 
    throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_ABONENT)   return abonent.c_str();
    else if (param == MSG_TEMPLATE_PARAM_ROWS) return rows.c_str();
    keyIsNotSupported(key);
    return 0;
  }

  virtual int32_t  getInt32 (const char* key) throw(AdapterException)
  { 
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_TOTAL) return total;
    keyIsNotSupported(key);
    return 0;
  }

  virtual int8_t   getInt8  (const char* key) throw(AdapterException) { return (int8_t  )getInt32(key); }
  virtual int16_t  getInt16 (const char* key) throw(AdapterException) { return (int16_t )getInt32(key); }
  virtual int64_t  getInt64 (const char* key) throw(AdapterException) { return (int64_t )getInt32(key); }
  virtual uint8_t  getUint8 (const char* key) throw(AdapterException) { return (uint8_t )getInt32(key); }
  virtual uint16_t getUint16(const char* key) throw(AdapterException) { return (uint16_t)getInt32(key); }
  virtual uint32_t getUint32(const char* key) throw(AdapterException) { return (uint32_t)getInt32(key); }
  virtual uint64_t getUint64(const char* key) throw(AdapterException) { return (uint64_t)getInt32(key); }
};

class InformGetAdapter  : public NotifyGetAdapter
{
protected:

  int32_t     count;
  time_t      date;

public:

  InformGetAdapter(const std::string& _abonent, const std::string& _caller, int32_t _count, time_t _date)
    : NotifyGetAdapter(_abonent, _caller), count(_count), date(_date) {}
  virtual ~InformGetAdapter() {}

  virtual bool isNull(const char* key) 
    throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_ABONENT)     return (abonent.length() <= 0);
    else if (param == MSG_TEMPLATE_PARAM_CALLER) return (caller.length() <= 0);
    else if (param == MSG_TEMPLATE_PARAM_COUNT)  return (count < 0);
    else if (param == MSG_TEMPLATE_PARAM_DATE)   return (date <= 0);
    keyIsNotSupported(key);
    return true;
  }
  virtual const char* getString(const char* key)
    throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_ABONENT)     return abonent.c_str();
    else if (param == MSG_TEMPLATE_PARAM_CALLER) return caller.c_str();
    keyIsNotSupported(key);
    return 0;
  }
  virtual int32_t  getInt32 (const char* key) throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_COUNT)  return count;
    keyIsNotSupported(key);
    return 0;
  }
  virtual time_t   getDateTime(const char* key) throw(AdapterException)
  {
    const char* param = findMessageTemplateKey(key);
    if (param == MSG_TEMPLATE_PARAM_DATE)   return date;
    keyIsNotSupported(key);
    return 0;
  }

  virtual int8_t   getInt8  (const char* key) throw(AdapterException) { return (int8_t  )getInt32(key); }
  virtual int16_t  getInt16 (const char* key) throw(AdapterException) { return (int16_t )getInt32(key); }
  virtual int64_t  getInt64 (const char* key) throw(AdapterException) { return (int64_t )getInt32(key); }
  virtual uint8_t  getUint8 (const char* key) throw(AdapterException) { return (uint8_t )getInt32(key); }
  virtual uint16_t getUint16(const char* key) throw(AdapterException) { return (uint16_t)getInt32(key); }
  virtual uint32_t getUint32(const char* key) throw(AdapterException) { return (uint32_t)getInt32(key); }
  virtual uint64_t getUint64(const char* key) throw(AdapterException) { return (uint64_t)getInt32(key); }
};

}}

#endif // SMSC_MCI_SME_MESSAGES
