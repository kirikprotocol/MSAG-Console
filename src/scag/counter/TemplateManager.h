#ifndef _SCAG_COUNTER_TEMPLATEMANAGER_H
#define _SCAG_COUNTER_TEMPLATEMANAGER_H

#include <string>
#include "Observer.h"

namespace scag2 {
namespace counter {

/// NOTE: a template for counter creation
class CounterTemplate
{
private:
    CounterTemplate( Counter* prototype ) :
    prototype_(prototype) {}

public:
    Counter* getPrototype() { return prototype_; }
    ~CounterTemplate();

    static CounterTemplate* create( const char*        type,
                                    Observer*          observer,
                                    int64_t            param0 = 0,
                                    int64_t            param1 = 0 );
private:
    CounterTemplate();
    CounterTemplate( const CounterTemplate& );
    CounterTemplate& operator = ( const CounterTemplate& );

private:
    Counter*    prototype_;  // owned, i.e. it is not registered in manager
};


class TemplateManager
{
public:
    virtual ~TemplateManager() {}
    virtual Counter* createCounter( const char*        templid,
                                    const std::string& name,
                                    unsigned lifetimeSeconds ) = 0;
    /// replace a template with a new one, or delete
    virtual void replaceTemplate( const char* name, CounterTemplate* tmpl ) = 0;
    
    // action tables
    virtual ObserverPtr getObserver( const char* name ) = 0;
    virtual void replaceObserver( const char* name, Observer* table ) = 0;
};

}
}

#endif /* !_SCAG_COUNTER_TEMPLATEMANAGER_H */
