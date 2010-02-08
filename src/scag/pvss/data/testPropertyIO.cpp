#include <iostream>
#include "Property.h"

using namespace scag2::pvss;


void setPropVal( Property& property, PropertyType pt )
{
    switch ( pt ) {
    case INT : property.setIntValue(int(time(0))); break;
    case STRING : property.setStringValue("хелло, ворлд"); break;
    case DATE: property.setDateValue(time(0)); break;
    case BOOL: property.setBoolValue(time(0) % 2 ? true : false); break;
    }
}


int main()
{
    const TimePolicy timePolicies[] = {
        INFINIT, FIXED, ACCESS, R_ACCESS, W_ACCESS
    };

    const PropertyType propTypes[] = {
        INT, STRING, DATE, BOOL
    };

    for ( unsigned i = 0; i < sizeof(timePolicies)/sizeof(TimePolicy); ++i ) {
        TimePolicy tp = timePolicies[i];
        for ( unsigned j = 0; j < sizeof(propTypes)/sizeof(PropertyType); ++j ) {
            PropertyType pt = propTypes[j];

            std::cout << std::endl << "======" << std::endl;

            std::string name = "test_";
            name += propertyTypeToString(pt);
            name += "_";
            name += timePolicyToString(tp);

            Property property;
            property.setName(name);

            setPropVal( property, pt );
            property.setTimePolicy( tp, -1, 12345 );

            const std::string stream = property.toString();
            std::cout << "property: " << stream << std::endl;
            property.setTimePolicy( tp, property.getFinalDate(), property.getLifeTime() );
            const std::string stream2 = property.toString();
            std::cout << "property[2]: " << stream2 << std::endl;

            Property prop2;
            prop2.fromString(stream);
            const std::string prop2str = prop2.toString();
            std::cout << "prop2:    " << prop2str << std::endl;
            if ( stream != prop2str || stream != stream2 ) {
                std::cout << "*** ABOVE TWO STRINGS DIFFER!" << std::endl;
            }
        }
    }
    return 0;
}
