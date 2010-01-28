#include "logger/Logger.h"
#include "scag/counter/TemplateManager.h"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"

namespace scag2 {
namespace counter {
namespace impl {

class TemplateManagerImpl : public TemplateManager
{
public:
    TemplateManagerImpl();
    virtual ~TemplateManagerImpl();
    virtual Counter* createCounter( const char*        templid,
                                    const std::string& name,
                                    unsigned           lifetimeSeconds );
    virtual void replaceTemplate( const char* name, CounterTemplate* tmpl );

    // action tables
    virtual ObserverPtr getObserver( const char* name );
    virtual void replaceObserver( const char* name, Observer* table );

private:
    smsc::logger::Logger*                         log_;
    smsc::core::synchronization::Mutex            lock_;
    smsc::core::buffers::Hash< CounterTemplate* > templates_;
    smsc::core::buffers::Hash< Observer* >     actionTables_;
};

}
}
}
