#ifndef __SMSC_INFOSME_TASKTYPES_HPP__
#define __SMSC_INFOSME_TASKTYPES_HPP__

#include <string>
#include "util/int.h"
#include "DateTime.h"
#include "system/status.h"
#include "core/buffers/File.hpp"
#include "core/buffers/Hash.hpp"
#include "sms/sms.h"

namespace smsc{
namespace infosme{

  static const uint8_t MESSAGE_NEW_STATE          = 0; //    
  static const uint8_t MESSAGE_WAIT_STATE         = 1; //  submitResponce
  static const uint8_t MESSAGE_ENROUTE_STATE      = 2; //   ,  deliveryReciept
  static const uint8_t MESSAGE_DELIVERED_STATE    = 3; // 
  static const uint8_t MESSAGE_EXPIRED_STATE      = 4; //  ,   
  static const uint8_t MESSAGE_FAILED_STATE       = 5; //  ,    
  static const uint8_t MESSAGE_DELETED_STATE      = 6; // for csv store only. message was created in another file.

  typedef enum {
      NEW         = MESSAGE_NEW_STATE,
      WAIT        = MESSAGE_WAIT_STATE,
      ENROUTE     = MESSAGE_ENROUTE_STATE,
      DELIVERED   = MESSAGE_DELIVERED_STATE,
      EXPIRED     = MESSAGE_EXPIRED_STATE,
      FAILED      = MESSAGE_FAILED_STATE,
      DELETED     = MESSAGE_DELETED_STATE
  } MessageState;

  struct Message
  {
      uint64_t    id;
      time_t date;
      std::string abonent;
      std::string message;
      std::string regionId;
      std::string userData;  // optional user data

      Message():id(0),date(0)
      {
      }

      //Message(uint64_t anId=0, std::string anAbonent="", std::string aMessage="", std::string aRegionId="")
      //  : id(anId), date(0),abonent(anAbonent), message(aMessage), regionId(aRegionId) {}
  };

  struct TaskInfo
  {
      uint32_t    uid;
      std::string name;
      bool        enabled;
      int         priority;

      bool    retryOnFail, replaceIfPresent, trackIntegrity, transactionMode, keepHistory, saveFinalState;
      bool    flash;
      bool     useDataSm;
      unsigned useUssdPush;       // 0 if not used, otherwise the id of ussdop

      time_t  endDate;            // full date/time
      //time_t  retryTime;          // only HH:mm:ss in seconds
      time_t  validityPeriod;     // only HH:mm:ss in seconds
      time_t  validityDate;       // full date/time
      time_t  activePeriodStart;  // only HH:mm:ss in seconds
      time_t  activePeriodEnd;    // only HH:mm:ss in seconds

      std::string retryPolicy;

      WeekDaysSet activeWeekDays; // Mon, Tue ...

      std::string tablePrefix;
      std::string querySql;
      std::string msgTemplate;
      std::string svcType;        // specified if replaceIfPresent == true
      std::string address;

      int     dsTimeout, dsUncommitedInProcess, dsUncommitedInGeneration;
      int     messagesCacheSize, messagesCacheSleep;

      TaskInfo()
          : uid(0), enabled(true), priority(0),
            retryOnFail(false), replaceIfPresent(false),
            trackIntegrity(false), transactionMode(false), keepHistory(false),
            saveFinalState(false),
            flash(false),
            useDataSm(false), useUssdPush(0),
            endDate(-1), validityPeriod(-1), validityDate(-1),
            activePeriodStart(-1), activePeriodEnd(-1), activeWeekDays(0),
            dsTimeout(0), dsUncommitedInProcess(1), dsUncommitedInGeneration(1),
            messagesCacheSize(100), messagesCacheSleep(0) {}

      bool convertMSISDNStringToAddress(const char* string, smsc::sms::Address& addr) const
      {
          try {
              const smsc::sms::Address converted(string);
              addr = converted;
          } catch (...) {
              return false;
          }
          return true;
      }
  };


struct TaskMsgId
{
    uint32_t taskId;
    uint64_t msgId;

    uint32_t getTaskId() const
    {
        return taskId;
    }
    
    TaskMsgId(uint32_t taskId=0, uint64_t msgId=0)
        : taskId(taskId), msgId(msgId) {}
};


struct ReceiptId 
{
    static const uint8_t SMSC_MSGID_MAX_SIZE = 65; //with 0

    ReceiptId() {
      memset(smscMessageId, 0, SMSC_MSGID_MAX_SIZE);
    };

    ReceiptId(const std::string& messageId) {
      setMessageId(messageId);
    };

    ReceiptId(const ReceiptId& receiptId) {
      setMessageId(receiptId.getMessageId());
    }
    ReceiptId& operator=(const ReceiptId& receiptId) {
      if ( &receiptId != this ) {
          setMessageId(receiptId.getMessageId());
      }
      return *this;
    }

    bool operator==(const ReceiptId& receiptId) {
      return (strcmp(smscMessageId, receiptId.getMessageId()) == 0); 
    }
    bool operator!=(const ReceiptId& receiptId) {
      return !(*this == receiptId);
    }

    void setMessageId(const std::string& messageId) {
      memset(smscMessageId, 0, SMSC_MSGID_MAX_SIZE);
      size_t size = messageId.size();
      memcpy(smscMessageId, messageId.c_str(), size < SMSC_MSGID_MAX_SIZE ? size : SMSC_MSGID_MAX_SIZE - 1);
    }

    const char* getMessageId() const {
      return smscMessageId;
    }

    static uint32_t CalcHash(const ReceiptId& receiptId) {
      return smsc::core::buffers::HashFunc(receiptId.getMessageId());  
    }

    int Read(smsc::core::buffers::File& f) {
      char buf[SMSC_MSGID_MAX_SIZE];
      f.Read(buf, SMSC_MSGID_MAX_SIZE);
      memset(smscMessageId, 0, SMSC_MSGID_MAX_SIZE);
      memcpy(smscMessageId, buf, SMSC_MSGID_MAX_SIZE);
      return SMSC_MSGID_MAX_SIZE;
    }

    int Write(smsc::core::buffers::File& f) const {
      char buf[SMSC_MSGID_MAX_SIZE];
      memset(buf, 0, SMSC_MSGID_MAX_SIZE);
      memcpy(buf, smscMessageId, SMSC_MSGID_MAX_SIZE);
      f.Write(buf, SMSC_MSGID_MAX_SIZE);
      return SMSC_MSGID_MAX_SIZE;
    }

  private:
    void setMessageId(const char* messageId) {
      memcpy(smscMessageId, messageId, SMSC_MSGID_MAX_SIZE);
    }

  private:
    char smscMessageId[SMSC_MSGID_MAX_SIZE];
};


struct ResponseData
{
    std::string msgId;
    int status;
    int seqNum;
    bool accepted, retry, immediate, trafficst;

    ResponseData( int argStatus,
                  int argSeqNum,
                  const std::string& argMsgId ) :
    msgId(argMsgId), status(argStatus), seqNum(argSeqNum)
    {
      using namespace smsc::system;
      accepted = status==Status::OK;
      retry     = !accepted && (!Status::isErrorPermanent(status));

      immediate = (status == Status::MSGQFUL   ||
                        status == Status::THROTTLED ||
                        status == Status::LICENSELIMITREJECT
                        //commented for Lugovoj's request
                        //|| status == Status::SUBSCRBUSYMT
                        );

      trafficst = (status == Status::MSGQFUL                   ||
                        status == Status::THROTTLED                 ||
                        status == Status::LICENSELIMITREJECT        ||
                        status == Status::MAP_RESOURCE_LIMITATION   ||
                        status == Status::MAP_NO_RESPONSE_FROM_PEER ||
                        status == Status::SMENOTCONNECTED           ||
                        status == Status::SYSFAILURE);
    }
};

}
}

#endif

