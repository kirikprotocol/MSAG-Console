#ifndef ARCHIVE_DAEMON_COMMUNICATOR_DECLARATIONS
#define ARCHIVE_DAEMON_COMMUNICATOR_DECLARATIONS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <string>

#include <logger/Logger.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/TmpBuf.hpp>
#include <core/network/Socket.hpp>
#include <util/Exception.hpp>

#include <sms/sms.h>

#include "index/SmsIndex.hpp"

namespace smsc { namespace store
{
    using smsc::logger::Logger;
    using smsc::util::Exception;
    
    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    using namespace smsc::store::index;
    using namespace smsc::sms;

    using smsc::core::network::Socket;

    struct EOFException : public Exception
    {
        EOFException() : Exception() {};
        EOFException(const char* fmt, ...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        virtual ~EOFException() throw() {};
    };
    struct CommunicationException : public Exception
    {
        CommunicationException() : Exception() {};
        CommunicationException(const char* fmt, ...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        virtual ~CommunicationException() throw() {};
    };
    
    const uint8_t SMSC_BYTE_UNDEF_TYPE = (uint8_t) 0;
    const uint8_t SMSC_BYTE_QUERY_TYPE = (uint8_t)10;
    const uint8_t SMSC_BYTE_COUNT_TYPE = (uint8_t)20;
    const uint8_t SMSC_BYTE_TOTAL_TYPE = (uint8_t)30;
    const uint8_t SMSC_BYTE_RSSMS_TYPE = (uint8_t)40;
    const uint8_t SMSC_BYTE_RSNXT_TYPE = (uint8_t)50;
    const uint8_t SMSC_BYTE_EMPTY_TYPE = (uint8_t)60;
    const uint8_t SMSC_BYTE_ERROR_TYPE = (uint8_t)70;
    
    struct Message
    {
        typedef enum {
            UNDEF = SMSC_BYTE_UNDEF_TYPE, QUERY = SMSC_BYTE_QUERY_TYPE,
            COUNT = SMSC_BYTE_COUNT_TYPE, TOTAL = SMSC_BYTE_TOTAL_TYPE,
            RSSMS = SMSC_BYTE_RSSMS_TYPE, RSNXT = SMSC_BYTE_RSNXT_TYPE,
            EMPTY = SMSC_BYTE_EMPTY_TYPE, ERROR = SMSC_BYTE_ERROR_TYPE 
        } Type;

        Message::Type type;
        
        static Message* create(uint8_t type);
        
        static void write(Socket* socket, const void* buffer, size_t size);
        static void read(Socket* socket, void* buffer, size_t size);
        static void write(Socket* socket, const std::string& str);
        static void read(Socket* socket, std::string& str);
        
        virtual void send(Socket* socket);
        virtual void receive(Socket* socket);
        
        virtual ~Message() {};
    
    protected:
        
        Message(Message::Type type=UNDEF) : type(type) {};
    };
    
    struct EmptyMessage : public Message
    {
        EmptyMessage() : Message(EMPTY) {};
        virtual ~EmptyMessage() {};
    };
    struct ErrorMessage : public Message
    {
        std::string error;

        ErrorMessage(std::string msg="") : Message(ERROR), error(msg) {};
        virtual ~ErrorMessage() {};
        
        virtual void send(Socket* socket);
        virtual void receive(Socket* socket);
    };
    struct QueryMessage : public Message
    {
        time_t          fromDate, tillDate;
        bool            bSrcMask, bDstMask, bAbnMask;
        std::string     srcMask, dstMask, abnMask;
        int32_t         status, lastResult;
        Array<Param>    parameters;
        
        QueryMessage(Message::Type type=QUERY) : Message(type), fromDate(-1), tillDate(-1), 
            bSrcMask(false), bDstMask(false), bAbnMask(false), 
            srcMask(""), dstMask(""), abnMask(""), status(-1), lastResult(-1) {};
        virtual ~QueryMessage() {};

        virtual void send(Socket* socket);
        virtual void receive(Socket* socket);
    };
    struct CountMessage : public QueryMessage
    {
        CountMessage() : QueryMessage(COUNT) {};
        virtual ~CountMessage() {};
    };
    struct TotalMessage : public Message
    {
        uint64_t    total;

        TotalMessage(uint64_t _total=0) : Message(TOTAL), total(_total) {};
        virtual ~TotalMessage() {};
        
        virtual void send(Socket* socket);
        virtual void receive(Socket* socket);
    };
    struct RsNxtMessage : public Message
    {
        uint32_t next;

        RsNxtMessage() : Message(RSNXT), next(1) {};
        virtual ~RsNxtMessage() {};
        
        virtual void receive(Socket* socket);
    };
    struct RsSmsMessage : public Message
    {
        const SMSId& id;
        const SMS&   sms;

        RsSmsMessage(const SMSId& _id, const SMS& _sms) 
            : Message(RSSMS), id(_id), sms(_sms) {};
        virtual ~RsSmsMessage() {};
        
        virtual void send(Socket* socket);
        virtual void receive(Socket* socket);
    };

    class DaemonCommunicator
    {
        Socket* socket;
    
    public:

        DaemonCommunicator(Socket* _socket) : socket(_socket) {};
        virtual ~DaemonCommunicator() {};

        void send(Message* message);
        Message* receive();
    };
}}

#endif // ARCHIVE_DAEMON_COMMUNICATOR_DECLARATIONS


