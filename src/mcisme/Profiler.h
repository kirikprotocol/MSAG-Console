#ifndef SMSC_MCI_SME_PROFILER
#define SMSC_MCI_SME_PROFILER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

#include <logger/Logger.h>
#include <db/DataSource.h>

namespace smsc { namespace mcisme
{

    using namespace smsc::db;
    using smsc::logger::Logger;

    struct AbonentProfile
    {
        uint8_t eventMask;
        bool    inform, notify;
        int32_t informTemplateId, notifyTemplateId; // if -1 => default
        
        AbonentProfile() 
            : eventMask(0xFF), inform(true), notify(false), informTemplateId(-1), notifyTemplateId(-1) {};
        AbonentProfile(const AbonentProfile& pro) 
            : eventMask(pro.eventMask), inform(pro.inform), notify(pro.notify), 
              informTemplateId(pro.informTemplateId), notifyTemplateId(pro.notifyTemplateId) {};
        AbonentProfile& operator=(const AbonentProfile& pro) {
            eventMask = pro.eventMask;
            inform = pro.inform; notify = pro.notify;
            informTemplateId = pro.informTemplateId; notifyTemplateId = pro.notifyTemplateId;
            return (*this);
        };
    };
    
    class AbonentProfiler
    {
    private: 
        
        static DataSource*  ds;
        static Logger*  logger;
    
    public:

        static void init(DataSource* _ds);

        static bool delProfile(const char* abonent, Connection* connection=0);
        static void setProfile(const char* abonent, const AbonentProfile& profile, Connection* connection=0);
        static AbonentProfile getProfile(const char* abonent, Connection* connection=0);
    };
}}

#endif // SMSC_MCI_SME_PROFILER
