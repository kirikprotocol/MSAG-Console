#include "TimeField.h"

namespace scag2 {
namespace re {
namespace actions {

bool TimeField::init( const SectionParams& params,
                      PropertyObject& propertyObject,
                      const char* actionName,
                      const char* paramName,
                      bool required,
                      bool readonly )
{
    bool bExist;
    wait_ = 0;
    type_ = Action::CheckParameter( params,
                                   propertyObject,
                                   actionName,
                                   paramName,
                                   required, 
                                   readonly,
                                   name_,
                                   bExist );
    if ( ! bExist ) return false;

    if ( type_ == ftUnknown ) {

        if ( name_.empty() )
            throw SCAGException("Action: '%s' should have non-empty %s field",
                                actionName, paramName );

        static const char* formats[] = {
            "%u:%u:%u", "%u:%u", "%u", 0
        };

        for ( unsigned i = 0; ; ++i ) {

            const char* fmt = formats[i];
            if ( ! fmt )
                throw SCAGException( "Action: '%s' wrong %s field: %s",
                                     actionName, paramName, name_.c_str() );

            unsigned elts[3];
            elts[0] = elts[1] = elts[2] = 0;

            int nelts = sscanf( name_.c_str(), fmt,
                                &(elts[0]), &(elts[1]), &(elts[2]) );
            if ( nelts != int(3-i) ) continue;

            wait_ = 0;
            for ( unsigned j = 0; j < 3-i; ++j ) {
                wait_ = wait_*60 + elts[j];
            }
            break;
        }
    }
    return true;
}

unsigned TimeField::getTime( const char* actionName, ActionContext& context ) const
{
    unsigned tmo = wait_;
    if ( type_ != ftUnknown ) {
        Property* property = context.getProperty( name_ );
        if ( ! property ) {
            // smsc_log_error( logger, "Action '%s': invalid property %s for time",
            // actionName, name_.c_str() );
            throw SCAGException( "Action '%s': invalid property %s for time", 
                                 actionName, name_.c_str() );
        }
        tmo = unsigned(property->getInt());
    }
    return tmo;
}

}
}
}
