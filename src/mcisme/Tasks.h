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

#include "Messages.h"
#include "Profiler.h"
#include "Statistics.h"
#include "misscall/callproc.hpp"

namespace smsc { namespace mcisme
{
    using namespace smsc::misscall;
    
    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    using namespace smsc::db;

    using smsc::logger::Logger;

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

    struct TaskEvent : public MissedCallEvent
    {
        uint64_t    id, msg_id;

        TaskEvent(const TaskEvent& evt) 
            : id(evt.id), msg_id(evt.msg_id) { from=evt.from; to=evt.to; time=evt.time; cause=evt.cause; };
        TaskEvent(const MissedCallEvent& evt, uint64_t _id, uint64_t _msg_id=0) 
            : id(_id), msg_id(_msg_id) { from=evt.from; to=evt.to; time=evt.time; cause=evt.cause; };
        TaskEvent(std::string _from="", std::string _to="", time_t _time=0,
                  uint64_t _id=0, uint64_t _msg_id=0, uint8_t _cause=ALL)
            : id(_id), msg_id(_msg_id) { from = _from; to = _to; time = _time; cause = _cause; };
    };

    class Task
    {
    private:
        
        static DataSource*  ds;
        static Statistics*  statistics;
        static Logger*      logger;
        static uint64_t     currentId, sequenceId; // id generation sequence control
        static int          maxCallersCount;       // maximum distinct callers
        
        uint64_t            currentMessageId;
        MessageState        currentMessageState;
        std::string         abonent, cur_smsc_id;
        Array<TaskEvent>    events;
        int                 newEventsCount, callersCount;

        AbonentProfile           abonentProfile;
        InformTemplateFormatter* templateFormatter;

        void loadCallersCount(Connection* connection);
        bool loadup(uint64_t currId, Connection* connection=0); // used from loadup() & loadupAll()
        void doWait(Connection* connection, const char* smsc_id, const MessageState& state);
        void doNewCurrent(Connection* connection);
        void clearCurrent(Connection* connection);

    public:

        static bool         bInformAll, bNotifyAll;

        static void         init(DataSource* _ds, Statistics* _statistics,
                                 int rowsPerMessage, int maxCallersCount = -1);
        static uint64_t     getNextId(Connection* connection=0);
        
        static bool         getMessage(const char* smsc_id, Message& message, 
                                       MessageState& state, Connection* connection=0);
        static Hash<Task *> loadupAll();

        Task(const std::string& _abonent)  
            : currentMessageId(0), currentMessageState(UNKNOWNST), abonent(_abonent),
              cur_smsc_id(""), newEventsCount(0), callersCount(0), templateFormatter(0) {};
        virtual ~Task() {};
        
        void loadup();
        
        inline const AbonentProfile& getAbonentProfile() {
            return abonentProfile;
        };
        inline void setTemplateFormatter(InformTemplateFormatter* formatter) {
            templateFormatter = formatter;
        };
        
        inline bool checkCallersCount() {
            return (Task::maxCallersCount <= 0 || callersCount < Task::maxCallersCount);
        };
        inline int getCallersCount() {
            return callersCount;
        };

        inline int getEventsCount() { return events.Count(); };
        inline int getNewEventsCount() { return newEventsCount; };
        inline const std::string& getAbonent() { return abonent; };
        inline const std::string& getCurrentSmscId() { return cur_smsc_id; };
        inline uint64_t getCurrentMessageId() { return currentMessageId; };
        inline const MessageState getCurrentState() { return currentMessageState; };
        
        // Adds new event to chain & inserts inassigned event to DB
        void addEvent(const MissedCallEvent& event);

        // Форматирует сообщение из текущих событий до его заполнения (если currentMessageId == 0, то создаёт новое).
        // Маркирует event'ы текущим id'ом сообщения в таблице событий.
        bool formatMessage(Message& message);

        // Deletes message (by msg_id if defined, else getMessage(smsc_id)) & all assigned events
        // Returns set of deleted events (for notification(s) processing) by caller
        Array<std::string> finalizeMessage(const char* smsc_id, bool delivered, bool retry, uint64_t msg_id=0);

        void waitCancel  (const char* smsc_id); // Makes current message WAIT_CNCL & set smsc_id
        void waitReceipt (const char* smsc_id); // Makes current message WAIT_RCPT & set smsc_id
        void waitResponce();                    // Makes current message WAIT_RESP & set smsc_id to null

        // Makes current message WAIT_RCPT & set smsc_id, 
        // shifts task events by eventCount & clear currentMessageId
        void waitReceipt(int eventCount, const char* smsc_id);
    };

}}

#endif // SMSC_MCI_SME_TASKS

