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
        bool        replace, notify;
        std::string abonent, message, smsc_id;
        int         eventCount;

        static int countEvents(const std::string& message);

        Message(uint64_t id=0, std::string abonent="",
                std::string message="", std::string smsc_id="", bool replace=false) 
            : id(id), abonent(abonent), message(message), smsc_id(smsc_id), replace(replace),
              eventCount(countEvents(message)) {};
        Message(const Message& msg) 
            : id(msg.id), abonent(msg.abonent), message(msg.message), smsc_id(msg.smsc_id),
              replace(msg.replace), eventCount(msg.eventCount) {};
        
        Message& operator=(const Message& msg) {
            id = msg.id; eventCount = msg.eventCount;
            abonent = msg.abonent; message = msg.message;
            smsc_id = msg.smsc_id; replace = msg.replace;
            return (*this);
        };

        bool addEvent(const MissedCallEvent& event, bool force=false);
    };
    
    static const uint8_t MESSAGE_NEW_STATE  = 0;  // Новое или перешедуленное сообщение
    static const uint8_t MESSAGE_WAIT_RESP  = 10; // Ожидает submit|replace responce 
    static const uint8_t MESSAGE_WAIT_RCPT  = 20; // Ожидает delivery reciept

    typedef enum {
        NEW_STATE   = MESSAGE_NEW_STATE,
        WAIT_RESP   = MESSAGE_WAIT_RESP,
        WAIT_RCPT   = MESSAGE_WAIT_RCPT
    } MessageState;
    
    class Task : public Mutex
    {
    private:
        
        static Logger*      logger;
        static DataSource*  ds;
        static uint64_t     currentId, sequenceId;

        std::string     abonent;
        Array<Message>  messages; // Messages for abonent, 0 - current
        bool            bUpdated;

        void insertNewEvent(Connection* connection, 
                            const MissedCallEvent& event, bool setCurrent=false);
        void updateMessageText(Connection* connection, const Message& message);

    public:

        static void init(DataSource* _ds);
        static uint64_t getNextId(Connection* connection=0);

        Task(std::string abonent) : abonent(abonent), bUpdated(false) {};
        virtual ~Task() {};

        inline bool wasUpdated() { // check wether current message was updated while whaiting responce
            return bUpdated;
        };
        
        void loadMessages(Connection* connection=0); // loadup task messages

        void addEvent(const MissedCallEvent& event);
        
        bool getMessage(Message& message);
        bool nextMessage(const char* smsc_id, Message& message);
        //void skipFirstNEvents();
        void deleteAllMessages();
    };

    /* TODO: Implement it ???
    class AbonentTask : public Task {};
    class CallerTask : public Task {};
    */
        
}}

#endif // SMSC_MCI_SME_TASKS

