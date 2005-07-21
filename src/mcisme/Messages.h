#ifndef SMSC_MCI_SME_MESSAGES
#define SMSC_MCI_SME_MESSAGES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include <string>

#include <core/buffers/TmpBuf.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include "Templates.h"
#include "misscall/callproc.hpp"

namespace smsc { namespace mcisme
{
    using namespace smsc::misscall;
    
    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    
    struct Message
    {
        uint64_t    id;
        uint32_t    attempts;
        std::string abonent, message, smsc_id;
        bool        cancel, notification, skip;
        int         rowsCount, eventsCount;

        static int  maxRowsPerMessage;
        
        Message() { reset(); };
        Message(const Message& msg) 
            : id(msg.id), attempts(msg.attempts), abonent(msg.abonent), message(msg.message),
              smsc_id(msg.smsc_id), cancel(msg.cancel), notification(msg.notification), 
              skip(msg.skip), rowsCount(msg.rowsCount), eventsCount(msg.eventsCount) {};
        
        Message& operator=(const Message& msg) {
            id = msg.id; attempts = msg.attempts;
            abonent = msg.abonent; message = msg.message; smsc_id = msg.smsc_id; 
            cancel = msg.cancel; notification = msg.notification; skip = msg.skip;
            rowsCount = msg.rowsCount; eventsCount  = msg.eventsCount; 
            return (*this);
        };

        inline void reset(const std::string& _abonent="") {
            id = 0; attempts = 0; this->abonent = _abonent;
            message = ""; smsc_id = ""; skip = false;
            cancel = false; notification = false; 
            rowsCount = 0; eventsCount = 0; 
        };
        inline bool isFull() {
            return (rowsCount >= Message::maxRowsPerMessage);
        };
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

        InformTemplateFormatter*    formatter;

        Hash <uint32_t>             counters;
        Array<MissedCallEvent>      events;
        
        bool isLastFromCaller(int index);
    
    public:

        MessageFormatter(InformTemplateFormatter* _formatter) : formatter(_formatter) {};

        bool canAdd(const MissedCallEvent& event);
        void addEvent(const MissedCallEvent& event);
        void formatMessage(Message& message, int timeOffset=0);
    };

    void keyIsNotSupported(const char* type) throw(AdapterException);
    void typeIsNotSupported(const char* type) throw(AdapterException);
    const char* findMessageTemplateKey(const char* key) throw(AdapterException);

    class NotifyGetAdapter : public GetAdapter
    {
    protected:

        std::string abonent, caller;

    public:    

        NotifyGetAdapter(const std::string& _abonent="", const std::string& _caller="")
            : GetAdapter(), abonent(_abonent), caller(_caller) {};
        virtual ~NotifyGetAdapter() {};

        virtual bool isNull(const char* key) 
            throw(AdapterException)
        {
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_ABONENT)     return (abonent.length() <= 0);
            else if (param == MSG_TEMPLATE_PARAM_CALLER) return ( caller.length() <= 0);
            keyIsNotSupported(key);
            return true;
        };
        virtual const char* getString(const char* key) 
            throw(AdapterException)
        {
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_ABONENT)     return abonent.c_str();
            else if (param == MSG_TEMPLATE_PARAM_CALLER) return caller.c_str();
            keyIsNotSupported(key);
            return 0;
        };

        virtual int8_t   getInt8  (const char* key) throw(AdapterException) { typeIsNotSupported("int8");   return 0; };
        virtual int16_t  getInt16 (const char* key) throw(AdapterException) { typeIsNotSupported("int16");  return 0; };
        virtual int32_t  getInt32 (const char* key) throw(AdapterException) { typeIsNotSupported("int32");  return 0; };
        virtual int64_t  getInt64 (const char* key) throw(AdapterException) { typeIsNotSupported("int64");  return 0; };
        virtual uint8_t  getUint8 (const char* key) throw(AdapterException) { typeIsNotSupported("uint8");  return 0; };
        virtual uint16_t getUint16(const char* key) throw(AdapterException) { typeIsNotSupported("uint16"); return 0; };
        virtual uint32_t getUint32(const char* key) throw(AdapterException) { typeIsNotSupported("uint32"); return 0; };
        virtual uint64_t getUint64(const char* key) throw(AdapterException) { typeIsNotSupported("uint64"); return 0; };
        virtual time_t getDateTime(const char* key) throw(AdapterException) { typeIsNotSupported("date");   return 0; };
        virtual float  getFloat(const char* key)    throw(AdapterException) { typeIsNotSupported("float");  return 0; };
        virtual double getDouble(const char* key)   throw(AdapterException) { typeIsNotSupported("double"); return 0; };
        virtual long double getLongDouble(const char* key) throw(AdapterException) { 
            typeIsNotSupported("long-double"); return 0; 
        };
    };

    class MessageGetAdapter : public NotifyGetAdapter
    {
        std::string rows;
        int32_t     total;

    public:

        MessageGetAdapter(const std::string& abonent, const std::string& _rows, int32_t _total)
            : NotifyGetAdapter(abonent, ""), rows(_rows), total(_total) {};
        virtual ~MessageGetAdapter() {};

        virtual bool isNull(const char* key) 
            throw(AdapterException)
        {
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_ABONENT)    return (abonent.length() <= 0);
            else if (param == MSG_TEMPLATE_PARAM_ROWS)  return (rows.length() <= 0);
            else if (param == MSG_TEMPLATE_PARAM_TOTAL) return (total < 0);
            keyIsNotSupported(key);
            return true;
        };
        virtual const char* getString(const char* key) 
            throw(AdapterException)
        {
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_ABONENT)   return abonent.c_str();
            else if (param == MSG_TEMPLATE_PARAM_ROWS) return rows.c_str();
            keyIsNotSupported(key);
            return 0;
        };
        virtual int32_t  getInt32 (const char* key) throw(AdapterException)
        { 
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_TOTAL) return total;
            keyIsNotSupported(key);
            return 0;
        };

        virtual int8_t   getInt8  (const char* key) throw(AdapterException) { return (int8_t  )getInt32(key); };
        virtual int16_t  getInt16 (const char* key) throw(AdapterException) { return (int16_t )getInt32(key); };
        virtual int64_t  getInt64 (const char* key) throw(AdapterException) { return (int64_t )getInt32(key); };
        virtual uint8_t  getUint8 (const char* key) throw(AdapterException) { return (uint8_t )getInt32(key); };
        virtual uint16_t getUint16(const char* key) throw(AdapterException) { return (uint16_t)getInt32(key); };
        virtual uint32_t getUint32(const char* key) throw(AdapterException) { return (uint32_t)getInt32(key); };
        virtual uint64_t getUint64(const char* key) throw(AdapterException) { return (uint64_t)getInt32(key); };
    };

    class InformGetAdapter  : public NotifyGetAdapter
    {
    protected:

        int32_t     count;
        time_t      date;

    public:    

        InformGetAdapter(const std::string& _abonent, const std::string& _caller, int32_t _count, time_t _date)
            : NotifyGetAdapter(_abonent, _caller), count(_count), date(_date) {};
        virtual ~InformGetAdapter() {};

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
        };
        virtual const char* getString(const char* key) 
            throw(AdapterException)
        {
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_ABONENT)     return abonent.c_str();
            else if (param == MSG_TEMPLATE_PARAM_CALLER) return caller.c_str();
            keyIsNotSupported(key);
            return 0;
        };
        virtual int32_t  getInt32 (const char* key) throw(AdapterException)
        { 
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_COUNT)  return count;
            keyIsNotSupported(key);
            return 0;
        };
        virtual time_t   getDateTime(const char* key) throw(AdapterException)
        { 
            const char* param = findMessageTemplateKey(key);
            if (param == MSG_TEMPLATE_PARAM_DATE)   return date;
            keyIsNotSupported(key);
            return 0; 
        };

        virtual int8_t   getInt8  (const char* key) throw(AdapterException) { return (int8_t  )getInt32(key); };
        virtual int16_t  getInt16 (const char* key) throw(AdapterException) { return (int16_t )getInt32(key); };
        virtual int64_t  getInt64 (const char* key) throw(AdapterException) { return (int64_t )getInt32(key); };
        virtual uint8_t  getUint8 (const char* key) throw(AdapterException) { return (uint8_t )getInt32(key); };
        virtual uint16_t getUint16(const char* key) throw(AdapterException) { return (uint16_t)getInt32(key); };
        virtual uint32_t getUint32(const char* key) throw(AdapterException) { return (uint32_t)getInt32(key); };
        virtual uint64_t getUint64(const char* key) throw(AdapterException) { return (uint64_t)getInt32(key); };
    };

}}

#endif // SMSC_MCI_SME_MESSAGES
