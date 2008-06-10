#ifndef __SMSC_INFOSME_TASKTYPES_HPP__
#define __SMSC_INFOSME_TASKTYPES_HPP__

#include <string>
#include "util/int.h"
#include "DateTime.h"

namespace smsc{
namespace infosme{

  static const uint8_t MESSAGE_NEW_STATE          = 0; // Новое или перешедуленное сообщение
  static const uint8_t MESSAGE_WAIT_STATE         = 1; // Ожидает submitResponce
  static const uint8_t MESSAGE_ENROUTE_STATE      = 2; // В процессе доставки, ожидает deliveryReciept
  static const uint8_t MESSAGE_DELIVERED_STATE    = 3; // Доставлено
  static const uint8_t MESSAGE_EXPIRED_STATE      = 4; // Не доставлено, время жизни истекло
  static const uint8_t MESSAGE_FAILED_STATE       = 5; // Не доставлено, фатальная ошибка при доставке
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

      bool    retryOnFail, replaceIfPresent, trackIntegrity, transactionMode, keepHistory;
      bool    flash;

      time_t  endDate;            // full date/time
      time_t  retryTime;          // only HH:mm:ss in seconds
      time_t  validityPeriod;     // only HH:mm:ss in seconds
      time_t  validityDate;       // full date/time
      time_t  activePeriodStart;  // only HH:mm:ss in seconds
      time_t  activePeriodEnd;    // only HH:mm:ss in seconds

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
            trackIntegrity(false), transactionMode(false), keepHistory(false),flash(false),
            endDate(-1), retryTime(-1), validityPeriod(-1), validityDate(-1),
            activePeriodStart(-1), activePeriodEnd(-1), activeWeekDays(0),
            dsTimeout(0), dsUncommitedInProcess(1), dsUncommitedInGeneration(1),
            messagesCacheSize(100), messagesCacheSleep(0) {};
  };


}
}

#endif

