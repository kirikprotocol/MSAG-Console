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
        int         seqNumber;
        bool        replace, notify;
        std::string abonent, message;

        Message(uint64_t id=0, int seq=0, std::string abonent="", std::string message="",
                bool replace=false, bool notify=false) 
            : id(id), seqNumber(seq), abonent(abonent), message(message), replace(replace), notify(notify) {};
        Message(const Message& msg) 
            : id(msg.id), seqNumber(msg.seqNumber), abonent(msg.abonent), message(msg.message),
              replace(msg.replace), notify(msg.notify) {};
        
        Message& operator=(const Message& msg) {
            id = msg.id; seqNumber = msg.seqNumber;
            abonent = msg.abonent; message = msg.message;
            replace = msg.replace; notify = msg.notify;
            return (*this);
        };
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
        
        static smsc::logger::Logger *logger;
        static DataSource*  ds;

        std::string     abonent;
        Array<Message>  messages; // Messages for abonent, 0 - current

    public:

        static void init(DataSource* _ds) {
            Task::logger = Logger::getInstance("smsc.mcisme.Task");
            Task::ds = _ds;
        };

        Task(std::string abonent) : abonent(abonent) {};
        virtual ~Task() {};

        void load(); // loadup messages on startup
        void roll(); // roll to next message (if available)

        void addEvent(const MissedCallEvent& event);
        void formatMessage(Message& message);
    };

    /* TODO: Implement it ???
    class AbonentTask : public Task {};
    class CallerTask : public Task {};
    */
        
}}

#endif // SMSC_MCI_SME_TASKS

