#ifndef SMSC_STAT_STATISTICS
#define SMSC_STAT_STATISTICS

#include <util/Exception.hpp>

namespace smsc { namespace stat 
{
    using smsc::util::Exception;

    class StatisticsException : public Exception
    {
    public:
        
        StatisticsException() 
            : Exception("Statistics collection failed !") {};
        StatisticsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        StatisticsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~StatisticsException() throw() {};
    };

    class Statistics
    {
    public:

        virtual void flushStatistics() = 0;

        virtual void updateScheduled() = 0;
        virtual void updateAccepted(const char* srcSmeId) = 0;
        virtual void updateRejected(int errcode) = 0;
        virtual void updateChanged(const char* dstSmeId, 
            const char* routeId, int errcode = 0) = 0;
        
        virtual ~Statistics() {};
        
    protected:
        
        Statistics() {};
    };

}}

#endif // SMSC_STAT_STATISTICS

