
#ifndef SMSC_MSCMAN_MSC_STATUS
#define SMSC_MSCMAN_MSC_STATUS

#include "core/buffers/File.hpp"

namespace smsc { namespace mscman
{
    class MscStatus
    {
    protected:

        MscStatus() {};

    public:

        virtual ~MscStatus() {};

        virtual void report(const char* msc, bool status,smsc::core::buffers::File::offset_type offset=0) = 0;
        virtual bool check(const char* msc) = 0;
    };
}}

#endif //SMSC_MSCMAN_MSC_STATUS
