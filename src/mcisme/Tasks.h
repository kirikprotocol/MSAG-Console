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
        std::string abonent, message;
        int         eventCount;

        Message(uint64_t id=0, std::string abonent="", std::string message="",
                bool replace=false, bool notify=false) 
            : id(id), abonent(abonent), message(message),
              replace(replace), notify(notify), eventCount(0) {};
        Message(const Message& msg) 
            : id(msg.id), abonent(msg.abonent), message(msg.message),
              replace(msg.replace), notify(msg.notify), eventCount(0) {};
        
        Message& operator=(const Message& msg) {
            id = msg.id; eventCount = msg.eventCount;
            abonent = msg.abonent; message = msg.message;
            replace = msg.replace; notify = msg.notify;
            return (*this);
        };

        bool addEvent(const MissedCallEvent& event, bool force=false);
    };
    
    static const uint8_t MESSAGE_NEW_STATE          = 0;  // Новое или перешедуленное сообщение
    static const uint8_t MESSAGE_WAIT_STATE         = 10; // Ожидает submitResponce
    static const uint8_t MESSAGE_ENROUTE_STATE      = 20; // В процессе доставки, ожидает deliveryReciept

    typedef enum {
        NEW         = MESSAGE_NEW_STATE,
        WAIT        = MESSAGE_WAIT_STATE,
        ENROUTE     = MESSAGE_ENROUTE_STATE
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
        
        void load(); // loadup messages on startup
        void roll(); // roll to next message (if available)

        void addEvent(const MissedCallEvent& event);
        bool getMessage(Message& message);
        
        void rollCurrent(const char* smsc_id);
    };

    /* TODO: Implement it ???
    class AbonentTask : public Task {};
    class CallerTask : public Task {};
    */
        
}}

#endif // SMSC_MCI_SME_TASKS

