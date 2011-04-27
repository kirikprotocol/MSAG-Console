#ifndef _SCAG_PVSS_PVSS_PROFILEBACKUP_H
#define _SCAG_PVSS_PVSS_PROFILEBACKUP_H

#include <vector>
#include "scag/pvss/data/Property.h"

namespace smsc { namespace logger { class Logger; }}

namespace scag2 {
namespace pvss {

class Profile;
class ProfileLog;

class ProfileBackup
{
    static smsc::logger::Logger* log_;

private:
    enum PvssOpType {
        PROPADD = 1,
        PROPDEL = 2,
        PROPMOD = 3,
        PROPFIX = 4,    // timepol, e.g. expiration
        PROPGET = 5
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
        unsigned  position;
        uint8_t   opType;
    };
    struct LogMsg {
        LogMsg( const std::string& m, PvssOpType o ) : msg(m), opType(o) {}
        std::string msg;
        uint8_t     opType;
    };

public:

    // ProfileBackup( smsc::logger::Logger* dblog );
    ProfileBackup( ProfileLog* dblog );

    ~ProfileBackup();

    /// add property.
    void addProperty( const Property& prop, bool logOnly = false );

    /// log that property was accessed.
    void getProperty( const Property& prop );

    /// fix the property value.  preparation for update.
    void fixProperty( const Property& propold );

    /// log that property was modified.
    void propertyUpdated( const Property& propnew );

    /// delete property.
    void delProperty( Property* prop, bool logOnly = false );

    /// expire property (the message goes to the log immediately).
    void expireProperty( const char* profkey, const Property& prop );

    /// fix current time policy.
    void fixTimePolicy( const Property& prop );

    /// cleanup the backup state.
    void cleanup();

    /// restore the profile to its prestine conditions
    void rollback( Profile& prof );

    /// or, alternatively, flush important logs to the logger
    void flushLogs( const Profile& prof );

private:
    inline void addLogMsg( PvssOpType optype, const std::string& message )
    {
        if ( jlen_ < journal_.size() ) {
            LogMsg& msg = journal_[jlen_++];
            msg.opType = uint8_t(optype);
            msg.msg = message;
        } else {
            journal_.push_back( LogMsg(message,optype) );
            jlen_ = journal_.size();
        }
    }

private:
    // smsc::logger::Logger*      dblog_;
    ProfileLog*                dblog_;
    std::vector< PvssOp >      operationList_;
    std::vector< Property* >   deletedProperties_;  // delete
    std::vector< Property >    modifiedProperties_; // modify
    std::vector< PropTime >    fixedPolicies_;      // added, timepol
    std::vector< LogMsg >      journal_;
    size_t                     jlen_;
};

}
}

#endif
