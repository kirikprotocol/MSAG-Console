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

    struct StatInfo
    {
        int errcode;       // 0 - if status ok
        std::string smeId; // either src or dst smeId
        std::string routeId;
        signed long providerId, categoryId; // -1 if undefined

        StatInfo(const std::string& _smeId, const std::string& _routeId, int _errcode = 0,
                 signed long _providerId = -1, signed long _categoryId = -1)
            : smeId(_smeId), routeId(_routeId), errcode(_errcode), 
              providerId(_providerId), categoryId(_categoryId) {};
    };

    class Statistics
    {
    public:

        virtual void flushStatistics() = 0;

        virtual void updateAccepted (const StatInfo& info) = 0;
        virtual void updateRejected (const StatInfo& info) = 0;
        virtual void updateTemporal (const StatInfo& info) = 0;
        virtual void updateChanged  (const StatInfo& info) = 0;
        virtual void updateScheduled(const StatInfo& info) = 0;
        
        virtual ~Statistics() {};
        
    protected:
        
        Statistics() {};
    };

}}

#endif // SMSC_STAT_STATISTICS

