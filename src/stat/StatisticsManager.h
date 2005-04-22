#ifndef SMSC_STAT_STATISTICS_MANAGER
#define SMSC_STAT_STATISTICS_MANAGER

#include "Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include <core/buffers/Hash.hpp>
#include <core/threads/ThreadedTask.hpp>

#include <logger/Logger.h>

#include <util/timeslotcounter.hpp>

namespace smsc { namespace stat 
{
    using namespace core::threads;
    using namespace core::synchronization;
    
    using core::buffers::IntHash;
    using core::buffers::TmpBuf;
    using core::buffers::Hash;
    
    using smsc::logger::Logger;
    using smsc::util::TimeSlotCounter;
    
    struct SmsStat
    {
        int accepted, rejected;                       
        int delivered, failed, rescheduled, temporal;
        int peak_i, peak_o;
        IntHash<int> errors;
        
        TimeSlotCounter<int>* i_counter;
        TimeSlotCounter<int>* o_counter;
        
        SmsStat(int accepted = 0, int rejected = 0, 
                int delivered = 0, int failed = 0, 
                int rescheduled = 0, int temporal = 0) 
            : accepted(accepted), rejected(rejected),
              delivered(delivered), failed(failed),
              rescheduled(rescheduled), temporal(temporal),
              peak_i(0), peak_o(0), i_counter(0), o_counter(0)
        {
            initCounters();
        };
        SmsStat(const SmsStat& stat) 
            : accepted(stat.accepted), rejected(stat.rejected), 
              delivered(stat.delivered), failed(stat.failed),
              rescheduled(stat.rescheduled), temporal(stat.temporal),
              peak_i(stat.peak_i), peak_o(stat.peak_o), 
              errors(stat.errors), i_counter(0), o_counter(0)
        {
            initCounters();
        };
        virtual ~SmsStat() {
            deleteCounters();
        }
        
        SmsStat& operator =(const SmsStat& stat) {
            accepted = stat.accepted; rejected = stat.rejected;
            delivered = stat.delivered; failed = stat.failed;
            rescheduled = stat.rescheduled; temporal = stat.temporal;
            peak_i = stat.peak_i; peak_o = stat.peak_o;
            errors = stat.errors;
            reinitCounters();
            return (*this);
        };

        inline void Empty() {
            accepted = 0; rejected = 0; delivered = 0; 
            failed = 0; rescheduled = 0; temporal = 0;
            peak_i = 0; peak_o = 0;
            errors.Empty();
            reinitCounters();
        };

        inline void incICounter()
        {
            if (!i_counter) return;
            i_counter->Inc();
            int count = i_counter->Get();
            if (count > peak_i) peak_i = count;
        }
        inline void incOCounter()
        {
            if (!o_counter) return;
            o_counter->Inc();
            int count = o_counter->Get();
            if (count > peak_o) peak_o = count;
        }
    
    protected:

        inline void deleteCounters() {
            if (i_counter) delete i_counter; i_counter = 0;
            if (o_counter) delete o_counter; o_counter = 0;
        };
        inline void initCounters() {
            i_counter = new TimeSlotCounter<int>(1, 10);
            o_counter = new TimeSlotCounter<int>(1, 10);
        }
        inline void reinitCounters() {
            deleteCounters();
            initCounters();
        };
        
    };

    struct RouteStat : public SmsStat
    {
        signed long providerId, categoryId;

        RouteStat(int accepted = 0, int rejected = 0, 
                  int delivered = 0, int failed = 0, 
                  int rescheduled = 0, int temporal = 0,
                  signed long _providerId = -1, signed long _categoryId = -1) 
            : SmsStat(accepted, rejected, delivered, rescheduled, temporal), 
              providerId(_providerId), categoryId(_categoryId) {};

        RouteStat(const RouteStat& stat) 
            : SmsStat(stat), providerId(stat.providerId), categoryId(stat.categoryId) {};

        virtual ~RouteStat() {};
    };

    class StatStorage
    {
    private:
    
        smsc::logger::Logger    *logger;

        std::string     location;
        bool            bFileTM;
        tm              fileTM;
        FILE*           file;
    
        void close();
        void flush();
        void write(const void* data, size_t size);

        static bool createDir(const std::string& dir);
        bool createStatDir();
    
    public:
    
        StatStorage(const std::string& location);
        ~StatStorage();
        
        void dump(const uint8_t* buff, int buffLen, const tm& flushTM);
    };
    
    class StatisticsManager : public Statistics, public ThreadedTask
    {
    protected:
    
		smsc::logger::Logger    *logger;
        
        SmsStat         statGeneral[2];
        Hash<SmsStat>   statBySmeId[2];
        Hash<RouteStat> statByRoute[2];
        
        short   currentIndex;
        bool    bExternalFlush;
        
        Mutex   stopLock, switchLock, flushLock;
        Event   awakeEvent, exitEvent, doneEvent;
        bool    isStarted;

        StatStorage storage;
        
        short switchCounters();
        void  resetCounters(short index);
        void  flushCounters(short index);
        void  addError(IntHash<int>& hash, int errcode);

        void  calculateTime(tm& flushTM);
        int   calculateToSleep(); // returns msecs to next minute

    public:
        
        virtual const char* taskName() { return "StatisticsTask"; };
        virtual int Execute();
        virtual void stop();

        virtual void flushStatistics();

        virtual void updateAccepted (const StatInfo& info);
        virtual void updateRejected (const StatInfo& info);
        virtual void updateTemporal (const StatInfo& info);
        virtual void updateChanged  (const StatInfo& info);
        virtual void updateScheduled(const StatInfo& info);
        
        StatisticsManager(const std::string& location);
        virtual ~StatisticsManager();
    };

}}

#endif //SMSC_STAT_STATISTICS_MANAGER
