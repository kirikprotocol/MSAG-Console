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
        int         eventCount;
        bool        replace;
        
        Message(uint64_t id=0, std::string abonent="",
                std::string _message="", std::string smsc_id="", bool replace=false) 
            : id(id), attempts(0), abonent(abonent), message(_message),
              smsc_id(smsc_id), eventCount(Message::countEvents(_message)), replace(replace) {};
        Message(const Message& msg) 
            : id(msg.id), attempts(msg.attempts), abonent(msg.abonent), message(msg.message),
              smsc_id(msg.smsc_id), eventCount(msg.eventCount), replace(msg.replace) {};
        
        Message& operator=(const Message& msg) {
            id = msg.id; attempts = msg.attempts;
            abonent = msg.abonent; message = msg.message;
            smsc_id = msg.smsc_id; replace = msg.replace;
            eventCount = msg.eventCount;
            return (*this);
        };

        static int countEvents(const std::string& message);

        bool addEvent(const MissedCallEvent& event, bool force=false);
        bool isFull();
    };
    
    static const uint8_t MESSAGE_WAIT_RESP  = 10; // ќжидает submit|replace responce или готова к доставке
    static const uint8_t MESSAGE_WAIT_RCPT  = 20; // ќжидает delivery reciept

    typedef enum {
        WAIT_RESP   = MESSAGE_WAIT_RESP,
        WAIT_RCPT   = MESSAGE_WAIT_RCPT
    } MessageState;
    
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
        static Logger*      logger;
        static uint64_t     currentId, sequenceId;

        bool                bNeedReplace;
        std::string         abonent, cur_smsc_id;
        uint64_t            currentMessageId;
        Array<TaskEvent>    events;

        void loadup(uint64_t currId, Connection* connection=0); // used from loadup() & loadupAll()
        void doWait(Connection* connection, const char* smsc_id, const MessageState& state);
        void doNewCurrent(Connection* connection);

    public:

        static void         init(DataSource* _ds);
        static uint64_t     getNextId(Connection* connection=0);
        
        static bool getMessage(const char* smsc_id, Message& message, Connection* connection=0);
        static Hash<Task *> loadupAll();

        Task(std::string abonent)  
            : bNeedReplace(false), abonent(abonent), cur_smsc_id(""), currentMessageId(0) {};
        virtual ~Task() {};
        
        void loadup();

        inline int eventsCount() { return events.Count(); };
        inline const std::string& getAbonent() { return abonent; };
        
        // Adds new event to chain & inserts inassigned event to DB
        void addEvent(const MissedCallEvent& event);

        // Formats message from events chain to message capacity, assigns events to current message
        bool formatMessage(Message& message);

        void deleteMessages();

        // Deletes message (by msg_id if defined, else getMessage(smsc_id)) & all assigned events
        // Returns set of deleted events (for notification(s) processing) by caller
        Array<std::string> finalizeMessage(const char* smsc_id, bool delivered, bool retry, uint64_t msg_id=0);

        void waitResponce(const char* smsc_id); // Makes current message WAIT_RESP & set smsc_id
        void waitReceipt (const char* smsc_id); // Makes current message WAIT_RCPT & set smsc_id

        // Makes current message WAIT_RCPT & create new current message & shifts task events by eventCount
        void waitReceipt(int eventCount, const char* smsc_id);
    };

}}

#endif // SMSC_MCI_SME_TASKS

