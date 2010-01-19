#include "ActionCounter.h"
#include "scag/counter/Counter.h"
#include "scag/counter/Manager.h"

namespace {
smsc::core::synchronization::Mutex logMutex;
}

namespace scag2 {
namespace re {
namespace actions {

smsc::logger::Logger* ActionCounterBase::log_ = 0;

ActionCounterBase::ActionCounterBase() : Action(),
name_(*this,"name",true,true),
scope_(ScopeType(-1)),
status_(*this,"status",false,false)
{
    if (!log_) {
        MutexGuard mg(logMutex);
        if (!log_) log_ = smsc::logger::Logger::getInstance("re.counter");
    }
}


void ActionCounterBase::init( const SectionParams& params,
                              PropertyObject propertyObject )
{
    name_.init(params,propertyObject);
    status_.init(params,propertyObject);
    StringField scope(*this,"scope",true,true);
    scope.init(params,propertyObject);
    if ( scope.getType() == ftUnknown ) {
        if ( strcmp(scope.getStringValue(),"USER") ) {
            scope_ = USER;
        } else if ( strcmp(scope.getStringValue(),"SERVICE") ) {
            scope_ = SERVICE;
        } else if ( strcmp(scope.getStringValue(),"OPERATOR") ) {
            scope_ = OPERATOR;
        } else if ( strcmp(scope.getStringValue(),"PROVIDER") ) {
            scope_ = PROVIDER;
        } else if ( strcmp(scope.getStringValue(),"SYSTEM") ) {
            scope_ = SYSTEM;
        }
        throw SCAGException("action '%s': unknown scope '%s'",
                            opname(),scope.getStringValue());
    } else {
        throw SCAGException("action '%s': only constant scopes allowed",opname());
    }
}


IParserHandler* ActionCounterBase::StartXMLSubSection( const std::string& name,
                                                       const SectionParams& params,
                                                       const ActionFactory& factory)
{
    throw SCAGException("Action '%s': cannot have a child object",opname());
}


bool ActionCounterBase::prefetch( ActionContext& context,
                                  counter::CounterPtrAny& ptr )
{
    const char* name = name_.getValue(context);
    counter::Manager& mgr = counter::Manager::getInstance();
    ptr = mgr.getAnyCounter(name);
    return postFetch(context,ptr);
}


bool ActionCounterBase::postFetch( ActionContext& context,
                                   counter::CounterPtrAny& ptr )
{
    return setStatus(context,ptr.get() ? OK : NOTFOUND);
}


bool ActionCounterBase::setStatus(ActionContext& ctx, Status status)
{
    if ( status_.isFound() ) {
        Property* p = status_.getProperty(ctx);
        if (p) p->setInt(int(status));
    }
    return (status == OK);
}


void ActionCounterInc::init( const SectionParams& params,
                             PropertyObject propertyObject )
{
    ActionCounterBase::init(params,propertyObject);
    value_.init(params,propertyObject);
    weight_.init(params,propertyObject);
}


bool ActionCounterInc::run( ActionContext& context )
{
    smsc_log_debug(log_,"%s run",opname());
    const int64_t defVal = isInc() ? 1 : 0;
    const int64_t val = value_.isFound() ? value_.getValue(context) : defVal;
    const int64_t weight = weight_.isFound() ? weight_.getValue(context) : defVal;
    counter::CounterPtrAny ptr;
    if ( prefetch(context,ptr) ) {
        ptr->increment(val,weight);
    }
    return 0;
}


void ActionCounterCreate::init( const SectionParams& params,
                                PropertyObject propertyObject )
{
    ActionCounterInc::init(params,propertyObject);
    templid_.init(params,propertyObject);
    reset_.init(params,propertyObject);
    lifetime_.init(params,propertyObject);
}


bool ActionCounterCreate::postFetch( ActionContext& context,
                                     counter::CounterPtrAny& ptr )
{
    if ( ptr.get() ) {
        if ( reset_.getValue(context) ) ptr->reset();
        return setStatus(context,OK);
    }
    counter::Manager& mgr = counter::Manager::getInstance();
    counter::TemplateManager* tmgr = mgr.getTemplateManager();
    if ( !tmgr ) return setStatus(context,NOTFOUND);
    Counter* c = tmgr->createCounter( makeName(name_-.getValue(context)),
                                      templid_.getValue(context),
                                      unsigned(lifetime_.getValue(context)) );
    if ( !c ) return setStatus(context,NOTFOUND);
    try {
        ptr = mgr.registerAnyCounter(c);
    } catch ( std::exception& e ) {
        return setStatus(context,TYPEMISMATCH);
    }
    return setStatus(ptr.get()?OK:NOTFOUND);
}


bool ActionCounterReset::run( ActionContext& context )
{
    smsc_log_debug(log_,"%s run",opname());
    counter::CounterPtrAny ptr;
    if ( prefetch(context,ptr) ) ptr->reset();
    return true;
}


void ActionCounterGet::init( const SectionParams& params,
                             PropertyObject propertyObject )
{
    ActionCounterBase::init(params,propertyObject);
    value_.init(params,propertyObject);
}


bool ActionCounterGet::postFetch( ActionContext& context,
                                  counter::CounterPtrAny& ptr )
{
    if ( ! ptr.get() ) return setStatus(context,NOTFOUND);
    int64_t result = ptr->getValue();
    if ( value_.isFound() ) {
        Property* p = value_.getProperty(context);
        if (p) p->setInt(result);
    }
    return setStatus(context,OK);
}

}
}
}
