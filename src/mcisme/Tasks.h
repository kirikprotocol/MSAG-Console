#ifndef SMSC_MCI_SME_TASKS
#define SMSC_MCI_SME_TASKS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

#include <logger/Logger.h>

#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <db/DataSource.h>

#include "Statistics.h"
#include "misscall/callproc.hpp"

namespace smsc { namespace mcisme
{
    using namespace smsc::misscall;
    
    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    using namespace smsc::db;

    using smsc::logger::Logger;

    struct Message
    {
        uint64_t    id;
        uint32_t    attempts;
        std::string abonent, message, smsc_id;
        bool        cancel, notification;
        int         rowsCount, eventsCount;

        static int  maxRowsPerMessage;
        
        Message() { reset(); };
        Message(const Message& msg) 
            : id(msg.id), attempts(msg.attempts), abonent(msg.abonent), message(msg.message),
              smsc_id(msg.smsc_id), cancel(msg.cancel), notification(msg.notification),
              rowsCount(msg.rowsCount), eventsCount(msg.eventsCount) {};
        
        Message& operator=(const Message& msg) {
            id = msg.id; attempts = msg.attempts;
            abonent = msg.abonent; message = msg.message; smsc_id = msg.smsc_id; 
            cancel = msg.cancel; notification = msg.notification;
            rowsCount = msg.rowsCount; eventsCount  = msg.eventsCount; 
            return (*this);
        };

        inline void reset(const std::string& _abonent="") {
            id = 0; attempts = 0; this->abonent = _abonent;
            message = ""; smsc_id = ""; cancel = false; notification = false;
            rowsCount = 0; eventsCount = 0; 
        };
        inline bool isFull() {
            return (rowsCount >= Message::maxRowsPerMessage);
        };
    };
    
    class MessageFormatter
    {
    private:

        bool                    separate;
        Hash <uint32_t>         counters;
        Array<MissedCallEvent>  events;

        bool isLastFromCaller(int index);
    
    public:

        MessageFormatter(bool _separate) : separate(_separate) {};

        bool canAdd(const MissedCallEvent& event);
        void addEvent(const MissedCallEvent& event);
        void formatMessage(Message& message);
    };

    static const uint8_t MESSAGE_UNKNOWNST  =  0; // Неизвестное состояние
    static const uint8_t MESSAGE_WAIT_RESP  = 10; // Ожидает submit responce или готова к отправке
    static const uint8_t MESSAGE_WAIT_CNCL  = 20; // Ожидает cancel responce или готова к отмене
    static const uint8_t MESSAGE_WAIT_RCPT  = 30; // Ожидает delivery reciept

    typedef enum {
        UNKNOWNST   = MESSAGE_UNKNOWNST,
        WAIT_RESP   = MESSAGE_WAIT_RESP,
        WAIT_CNCL   = MESSAGE_WAIT_CNCL,
        WAIT_RCPT   = MESSAGE_WAIT_RCPT
    } MessageState;

    struct AbonentProfile
    {
        bool inform, notify, separate;

        AbonentProfile(bool _inform=false, bool _notify=false, bool _separate=false)
            : inform(_inform), notify(_notify), separate(_separate) {};
        AbonentProfile(const AbonentProfile& pro)
            : inform(pro.inform), notify(pro.notify), separate(pro.separate) {};
        AbonentProfile& operator=(const AbonentProfile& pro) {
            inform = pro.inform; notify = pro.notify; separate = pro.separate;
            return (*this);
        };
    };
    
    struct TaskEvent : public MissedCallEvent
    {
        uint64_t    id, msg_id;

        TaskEvent(const TaskEvent& evt) 
            : id(evt.id), msg_id(evt.msg_id) { from=evt.from; to=evt.to; time=evt.time;};
        TaskEvent(const MissedCallEvent& evt, uint64_t _id, uint64_t _msg_id=0) 
            : id(_id), msg_id(_msg_id) { from=evt.from; to=evt.to; time=evt.time; };
        TaskEvent(std::string _from="", std::string _to="", time_t _time=0, uint64_t _id=0, uint64_t _msg_id=0)
            : id(_id), msg_id(_msg_id) { from = _from; to = _to; time = _time; };
    };

    class Task
    {
    private:
        
        static DataSource*  ds;
        static Statistics*  statistics;
        static Logger*      logger;
        static uint64_t     currentId, sequenceId; // id generation sequence control
        
        uint64_t            currentMessageId;
        MessageState        currentMessageState;
        std::string         abonent, cur_smsc_id;
        Array<TaskEvent>    events;
        int                 newEventsCount;

        void loadup(uint64_t currId, Connection* connection=0); // used from loadup() & loadupAll()
        void doWait(Connection* connection, const char* smsc_id, const MessageState& state);
        void doNewCurrent(Connection* connection);

    public:

        static bool         bInformAll, bNotifyAll, bSeparateAll;

        static void         init(DataSource* _ds, Statistics* _statistics, int rowsPerMessage);
        static uint64_t     getNextId(Connection* connection=0);
        
        static bool         getMessage(const char* smsc_id, Message& message, Connection* connection=0);
        static Hash<Task *> loadupAll();

        static bool delProfile(const char* abonent);
        static void setProfile(const char* abonent, const AbonentProfile& profile);
        static AbonentProfile getProfile(const char* abonent);

        Task(const std::string& _abonent)  
            : currentMessageId(0), currentMessageState(UNKNOWNST),
              abonent(_abonent), cur_smsc_id(""), newEventsCount(0) {};
        virtual ~Task() {};
        
        void loadup();

        inline int getEventsCount() { return events.Count(); };
        inline int getNewEventsCount() { return newEventsCount; };
        inline const std::string& getAbonent() { return abonent; };
        inline const std::string& getCurrentSmscId() { return cur_smsc_id; };
        inline uint64_t getCurrentMessageId() { return currentMessageId; };
        inline const MessageState getCurrentState() { return currentMessageState; };
        
        // Adds new event to chain & inserts inassigned event to DB
        void addEvent(const MissedCallEvent& event);

        // Formats message from events chain to message capacity, assigns events to current message
        bool formatMessage(Message& message);

        // Deletes message (by msg_id if defined, else getMessage(smsc_id)) & all assigned events
        // Returns set of deleted events (for notification(s) processing) by caller
        Array<std::string> finalizeMessage(const char* smsc_id, bool delivered, bool retry, uint64_t msg_id=0);

        void waitCancel  (const char* smsc_id); // Makes current message WAIT_CNCL & set smsc_id
        void waitReceipt (const char* smsc_id); // Makes current message WAIT_RCPT & set smsc_id
        void waitResponce();                    // Makes current message WAIT_RESP & set smsc_id to null

        // Makes current message WAIT_RCPT & create new current message & shifts task events by eventCount
        void waitReceipt(int eventCount, const char* smsc_id);
    };

}}

#endif // SMSC_MCI_SME_TASKS

