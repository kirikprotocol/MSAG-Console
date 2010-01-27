// a test of timesnapshot
#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/counter/Manager.h"
#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/impl/TemplateManagerImpl.h"


using namespace scag2::counter;


void initCounters()
{
    impl::HashCountManager* mgr = 
        new impl::HashCountManager( new impl::TemplateManagerImpl(), 10 );
    mgr->start();
}


const char* smppname = "sys.traffic.smpp";

int main()
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");
    initCounters();

    {
        // making a counter templates
        Manager& mgr = Manager::getInstance();
        TemplateManager* tmgr = mgr.getTemplateManager();
        tmgr->replaceTemplate(smppname,
                              CounterTemplate::create("timesnapshot",0,5));
    }
    
    CounterPtrAny p = Manager::getInstance().createCounter(smppname,smppname);

    {
        smsc::core::synchronization::EventMonitor m;
        MutexGuard mg(m);
        for ( size_t i = 0; i < 500; ++i ) {
            int64_t val = p->increment();
            smsc_log_info(logger,"value=%llu",val);
            m.wait(100);
        }
    }
    return 0;
}
