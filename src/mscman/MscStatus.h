
#ifndef SMSC_MSCMAN_MSC_STATUS
#define SMSC_MSCMAN_MSC_STATUS

namespace smsc { namespace mscman 
{
    class MscStatus
    {
    protected:
        
        MscStatus() {};

    public:

        virtual ~MscStatus() {};

        virtual void report(const char* msc, bool status) = 0;
        virtual bool check(const char* msc) = 0;
    };
}}

#endif //SMSC_MSCMAN_MSC_STATUS

