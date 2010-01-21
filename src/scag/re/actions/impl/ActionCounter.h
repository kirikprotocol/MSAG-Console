#ifndef _SCAG_RE_ACTIONS_IMPL_ACTIONCOUNTER_H
#define _SCAG_RE_ACTIONS_IMPL_ACTIONCOUNTER_H

#include "scag/re/base/Action2.h"
#include "scag/re/base/StringField.h"
#include "scag/re/base/IntField.h"
#include "scag/re/base/TimeField.h"

namespace scag2 {
namespace counter {
class CounterPtrAny;
}

namespace re {
namespace actions {

class ActionCounterBase : public Action
{
protected:
    enum Status {
            OK = 0,
            NOTFOUND = 1,
            TYPEMISMATCH = 2,
            FUNCNOTFOUND = 3
    };
    enum ScopeType {
            USER = 0,
            SERVICE = 1,
            OPERATOR = 2,
            PROVIDER = 3,
            SYSTEM = 4
    };

protected:
    ActionCounterBase();
    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    virtual IParserHandler* StartXMLSubSection( const std::string& name,
                                                const SectionParams& params,
                                                const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name) {
        return true;
    }

    /// prefetch the counter
    bool prefetch( ActionContext& context, counter::CounterPtrAny& ptr );
    virtual bool postFetch( ActionContext& context, counter::CounterPtrAny& ptr);
    bool setStatus( ActionContext& context, Status status );
    std::string makeName( ActionContext& context, const char* name ) const;

protected:
    static smsc::logger::Logger* log_;

protected:
    StringField name_;
    ScopeType   scope_;
    StringField status_; // to write status
};


class ActionCounterInc : public ActionCounterBase
{
public:
    ActionCounterInc() :
    ActionCounterBase(),
    value_(*this,"inc",false,true),
    weight_(*this,"weight",false,true) {}
    virtual const char* opname() const { return "counter:inc"; }
    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    virtual bool run( ActionContext& context );
protected:
    virtual bool isInc() const { return true; }
    int64_t getValue( ActionContext& context, IntField& field, int64_t defVal );
protected:
    IntField value_;
    IntField weight_;
};


class ActionCounterCreate : public ActionCounterInc
{
public:
    ActionCounterCreate() : ActionCounterInc(),
    templid_(*this,"type",true,true),
    reset_(*this,"reset",false,true),
    lifetime_(*this,"lifetime",false,true)
    {}
    virtual const char* opname() const { return "counter:create"; }
    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    // virtual bool run( ActionContext& context );
protected:
    virtual bool isInc() const { return false; }
    virtual bool postFetch( ActionContext& context,
                            counter::CounterPtrAny& ptr);
protected:
    StringField templid_;
    IntField    reset_;
    TimeField   lifetime_;
};

class ActionCounterReset : public ActionCounterBase
{
public:
    ActionCounterReset() : ActionCounterBase() {}
    virtual const char* opname() const { return "counter:reset"; }
    virtual bool run( ActionContext& context );
};

class ActionCounterGet : public ActionCounterBase
{
public:
    ActionCounterGet() : ActionCounterBase(),
    value_(*this,"value",true,false) {}
    virtual const char* opname() const { return "counter:get"; }
    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    virtual bool run( ActionContext& context );
    // virtual bool postFetch( ActionContext& context,
    // counter::CounterPtrAny& ptr);
protected:
    StringField value_;
};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_IMPL_ACTIONCOUNTER_H */
