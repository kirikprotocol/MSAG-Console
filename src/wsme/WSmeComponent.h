#ifndef SMSC_WSME_COMPONENT
#define SMSC_WSME_COMPONENT

#include <log4cpp/Category.hh>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>

#include "WSmeAdmin.h"

namespace smsc { namespace wsme
{
    using namespace smsc::admin::service;
    
    class WSmeComponent : public Component
    {
    private:
    
        log4cpp::Category  &log;

        WSmeAdmin          &admin;
        Methods             methods;
        
        enum {
            addVisitorMethod, removeVisitorMethod,
            addLangMethod, removeLangMethod,
            addAdMethod, removeAdMethod
        };
        
        void error(const char* method, const char* param)
            throw (ProcessException);
    
    protected:
        
        void addVisitor(const Arguments& args)
            throw (ProcessException);
        void removeVisitor(const Arguments& args)
            throw (ProcessException);           

        void addLang(const Arguments& args)
            throw (ProcessException);
        void removeLang(const Arguments& args)
            throw (ProcessException);
       
        void addAd(const Arguments& args)
            throw (ProcessException);
        void removeAd(const Arguments& args)
            throw (ProcessException);
        
    public:
    
        WSmeComponent(WSmeAdmin& _admin);
        virtual ~WSmeComponent();
        
        virtual const char* const getName() const {
            return "WSme";
        }
        virtual const Methods& getMethods() const {
            return methods;
        }
        
        virtual Variant call(const Method& method, const Arguments& args)
            throw (AdminException);
    };

}
}
#endif // ifndef SMSC_WSME_COMPONENT
