#ifndef SMSC_MCI_SME_STATISTICS
#define SMSC_MCI_SME_STATISTICS

#include <string>

namespace smsc { namespace mcisme 
{
    struct EventsStat
    {
        unsigned missed, delivered, failed, notified;
        
        EventsStat(unsigned missed=0, unsigned delivered=0, 
                   unsigned failed=0, unsigned notified=0) 
            : missed(missed), delivered(delivered), 
              failed(failed), notified (notified) {};
        EventsStat(const EventsStat& stat) 
            : missed(stat.missed), delivered(stat.delivered), 
              failed(stat.failed), notified (stat.notified) {};
        
        EventsStat& operator =(const EventsStat& stat) {
            missed = stat.missed; delivered = stat.delivered; 
            failed = stat.failed; notified  = stat.notified;
            return (*this);
        };

        inline bool isEmpty() {
            return (missed<=0 && delivered<=0 && failed<=0 && notified<=0);
        };
        inline void Empty() {
            missed = 0; delivered = 0; failed = 0; notified = 0;
        };
    };

    class Statistics
    {
    public:

        virtual void flushStatistics() = 0;
        virtual EventsStat getStatistics() = 0;

        virtual void incMissed   (const char* abonent) = 0;
        virtual void incDelivered(const char* abonent) = 0;
        virtual void incFailed   (const char* abonent) = 0;
        virtual void incNotified (const char* abonent) = 0;
        
        virtual ~Statistics() {};
        
    protected:
        
        Statistics() {};
    };

}}

#endif // SMSC_MCI_SME_STATISTICS

