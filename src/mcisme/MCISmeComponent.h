#ifndef SMSC_MCI_SME_COMPONENT
#define SMSC_MCI_SME_COMPONENT

#include <logger/Logger.h>
#include <util/Exception.hpp>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>

#include "MCISmeAdmin.h"

namespace smsc { namespace mcisme
{
    using namespace smsc::admin::service;
    
    using smsc::logger::Logger;
    using smsc::util::Exception;
    
    class MCISmeComponent : public Component
    {
    private:
    
        smsc::logger::Logger *logger;

        MCISmeAdmin       &admin;
        Methods            methods;
        
        enum { flushStatisticsMethod, getStatisticsMethod, getRuntimeMethod };
        
    protected:
        
        void error(const char* method, const char* param);

        Variant getStatistics();
        Variant getRuntime();
    
    public:
        
        MCISmeComponent(MCISmeAdmin& admin);
        virtual ~MCISmeComponent() {};
        
        virtual const char* const getName() const {
            return "MCISme";
        }
        virtual const Methods& getMethods() const {
            return methods;
        }
        
        virtual Variant call(const Method& method, const Arguments& args)
            throw (AdminException);
    };

}}

#endif // ifndef SMSC_MCI_SME_COMPONENT
