#ifndef _SCAG_PVSS_PVSS_PROFILECOMMANDPROCESSOR_H_
#define _SCAG_PVSS_PVSS_PROFILECOMMANDPROCESSOR_H_

#include <memory>

#include "scag/pvss/profile/Profile.h"
#include "scag/pvss/api/packets/CommandResponse.h"
#include "scag/pvss/api/packets/ProfileCommandVisitor.h"

// #include "DBLog.h"

namespace scag2 {
namespace pvss {

class ProfileRequest;
class ProfileBackup;
class BatchResponseComponent;

class ProfileCommandProcessor : public ProfileCommandVisitor 
{
public:
    ProfileCommandProcessor( ProfileBackup& backup, Profile* prof = 0 ) :
    log_(smsc::logger::Logger::getInstance("pvss.proc")),
    backup_(&backup), rollback_(false), profile_(prof), response_(0) {}

    inline CommandResponse* getResponse() { return response_.release(); }
    bool applyCommonLogic( const std::string& profkey,
                           ProfileRequest&    profileRequest,
                           Profile*           pf,
                           bool               createProfile );

    void finishProcessing( bool commit );

protected:
    /// full cleanup at the start of processing new profile
    void resetProfile(Profile *pf);
    // void flushLogs(Logger* logger);

    bool visitBatchCommand(BatchCommand &cmd) /* throw(PvapException) */ ;
    bool visitDelCommand(DelCommand &cmd) /* throw(PvapException) */ ;
    bool visitGetCommand(GetCommand &cmd) /* throw(PvapException) */ ;
    bool visitIncCommand(IncCommand &cmd) /* throw(PvapException) */ ;
    bool visitIncModCommand(IncModCommand &cmd) /* throw(PvapException) */ ;
    bool visitSetCommand(SetCommand &cmd) /* throw(PvapException) */ ;
    // BatchResponseComponent* getBatchResponseComponent();
    // const string& getDBLog() const;
    // void clearDBLog();

private:
    uint8_t incModProperty(Property& property, uint32_t mod, uint32_t &result);
    // void addBatchComponentDBLog(const string& logmsg);

private:
    smsc::logger::Logger* log_;
    ProfileBackup*        backup_;
    bool                  rollback_;
    // DBLog                 dblog_;
    // std::vector<std::string> batchLogs_;
    Profile*              profile_;
    std::auto_ptr<CommandResponse> response_;
};

}//pvss
}//scag2

#endif


