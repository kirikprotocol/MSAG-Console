#ifndef SMSC_MCI_SME_ADMIN
#define SMSC_MCI_SME_ADMIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

#include "Statistics.h"

namespace smsc { namespace mcisme
{
    struct RuntimeStat
    {
        int activeTasks, inQueueSize, outQueueSize, inSpeed, outSpeed;
        
        RuntimeStat(int at=0, int inQS=0, int outQS=0, int inS=0, int outS=0)
            : activeTasks(at), inQueueSize(inQS), outQueueSize(outQS), inSpeed(inS), outSpeed(outS) {};
        RuntimeStat(const RuntimeStat& stat)
            : activeTasks(stat.activeTasks), inQueueSize(stat.inQueueSize), outQueueSize(stat.outQueueSize),
              inSpeed(stat.inSpeed), outSpeed(stat.outSpeed) {};
        RuntimeStat& operator=(const RuntimeStat& stat) {
            activeTasks = stat.activeTasks;
            inQueueSize = stat.inQueueSize; outQueueSize = stat.outQueueSize;
            inSpeed     = stat.inSpeed;     outSpeed     = stat.outSpeed;
            return (*this);
        };
    };

    struct AdminInterface
    {
        virtual void flushStatistics() = 0;
        virtual EventsStat getStatistics() = 0;

        virtual int getActiveTasksCount() = 0;
        virtual int getInQueueSize()  = 0;
        virtual int getOutQueueSize() = 0;
        
        AdminInterface() {};

    protected:

        virtual ~AdminInterface() {};
    };

    struct MCISmeAdmin
    {
        virtual void flushStatistics() = 0;
        
        virtual EventsStat  getStatistics() = 0;
        virtual RuntimeStat getRuntimeStatistics() = 0;
        
        MCISmeAdmin() {};

    protected:

        virtual ~MCISmeAdmin() {};
    };
        
}}

#endif // SMSC_MCI_SME_ADMIN


