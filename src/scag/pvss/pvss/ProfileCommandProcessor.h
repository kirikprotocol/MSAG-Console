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

class ProfileCommandProcessor
{
public:
    class ReadonlyFilter : public ProfileCommandVisitor
    {
    public:
        ReadonlyFilter( ProfileCommandProcessor& proc, bool readonly ) :
        proc_(proc), readonly_(readonly) {}
        bool applyCommonLogic( const std::string& profkey,
                               ProfileRequest&    profileRequest,
                               Profile*           pf,
                               bool               createProfile );

        bool visitBatchCommand(BatchCommand &cmd) /* throw(PvapException) */ ;
        bool visitDelCommand(DelCommand &cmd) /* throw(PvapException) */ ;
        bool visitGetCommand(GetCommand &cmd) /* throw(PvapException) */ ;
        bool visitIncCommand(IncCommand &cmd) /* throw(PvapException) */ ;
        bool visitIncModCommand(IncModCommand &cmd) /* throw(PvapException) */ ;
        bool visitSetCommand(SetCommand &cmd) /* throw(PvapException) */ ;
        bool visitGetProfileCommand(GetProfileCommand& cmd);
    private:
        ProfileCommandProcessor& proc_;
        bool readonly_;
    };


public:
    ProfileCommandProcessor( ProfileBackup& backup,
                             Profile* prof = 0 ) :
    log_(smsc::logger::Logger::getInstance("pvss.proc")),
    backup_(&backup), rollback_(false), profile_(prof), response_(0) {}

    inline CommandResponse* getResponse() { return response_.release(); }
    void finishProcessing( bool commit );

protected:
    /// full cleanup at the start of processing new profile
    void resetProfile(Profile *pf);
    uint8_t incModProperty(Property& property, uint32_t mod, uint32_t &result);

private:
    smsc::logger::Logger* log_;
    ProfileBackup*        backup_;
    bool                  rollback_;
    Profile*              profile_;
    std::auto_ptr<CommandResponse> response_;
};

}//pvss
}//scag2

#endif


