
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <string>

#include "Uint64Converter.h"
#include "DaemonCommunicator.h"

using smsc::util::Uint64Converter;

namespace smsc { namespace store
{
    
void Message::read(Socket* socket, void* buffer, size_t size)
{
    if (!socket) throw CommunicationException("Message read failed. Socket NULL!");
    int toRead = size; char* readBuffer = (char *)buffer;
    while (toRead > 0) {
        int read = socket->canRead(10);
        if (read == 0) throw CommunicationException("Message read failed. Timeout expired.");
        else if (read > 0) {
            read = socket->Read(readBuffer, toRead);
            if (read > 0) { readBuffer+=read; toRead-=read; continue; }
        }
        throw EOFException("Message read failed. Socket closed. %s", strerror(errno));
    }
    /*
    if (size == 1) printf("Read %d: %d\n", size, *((uint8_t  *)buffer));
    if (size == 2) printf("Read %d: %d\n", size, *((uint16_t *)buffer));
    if (size == 4) printf("Read %d: %d\n", size, *((uint32_t *)buffer));
    */
}
void Message::write(Socket* socket, const void* buffer, size_t size)
{
    if (!socket) throw CommunicationException("Message send failed. Socket NULL!");
    int toWrite = size; const char* writeBuffer = (const char *)buffer;
    while (toWrite > 0) {
        int write = socket->canWrite(10);
        if (write == 0) throw CommunicationException("Message send failed. Timeout expired.");
        else if (write > 0) {
            write = socket->Write(writeBuffer, toWrite);
            if (write > 0) { writeBuffer+=write; toWrite-=write; continue; }
        }
        throw EOFException("Message send failed. Socket closed. %s", strerror(errno));
    }
    /*
    if (size == 1) printf("Write %d: %d\n", size, *((uint8_t  *)buffer));
    if (size == 2) printf("Write %d: %d\n", size, *((uint16_t *)buffer));
    if (size == 4) printf("Write %d: %d\n", size, *((uint32_t *)buffer));
    */
}
void Message::read(Socket* socket, std::string& str)
{
    uint32_t strLen = 0;
    Message::read(socket, &strLen, sizeof(strLen));
    strLen = ntohl(strLen);
    if (strLen > 0) {
        TmpBuf<char, 1024> bufferGuard(strLen+1);
        char* buffer = bufferGuard.get();
        Message::read(socket, buffer, strLen);
        buffer[strLen] = '\0'; str = buffer;
    }
    else str="";
}
void Message::write(Socket* socket, const std::string& str)
{
    int32_t  strLen = str.length();
    uint32_t writeLen = htonl(strLen);
    Message::write(socket, &writeLen, sizeof(writeLen));
    if (strLen > 0) {
        Message::write(socket, str.c_str(), strLen);
    }
}

void Message::send(Socket* socket)
{
    uint8_t t = (uint8_t)type;
    Message::write(socket, &t, sizeof(t));
}
void Message::receive(Socket* socket)
{
}
Message* Message::create(uint8_t type)
{
    switch(type)
    {
    case SMSC_BYTE_QUERY_TYPE: return new QueryMessage();
    case SMSC_BYTE_COUNT_TYPE: return new CountMessage();
    case SMSC_BYTE_RSNXT_TYPE: return new RsNxtMessage();
    case SMSC_BYTE_EMPTY_TYPE: return new EmptyMessage();
    case SMSC_BYTE_ERROR_TYPE: return new ErrorMessage();
    default:
        throw CommunicationException("Unsupported message received, type=%u", type);
    }
    return 0;
}

void ErrorMessage::send(Socket* socket)
{
    Message::send(socket);
    Message::write(socket, error);
}
void ErrorMessage::receive(Socket* socket)
{
    Message::read(socket, error);
}

void TotalMessage::send(Socket* socket)
{
    Message::send(socket);
    uint64_t value = Uint64Converter::toNetworkOrder(total);
    Message::write(socket, &value, sizeof(value));
}
void TotalMessage::receive(Socket* socket)
{
    Message::read(socket, &total, sizeof(total));
    total = Uint64Converter::toHostOrder(total);
}

static const char* PROTOCOL_DATETIME_PATTERN = "%02d.%02d.%4d %02d:%02d:%02d%n";
static time_t convertProtocolStrToDate(const char* str)
{
    tm tmdt; int bytes = 0;
    if (sscanf(str, PROTOCOL_DATETIME_PATTERN, &tmdt.tm_mday, &tmdt.tm_mon, &tmdt.tm_year,
               &tmdt.tm_hour, &tmdt.tm_min, &tmdt.tm_sec, &bytes) != 6 || bytes <= 0) {
        throw CommunicationException("Invalid date/time value format is '%s' "
                                     "Processing string: '%s'", PROTOCOL_DATETIME_PATTERN, str);
    }
    
    tmdt.tm_mon -= 1; tmdt.tm_year -= 1900; tmdt.tm_isdst = 0;
    time_t date = mktime(&tmdt); date -= timezone;
    return date;
}
static int8_t convertProtocolDateToStr(char* str, time_t date)
{
    int bytes = 0;
    if (date > 0) {
        tm tmdt; gmtime_r(&date, &tmdt); 
        sprintf(str, PROTOCOL_DATETIME_PATTERN, tmdt.tm_mday, tmdt.tm_mon+1, tmdt.tm_year+1900,
                tmdt.tm_hour, tmdt.tm_min, tmdt.tm_sec, &bytes);
    } 
    else str[0]='\0';
    return (int8_t)bytes;
}

void RsNxtMessage::receive(Socket* socket)
{
    Message::read(socket, &next, sizeof(next));
    next = ntohl(next);
}

void QueryMessage::receive(Socket* socket)
{
    parameters.Clean();
    fromDate = tillDate = -1;

    int8_t count = 0;
    Message::read(socket, &count, sizeof(count));
    if (count <= 0) return;
    
    do
    {
      Param   param; uint8_t param_type = 0; 
      Message::read(socket, &param_type, sizeof(param_type));

      switch(param_type)
      {
      case T_FROM_DATE: {
          Message::read(socket, param.sValue);
          fromDate = param.dValue = convertProtocolStrToDate(param.sValue.c_str());
          break;
      }
      case T_TILL_DATE: {
          Message::read(socket, param.sValue);
          tillDate = param.dValue = convertProtocolStrToDate(param.sValue.c_str());
          break;
      }
      case T_LAST_RESULT: case T_STATUS: {
          uint32_t value = 0;
          Message::read(socket, &value, sizeof(value));
          value = ntohl(value);
          if (param_type == T_STATUS) status = value;
          else lastResult = value;
          param.iValue = (uint64_t)value;
          break;
      }
      case T_SMS_ID: {
          uint64_t id = 0;
          Message::read(socket, &id, sizeof(id));
          param.iValue = Uint64Converter::toHostOrder(id);
          break;
      }
      case T_ABN_ADDRESS: case T_SRC_ADDRESS: case T_DST_ADDRESS:
      case T_SME_ID: case T_SRC_SME_ID: case T_DST_SME_ID: case T_ROUTE_ID: {
          Message::read(socket, param.sValue); 
          break;
      }
      default:
          throw CommunicationException("Invalid query parameter type %u", param_type);
      }
      param.type = (Param::ParamType)param_type;
      parameters.Push(param);
    }
    while (--count);
}
void QueryMessage::send(Socket* socket)
{
    // TODO: implement if needed
}

void RsSmsMessage::send(Socket* socket)
{
    Message::send(socket);

    char submitTimeBuffer[64]; char validTimeBuffer [64];
    char lastTimeBuffer  [64]; char nextTimeBuffer  [64];

    int8_t smsState     = (int8_t)sms.state;
    int8_t submitTimeSz = convertProtocolDateToStr(submitTimeBuffer, sms.submitTime);
    int8_t validTimeSz  = convertProtocolDateToStr(validTimeBuffer , sms.validTime);
    int8_t lastTimeSz   = convertProtocolDateToStr(lastTimeBuffer  , sms.lastTime);
    int8_t nextTimeSz   = convertProtocolDateToStr(nextTimeBuffer  , sms.nextTime);
    
    std::string oa      = sms.originatingAddress.toString();
    std::string da      = sms.destinationAddress.toString();
    std::string dda     = sms.dealiasedDestinationAddress.toString();
    int8_t oaSize       = oa.length();
    int8_t daSize       = da.length();
    int8_t ddaSize      = dda.length();
    
    int8_t svcSize      = strlen(sms.eServiceType);
    int8_t odMscSize    = strlen(sms.originatingDescriptor.msc);
    int8_t odImsiSize   = strlen(sms.originatingDescriptor.imsi);
    int8_t ddMscSize    = strlen(sms.destinationDescriptor.msc);
    int8_t ddImsiSize   = strlen(sms.destinationDescriptor.imsi);
    int8_t routeSize    = strlen(sms.routeId);
    int8_t srcSmeSize   = strlen(sms.srcSmeId);
    int8_t dstSmeSize   = strlen(sms.dstSmeId);
    
    int32_t bodyBuffLen = sms.messageBody.getBufferLength();

    uint32_t recordSize = sizeof(id)+sizeof(smsState)+
        sizeof(submitTimeSz)+submitTimeSz+sizeof(validTimeSz)+validTimeSz+
        sizeof(lastTimeSz)+lastTimeSz+sizeof(nextTimeSz)+nextTimeSz+
        sizeof(sms.attempts)+sizeof(sms.lastResult)+
        sizeof(oaSize)+oaSize+sizeof(daSize)+daSize+sizeof(ddaSize)+ddaSize+sizeof(sms.messageReference)+
        sizeof(svcSize)+svcSize+sizeof(sms.deliveryReport)+sizeof(sms.billingRecord)+
        sizeof(odMscSize)+odMscSize+sizeof(odImsiSize)+odImsiSize+sizeof(sms.originatingDescriptor.sme)+
        sizeof(ddMscSize)+ddMscSize+sizeof(ddImsiSize)+ddImsiSize+sizeof(sms.destinationDescriptor.sme)+
        sizeof(routeSize)+routeSize+sizeof(sms.serviceId)+sizeof(sms.priority)+
        sizeof(srcSmeSize)+srcSmeSize+sizeof(dstSmeSize)+dstSmeSize+sizeof(bodyBuffLen)+bodyBuffLen;

    uint32_t sendBufferSize = recordSize+sizeof(recordSize)*2;
    TmpBuf<char, 2048> sendBufferGuard(sendBufferSize);
    char* sendBuffer = sendBufferGuard.get(); char* position = sendBuffer;

    recordSize = htonl(recordSize);
    memcpy(position, &recordSize, sizeof(recordSize)); position+=sizeof(recordSize);
    SMSId idd = Uint64Converter::toNetworkOrder(id);
    memcpy(position, &idd, sizeof(idd)); position+=sizeof(idd);
    memcpy(position, &smsState, sizeof(smsState)); position+=sizeof(smsState);
    
    memcpy(position, &submitTimeSz, sizeof(submitTimeSz)); position+=sizeof(submitTimeSz);
    if (submitTimeSz > 0) { memcpy(position, submitTimeBuffer, submitTimeSz); position+=submitTimeSz;}
    memcpy(position, &validTimeSz, sizeof(validTimeSz)); position+=sizeof(validTimeSz);
    if (validTimeSz > 0)  { memcpy(position, validTimeBuffer, validTimeSz); position+=validTimeSz; }
    memcpy(position, &lastTimeSz, sizeof(lastTimeSz)); position+=sizeof(lastTimeSz);
    if (lastTimeSz > 0)   { memcpy(position, lastTimeBuffer, lastTimeSz); position+=lastTimeSz;}
    memcpy(position, &nextTimeSz, sizeof(nextTimeSz)); position+=sizeof(nextTimeSz);
    if (nextTimeSz > 0)   { memcpy(position, nextTimeBuffer, nextTimeSz); position+=nextTimeSz; }
    
    uint32_t attempts = htonl(sms.attempts);
    memcpy(position, &attempts, sizeof(attempts)); position+=sizeof(attempts);
    uint32_t lastResult = htonl(sms.lastResult);
    memcpy(position, &lastResult, sizeof(lastResult)); position+=sizeof(lastResult);
    
    memcpy(position, &oaSize, sizeof(oaSize)); position+=sizeof(oaSize);
    if (oaSize > 0)  { memcpy(position, oa.c_str(), oaSize); position+=oaSize;    }
    memcpy(position, &daSize, sizeof(daSize)); position+=sizeof(daSize);
    if (daSize > 0)  { memcpy(position, da.c_str(), daSize); position+=daSize;    }
    memcpy(position, &ddaSize, sizeof(ddaSize)); position+=sizeof(ddaSize);
    if (ddaSize > 0) { memcpy(position, dda.c_str(), ddaSize); position+=ddaSize; }
    
    uint16_t mr = htons(sms.messageReference);
    memcpy(position, &mr, sizeof(sms.messageReference)); position+=sizeof(sms.messageReference);
    memcpy(position, &svcSize, sizeof(svcSize)); position+=sizeof(svcSize);
    if (svcSize > 0) { memcpy(position, sms.eServiceType, svcSize); position+=svcSize; }
    memcpy(position, &sms.deliveryReport, sizeof(sms.deliveryReport)); position+=sizeof(sms.deliveryReport);
    memcpy(position, &sms.billingRecord, sizeof(sms.billingRecord)); position+=sizeof(sms.billingRecord);

    memcpy(position, &odMscSize, sizeof(odMscSize)); position+=sizeof(odMscSize);
    if (odMscSize > 0)  { memcpy(position, sms.originatingDescriptor.msc, odMscSize); position+=odMscSize; }
    memcpy(position, &odImsiSize, sizeof(odImsiSize)); position+=sizeof(odImsiSize);
    if (odImsiSize > 0) { memcpy(position, sms.originatingDescriptor.imsi, odImsiSize); position+=odImsiSize; }
    uint32_t odSme = htonl(sms.originatingDescriptor.sme);
    memcpy(position, &odSme, sizeof(odSme)); position+=sizeof(odSme);
    memcpy(position, &ddMscSize, sizeof(ddMscSize)); position+=sizeof(ddMscSize);
    if (ddMscSize > 0)  { memcpy(position, sms.destinationDescriptor.msc, ddMscSize); position+=ddMscSize; }
    memcpy(position, &ddImsiSize, sizeof(ddImsiSize)); position+=sizeof(ddImsiSize);
    if (ddImsiSize > 0) { memcpy(position, sms.destinationDescriptor.imsi, ddImsiSize); position+=ddImsiSize; }
    uint32_t ddSme = htonl(sms.destinationDescriptor.sme);
    memcpy(position, &ddSme, sizeof(ddSme)); position+=sizeof(ddSme);
    
    memcpy(position, &routeSize, sizeof(routeSize)); position+=sizeof(routeSize);
    if (routeSize > 0)  { memcpy(position, sms.routeId, routeSize); position+=routeSize; } 
    int32_t svcId = (int32_t)htonl(sms.serviceId);
    memcpy(position, &svcId, sizeof(svcId)); position+=sizeof(svcId);
    int32_t priority = (int32_t)htonl(sms.priority);
    memcpy(position, &priority, sizeof(sms.priority)); position+=sizeof(sms.priority);
    memcpy(position, &srcSmeSize, sizeof(srcSmeSize)); position+=sizeof(srcSmeSize);
    if (srcSmeSize > 0) { memcpy(position, sms.srcSmeId, srcSmeSize); position+=srcSmeSize; }
    memcpy(position, &dstSmeSize, sizeof(dstSmeSize)); position+=sizeof(dstSmeSize);
    if (dstSmeSize > 0) { memcpy(position, sms.dstSmeId, dstSmeSize); position+=dstSmeSize; }

    int32_t textLen = (int32_t)htonl(bodyBuffLen);
    memcpy(position, &textLen, sizeof(bodyBuffLen)); position+=sizeof(bodyBuffLen);
    if (bodyBuffLen > 0) {
        uint8_t* bodyBuffer = sms.messageBody.getBuffer();
        memcpy(position, bodyBuffer, bodyBuffLen); position+=bodyBuffLen;
    }
    
    memcpy(position, &recordSize, sizeof(recordSize)); position+=sizeof(recordSize);

    Message::write(socket, (void *)sendBuffer, sendBufferSize);
}

void RsSmsMessage::receive(Socket* socket)
{
    // TODO: implement if needed
}

void DaemonCommunicator::send(Message* message)
{
    if (!message) throw CommunicationException("Message to send is NULL!");
    message->send(socket);
}
Message* DaemonCommunicator::receive()
{
    uint8_t type = 0;
    Message::read(socket, &type, sizeof(type));

    Message* message = Message::create(type);
    if (!message) throw CommunicationException("Received message is NULL!");
    message->receive(socket);
    return message;
}

}}


