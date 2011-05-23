#include "ActionGenReceiptId.h"
#include "scag/util/Time.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionGenReceiptId::init( const SectionParams& params,
                               PropertyObject propertyObject )
{
    id_.init(params,propertyObject);
}


IParserHandler * ActionGenReceiptId::StartXMLSubSection( const std::string&,
                                                         const SectionParams&,
                                                         const ActionFactory& )
{
    throw SCAGException( "Action '%s' cannot include child objects", opname() );
}


bool ActionGenReceiptId::FinishXMLSubSection( const std::string& )
{
    return true;
}


bool ActionGenReceiptId::run( ActionContext& context )
{
    Property* property = id_.getProperty(context);
    if (!property) {
        throw SCAGException("Action '%s': Invalid property for %s", opname(),id_.getName());
    }
    static util::msectime_type startMsec = 0;
    static smsc::core::synchronization::Mutex lock;
    if (!startMsec) {
        smsc::core::synchronization::MutexGuard mg(lock);
        if (!startMsec) {
            struct ::tm t;
            memset(&t,0,sizeof(t));
            t.tm_year = 2010 - 1900;
            t.tm_mday = 1;
            t.tm_isdst = -1;
            startMsec = util::msectime_type(mktime(&t)) * 1000;
        }
    }
    const util::msectime_type msec = util::currentTimeMillis();
    util::msectime_type diff = msec - startMsec;
    {
        smsc::core::synchronization::MutexGuard mg(lock);    
        static util::msectime_type curVal = 0;
        if ( diff < curVal ) { diff = ++curVal; }
        else { curVal = diff; }
    }
    
    char buf[100];
    snprintf(buf,sizeof(buf),"%llx",int64_t(diff));
    property->setStr(buf);
    return true;
}

}
}
}
