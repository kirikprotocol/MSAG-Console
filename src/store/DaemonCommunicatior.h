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
#include <core/network/Socket.hpp>
#include <util/Exception.hpp>

#include <sms/sms.h>

namespace smsc { namespace store
{
    using smsc::logger::Logger;
    using smsc::util::Exception;
    
    using namespace smsc::core::synchronization;
    using namespace smsc::core::threads;
    using namespace smsc::core::buffers;
    using namespace smsc::sms;

    using smsc::core::network::Socket;

    struct CommunicationException : public Exception
    {
        CommunicationException() 
            : Exception() {};
        CommunicationException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        virtual ~CommunicationException() throw() {};
    };
    
    const uint8_t SMSC_BYTE_UNDEF_TYPE = (uint8_t)0;
    const uint8_t SMSC_BYTE_QUERY_TYPE = (uint8_t)10;
    const uint8_t SMSC_BYTE_RSSMS_TYPE = (uint8_t)20;
    const uint8_t SMSC_BYTE_RSNXT_TYPE = (uint8_t)30;
    const uint8_t SMSC_BYTE_EMPTY_TYPE = (uint8_t)40;
    const uint8_t SMSC_BYTE_ERROR_TYPE = (uint8_t)50;
    
    struct Parameter
    {
        typedef enum {
            ANY=0, INT=1, ID=2, DATE=3, BOOL=4, STRING=5, PATTERN=6 
        } Type;

        Parameter::Type type;
        std::string     str;

        union {
            uint64_t    id;
            int         val;
            time_t      date;
            bool        check;
        };
        
        Parameter(Parameter::Type _type=ANY) : type(_type) {};
        Parameter(Parameter::Type _type, bool value) : type(_type) {
            __require__(type == BOOL); check = value;
        };
        Parameter(Parameter::Type _type, std::string& value) : type(_type) {
            __require__(type == STRING || type == PATTERN); str = value;
        };
        Parameter(Parameter::Type _type, uint64_t value) : type(_type) {
            __require__(type == ID || type == INT || type == DATE);
            switch (type) {
                case ID:   id = value; break;
                case INT:  val = (int)value; break;
                case DATE: date = (time_t)value; break;
                default:   type = ANY;
            }
        };
        Parameter(const Parameter& param) {
            set(param);
        };
        Parameter& operator=(const Parameter& param) {
            set(param); return (*this);
        };
        
        void set(const Parameter& param) {
            switch(type = param.type) {
                case INT: val = param.val; break;
                case ID:  id = param.id; break;
                case DATE: date = param.date; break;
                case BOOL: check = param.check; break;
                case STRING: case PATTERN: str = param.str;  break;
                default: type = ANY;
            }
        };
    };

    struct Message
    {
        typedef enum {
            UNDEF = SMSC_BYTE_UNDEF_TYPE, QUERY = SMSC_BYTE_QUERY_TYPE, 
            RSSMS = SMSC_BYTE_RSSMS_TYPE, RSNXT = SMSC_BYTE_RSNXT_TYPE,
            EMPTY = SMSC_BYTE_EMPTY_TYPE, ERROR = SMSC_BYTE_ERROR_TYPE 
        } Type;

        Message::Type type;
        
        static void write(Socket* socket, const void* buffer, size_t size)
            throw(CommunicationException)
        {
            if (!socket) throw CommunicationException("Message send failed. Socket NULL!");
            if (socket->Write((const char *)buffer, (int)size) == -1)
                throw CommunicationException("Message send failed. %s", strerror(errno));
        };
        static void read(Socket* socket, void* buffer, size_t size)
            throw(CommunicationException)
        {
            if (!socket) throw CommunicationException("Message read failed. Socket NULL!");
            if (socket->Read((char *)buffer, (int)size) == -1)
                throw CommunicationException("Message read failed. %s", strerror(errno));
        };
        static void write(Socket* socket, const std::string& str)
        {
            uint32_t strLen = htonl(str.length());
            Message::write(socket, &strLen, sizeof(strLen));
            Message::write(socket, str.c_str(), str.length());
        };
        static void read(Socket* socket, std::string& str)
        {
            uint32_t strLen = 0;
            Message::read(socket, &strLen, sizeof(strLen));
            strLen = ntohl(strLen);
            std::auto_ptr<char> bufferGuard(new char[strLen+1]);
            char* buffer = bufferGuard.get();
            Message::read(socket, buffer, strLen);
            buffer[strLen] = '\0'; str = buffer;
        };
        
        virtual void send(Socket* socket)
            throw(CommunicationException)
        {
            uint8_t t = (uint8_t)type;
            Message::write(socket, &t, sizeof(t));
        };
        virtual void receive(Socket* socket)
            throw(CommunicationException)
        {
        };
        
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
        
        virtual void send(Socket* socket)
            throw(CommunicationException)
        {
            Message::send(socket);
            Message::write(socket, error);
        };
        virtual void receive(Socket* socket)
            throw(CommunicationException)
        {
            Message::read(socket, error);
        };
    };
    struct QueryMessage : public Message
    {
        Hash<Parameter> parameters;
        
        QueryMessage() : Message(QUERY) {};
        virtual ~QueryMessage() {};

        virtual void send(Socket* socket)
            throw(CommunicationException)
        {
            // TODO: implement if needed
        };
        virtual void receive(Socket* socket)
            throw(CommunicationException)
        {
            // TODO: implement parameters scanning
        };
    };
    struct RsNxtMessage : public Message
    {
        RsNxtMessage() : Message(RSNXT) {};
        virtual ~RsNxtMessage() {};
    };
    struct RsSmsMessage : public Message
    {
        SMSId id; SMS sms;

        RsSmsMessage() : Message(RSSMS) {};
        RsSmsMessage(const SMSId _id, const SMS& _sms) 
            : Message(RSSMS), id(_id), sms(_sms) {};
        virtual ~RsSmsMessage() {};
        
        virtual void send(Socket* socket)
            throw(CommunicationException)
        {
            Message::send(socket);

            // TODO: implement
        };
        virtual void receive(Socket* socket)
            throw(CommunicationException)
        {
            // TODO: implement if needed
        };
    };

    class DaemonCommunicator
    {
        Socket* socket;
    
        Message* createMessage(uint8_t type)
        {
            switch(type)
            {
                case SMSC_BYTE_QUERY_TYPE: return new QueryMessage();
                case SMSC_BYTE_RSSMS_TYPE: return new RsSmsMessage();
                case SMSC_BYTE_RSNXT_TYPE: return new RsNxtMessage();
                case SMSC_BYTE_EMPTY_TYPE: return new EmptyMessage();
                case SMSC_BYTE_ERROR_TYPE: return new ErrorMessage();
            }
            return 0;
        };

    public:

        DaemonCommunicator(Socket* _socket) : socket(_socket) {};
        virtual ~DaemonCommunicator() {};

        void send(Message* message)
            throw(CommunicationException)
        {
            message->send(socket);
        };
        Message* receive()
            throw(CommunicationException)
        {
            uint8_t type = 0;
            Message::read(socket, &type, sizeof(type));

            std::auto_ptr<Message> messageGuard(createMessage(type));
            Message* message = messageGuard.get();
            if (!message)
                throw CommunicationException("Unsupported message %u", type);
            
            message->receive(socket);
            return message;
        };
    };
}}

#endif // ARCHIVE_DAEMON_COMMUNICATOR_DECLARATIONS


