#ifndef SMSC_MCI_SME_MESSAGES
#define SMSC_MCI_SME_MESSAGES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <sys/types.h>

#include <string>

#include <core/buffers/TmpBuf.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include "Templates.h"
#include "misscall/callproc.hpp"
#include <mcisme/AbntAddr.hpp>
#include <logger/Logger.h>

namespace smsc {
namespace mcisme {
using namespace smsc::misscall;

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using smsc::logger::Logger;


namespace UDPattern {
// User Data
static const char UDHL = 0x02;
static const char IEI = 0x70;
static const char IEIDL = 0x00;
// Secured Data
static const char CPI = 0x70;
static const char CPL1 = 0x00;
static const char CPL2 = 0x00;
static const char CHI = 0x00;
static const char CHL = 0x0D;
static const char SPI1 = 0x00;
static const char SPI2 = 0x00;
static const char KIc = 0x00;
static const char KID = 0x00;
static const char TAR1 = 0x7F;
static const char TAR2 = 0x00;
static const char TAR3 = 0x10;
static const char CNTR12345 = 0x00;
static const char PCNTR = 0x00;

static const int indexCPL = 0x04;
static const int user_data_pattern_len = 19;
static const char user_data_pattern[19] = {UDHL, CPI, IEIDL, CPL1, CPL2, CHL, SPI1, SPI2, KIc, KID, TAR1, TAR2, TAR3, 0,0,0,0,0, 0};//, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
};

using namespace UDPattern;

struct Message
{
  std::string abonent, caller_abonent, message, smsc_id;
  bool        cancel, notification, skip, data_sm, secured_data;
  static int  maxRowsPerMessage;

private:
  mutable char* user_data;

  inline void reset(const std::string& _abonent="") {
    abonent = _abonent; caller_abonent = "";
    message = ""; smsc_id = ""; skip = false;
    cancel = false; notification = false; data_sm = false;
    data_sm = false; secured_data = false;
    //rowsCount = 0; eventsCount = 0; 
    if(user_data) delete[] user_data;
  }

public:        
  Message(): user_data(0), data_sm(false), secured_data(false)  { reset(); };
  ~Message(){if(user_data) delete[] user_data;};
  Message(const Message& msg) 
    : abonent(msg.abonent), caller_abonent(msg.caller_abonent), message(msg.message),
      smsc_id(msg.smsc_id), cancel(msg.cancel), notification(msg.notification),  
      skip(msg.skip), data_sm(msg.data_sm), secured_data(msg.secured_data),
      user_data(0)
  {
    user_data = new char[sizeof(msg.user_data)];
    memcpy(user_data, msg.user_data, sizeof(msg.user_data));
  }

  Message& operator=(const Message& msg) {
    if (this != &msg) {
      abonent = msg.abonent; caller_abonent = msg.caller_abonent;
      message = msg.message; smsc_id = msg.smsc_id; 
      cancel = msg.cancel; notification = msg.notification; skip = msg.skip;
      data_sm = msg.data_sm; secured_data = msg.secured_data;
      if(user_data) delete[] user_data;
      if(msg.user_data)
      {
        user_data = new char[sizeof(msg.user_data)];
        memcpy(user_data, msg.user_data, sizeof(msg.user_data));
      }
    }
    return (*this);
  }

  const char* GetMsg(void) const
  {
    if(!secured_data)
      return message.c_str();

    if(user_data) delete[] user_data;
    size_t secured_data_len = message.length();
    user_data = new char[user_data_pattern_len + secured_data_len];
    memcpy(user_data, user_data_pattern, user_data_pattern_len);
    memcpy(user_data + user_data_pattern_len, message.c_str(), secured_data_len);
    user_data[indexCPL] = static_cast<uint8_t>(CHL + secured_data_len + 1);
    return user_data;
  }

  int GetMsgLen(void) const
  {
    int len = static_cast<int>(message.length());
    if(secured_data) len += user_data_pattern_len;
    return len;
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
    
class MessageFormatter
{
private:
  smsc::logger::Logger *logger;
  InformTemplateFormatter*    formatter;

  Hash <uint32_t>             counters;
  Array<MissedCallEvent>      events;

  bool isLastFromCaller(int index);
public:

  MessageFormatter(InformTemplateFormatter* _formatter) : formatter(_formatter), logger(Logger::getInstance("mci.msgfmt")) {};

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

void addBanner(Message& message, const string& banner);

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
