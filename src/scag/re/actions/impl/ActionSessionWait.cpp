#include <time.h>
#include <cstdlib>
#include "ActionSessionWait.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {

ActionSessionWait::ActionSessionWait() :
wait_(0)
{
}


void ActionSessionWait::init( const SectionParams& params,
                              PropertyObject propertyObject )
{
    // --- input fields
    bool bExist;
    waitFieldType_ = CheckParameter( params,
                                     propertyObject, 
                                     "session:wait", "time",
                                     true, true,
                                     waitFieldName_,
                                     bExist );

    if ( waitFieldType_ == ftUnknown ) {

        if ( waitFieldName_.empty() )
            throw SCAGException("Action: 'session:wait' should have non-empty time field" );

        static const char* formats[] = {
            "%u:%u:%u", "%u:%u", "%u", 0
        };

        for ( unsigned i = 0; ; ++i ) {

            const char* fmt = formats[i];
            if ( ! *fmt )
                throw SCAGException( "Action: 'session:wait' wrong time field: %s",
                                     waitFieldName_.c_str() );

            unsigned elts[3];
            elts[0] = elts[1] = elts[2] = 0;

            int nelts = sscanf( waitFieldName_.c_str(), fmt,
                                &(elts[0]), &(elts[1]), &(elts[2]) );
            if ( nelts != 3 - i ) continue;

            wait_ = 0;
            for ( unsigned j = 0; j < i; ++j ) {
                wait_ = wait_*60 + elts[j];
            }
            break;
        }
    }
}


IParserHandler * ActionSessionWait::StartXMLSubSection( const std::string&,
                                                        const SectionParams&,
                                                        const ActionFactory& )
{
    throw SCAGException( "Action '%s' cannot include child objects", "session::wait" );
}


bool ActionSessionWait::FinishXMLSubSection( const std::string& )
{
    return true;
}


bool ActionSessionWait::run( ActionContext& context )
{
    unsigned tmo = wait_;
    if ( waitFieldType_ != ftUnknown ) {
        Property* property = context.getProperty( waitFieldName_ );
        if ( ! property ) {
            smsc_log_error( logger, "Action 'session:wait': invalid property %s for time",
                            waitFieldName_.c_str() );
            return true;
        }
        tmo = unsigned(property->getInt());

    }

    context.getSession().waitAtLeast( tmo );
    return true;
}

}
}
}
