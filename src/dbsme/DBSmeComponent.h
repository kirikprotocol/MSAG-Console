#ifndef SMSC_DBSME_COMPONENT
#define SMSC_DBSME_COMPONENT

#include <logger/Logger.h>
#include <util/Exception.hpp>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>

#include "DBSmeAdmin.h"

namespace smsc { namespace dbsme
{
    using namespace smsc::admin::service;
    
    using smsc::util::Logger;
    using smsc::util::Exception;
    
    class DBSmeComponent : public Component
    {
    private:
    
        log4cpp::Category  &logger;

        DBSmeAdmin         &admin;
        Methods            methods;
        
        enum
        {
            applyChangesMethod
        };
        
        void error(const char* method, const char* param);
    
    protected:

        //void applyChanges(const Arguments& args);
        
    public:
    
        DBSmeComponent(DBSmeAdmin& admin);
        virtual ~DBSmeComponent();
        
        virtual const char* const getName() const {
            return "DBSme";
        }
        virtual const Methods& getMethods() const {
            return methods;
        }
        
        virtual Variant call(const Method& method, const Arguments& args)
            throw (AdminException);
    };

}}

#endif // ifndef SMSC_DBSME_COMPONENT
