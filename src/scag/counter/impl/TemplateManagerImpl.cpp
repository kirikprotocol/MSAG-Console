#include "TemplateManagerImpl.h"
// #include "scag/counter/Accumulator.h"
// #include "scag/counter/Average.h"
// #include "scag/counter/TimeSnapshot.h"
#include "scag/counter/ActionTable.h"

namespace scag2 {
namespace counter {
namespace impl {

TemplateManagerImpl::TemplateManagerImpl() :
log_(smsc::logger::Logger::getInstance("cnt.temgr"))
{
    smsc_log_debug(log_,"ctor");
}


TemplateManagerImpl::~TemplateManagerImpl()
{
    // templates
    smsc_log_debug(log_,"dtor, cleaning templates and groups");
    char* key;
    CounterTemplate* val;
    for ( smsc::core::buffers::Hash< CounterTemplate* >::Iterator i(&templates_);
          i.Next(key,val); ) {
        if (val) delete val;
    }
    Observer* tbl;
    for ( smsc::core::buffers::Hash< Observer* >::Iterator i(&actionTables_);
          i.Next(key,tbl); ) {
        if (tbl) tbl->ref(false);
    }
}


Counter* TemplateManagerImpl::createCounter( const char* templid,
                                             const std::string& name,
                                             unsigned seconds )
{
    if (!templid) return 0;
    smsc_log_debug(log_,"asking to create a counter templid='%s' name='%s'",templid,name.c_str());
    MutexGuard mg(lock_);
    CounterTemplate** ptr = templates_.GetPtr(templid);
    if (!ptr || !*ptr) {
        smsc_log_error(log_,"counter template '%s' is not found",templid);
        return 0;
    }
    Counter* c = (*ptr)->getPrototype();
    if (!c) {
        smsc_log_error(log_,"counter prototype '%s' is not found",templid);
        return 0;
    }
    return c->clone(name,seconds);
}


void TemplateManagerImpl::replaceTemplate( const char*      name,
                                           CounterTemplate* tmpl )
{
    if (!name) return;
    smsc_log_debug(log_,"asking to replace/add templid='%s' with %p",name,tmpl);
    MutexGuard mg(lock_);
    CounterTemplate** ptr = templates_.GetPtr(name);
    if (ptr) {
        if (*ptr == tmpl) return;
        if (*ptr) delete *ptr;
        *ptr = tmpl;
    } else if (tmpl) {
        templates_.Insert(name,tmpl);
    }
}


std::vector< std::string > TemplateManagerImpl::getTemplateNames()
{
    std::vector< std::string > res;
    res.reserve( templates_.GetCount() );
    {
        MutexGuard mg(lock_);
        char* p;
        CounterTemplate* t;
        for ( smsc::core::buffers::Hash< CounterTemplate* >::Iterator i(&templates_);
              i.Next(p,t); ) {
            res.push_back(p);
        }
    }
    return res;
}


ObserverPtr TemplateManagerImpl::getObserver( const char* name )
{
    if (!name) return ObserverPtr();
    smsc_log_debug(log_,"asking to fetch observer='%s'",name);
    MutexGuard mg(lock_);
    Observer** ptr = actionTables_.GetPtr(name);
    if (!ptr) return ObserverPtr();
    return ObserverPtr(*ptr);
}


void TemplateManagerImpl::replaceObserver( const char* name,
                                           Observer* table )
{
    if (!name) return;
    smsc_log_debug(log_,"asking to replace/add observer='%s' with %p",name,table);
    MutexGuard mg(lock_);
    Observer** ptr = actionTables_.GetPtr(name);
    if (ptr) {
        if (*ptr == table) return;
        if (*ptr) (*ptr)->ref(false); // dtor
        *ptr = table;
        if (table) table->ref(true);
    } else if (table) {
        actionTables_.Insert(name,table);
        table->ref(true);
    }
}


std::vector< std::string > TemplateManagerImpl::getObserverNames()
{
    std::vector< std::string > res;
    res.reserve( actionTables_.GetCount() );
    {
        MutexGuard mg(lock_);
        char* p;
        Observer* t;
        for ( smsc::core::buffers::Hash< Observer* >::Iterator i(&actionTables_);
              i.Next(p,t); ) {
            res.push_back(p);
        }
    }
    return res;
}


void TemplateManagerImpl::init()
{
    struct Limit {
        const char* name;
        unsigned    maxval;
    };
    Limit limits[] = {
        { "sys.traffic.global.smpp", 100 },
        { "sys.traffic.smpp.sme", 100 },
        { "sys.traffic.smpp.smsc", 100 },
        { "sys.smpp.queue.global", 100 },
        { "sys.smpp.queue.in", 100 },
        { "sys.smpp.queue.out", 100 },
        { "sys.sessions.total", 1000 },
        { "sys.sessions.active", 1000 },
        { "sys.sessions.locked", 1000 },
        { 0, 0 }
    };
    for ( Limit* p = limits; p->name != 0; ++p ) {
        ActionList* l = new ActionList();
        l->push_back(ActionLimit(95*p->maxval/100,OPTYPEGE,SEVCRITICAL));
        l->push_back(ActionLimit(90*p->maxval/100,OPTYPEGE,SEVMAJOR));
        l->push_back(ActionLimit(80*p->maxval/100,OPTYPEGE,SEVMINOR));
        l->push_back(ActionLimit(70*p->maxval/100,OPTYPEGE,SEVWARNING));
        replaceObserver( p->name, new ActionTable(l) );
    }
}

}
}
}
