#ifndef SMSC_MCI_SME_ADMIN
#define SMSC_MCI_SME_ADMIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

#include "Statistics.h"

namespace smsc {
namespace mcisme {

struct RuntimeStat
{
  unsigned activeTasks, inQueueSize, outQueueSize, inSpeed, outSpeed;
        
  RuntimeStat(unsigned at=0, unsigned inQS=0, unsigned outQS=0, unsigned inS=0, unsigned outS=0)
    : activeTasks(at), inQueueSize(inQS), outQueueSize(outQS), inSpeed(inS), outSpeed(outS) {};
};

struct AdminInterface
{
  virtual void flushStatistics() = 0;
  virtual EventsStat getStatistics() const = 0;

  virtual unsigned getActiveTasksCount() const = 0;
  virtual unsigned getInQueueSize() const = 0;
  virtual unsigned getOutQueueSize() const = 0;
  virtual std::string getSchedItem(const std::string& Abonent) = 0;
  virtual std::string getSchedItems(void) = 0;

  AdminInterface() {};

protected:

  virtual ~AdminInterface() {};
};

struct MCISmeAdmin
{
  virtual void flushStatistics() = 0;
        
  virtual EventsStat  getStatistics() = 0;
  virtual RuntimeStat getRuntimeStatistics() = 0;
  virtual std::string getSchedItem(const std::string& Abonent) = 0;
  virtual std::string getSchedItems(void) = 0;

  MCISmeAdmin() {};

protected:

  virtual ~MCISmeAdmin() {};
};
        
}}

#endif // SMSC_MCI_SME_ADMIN


