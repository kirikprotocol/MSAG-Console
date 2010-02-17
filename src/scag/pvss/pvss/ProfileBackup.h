#ifndef _SCAG_PVSS_PVSS_PROFILEBACKUP_H
#define _SCAG_PVSS_PVSS_PROFILEBACKUP_H

#include <vector>
#include "scag/pvss/data/Property.h"

namespace smsc { namespace logger { class Logger; }}

namespace scag2 {
namespace pvss {

class Profile;

class ProfileBackup
{
    static smsc::logger::Logger* log_;

public:

    ProfileBackup();

    ~ProfileBackup() {
        cleanup(); 
    }

    // add property
    void addProperty( const Property& prop );

    /// delete property 
    void delProperty( Property* prop );

    /// fix current time policy
    void fixPolicy( Property& prop );

    /// fix current value of the property
    void fixProperty( Property& prop );

    /// cleanup the backup state
    void cleanup();

    /// restore the profile to its prestine conditions
    void rollback( Profile& prof );

private:
    enum PvssOpType {
        PROPADD = 1,
        PROPDEL = 2,
        PROPMOD = 3,
        PROPFIX = 4
    };
    struct PropTime {
        PropTime( const std::string& n ) : name(n) {}
        PropTime( const Property& prop ) :
        name(prop.getName()),
        timePolicy(prop.getTimePolicy()),
        finalDate(prop.getFinalDate()),
        lifeTime(prop.getLifeTime()) {}
        std::string name;
        TimePolicy  timePolicy;
        time_t      finalDate;
        uint32_t    lifeTime;
    };
    struct PvssOp {
        PvssOp( unsigned p, PvssOpType t ) : position(p), opType(t) {}
        unsigned   position;
        PvssOpType opType;
    };

private:
    std::vector< PvssOp >      operationList_;
    std::vector< Property* >   deletedProperties_;  // delete
    std::vector< Property >    modifiedProperties_; // modify
    std::vector< PropTime >    fixedPolicies_;      // added, timepol
};

}
}

#endif
