#ifndef SMSC_MCI_SME_ADMIN
#define SMSC_MCI_SME_ADMIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

namespace smsc { namespace mcisme
{
    struct MCISmeAdmin
    {
        virtual void flushStatistics() = 0;
        
        virtual int getInQueueSize() = 0;
        virtual int getOutQueueSize() = 0;
        
        MCISmeAdmin() {};

    protected:

        virtual ~MCISmeAdmin() {};
    };
        
}}

#endif // SMSC_MCI_SME_ADMIN


