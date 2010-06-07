#ifndef __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE2_H__

#include <vector>
#include "scag/transport/smpp/base/SmppCommandQueue2.h"
#include "scag/transport/smpp/base/SmppRouter2.h"
#include "scag/transport/smpp/base/SmppCommand2.h"
#include "scag/sessions/base/Session2.h"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace scag2 {

namespace util {
class HRTiming;
}

namespace transport {
namespace smpp {

class StateMachine: public smsc::core::threads::ThreadedTask
{
public:
    /// init registry.
    /// must be invoked before any StateMachine ctor.
    static void initRegistry( unsigned respTimeout );

    StateMachine( SmppCommandQueue* argQueue, SmppRouter* argRouteMan ) :
    queue_(argQueue), routeMan_(argRouteMan), log_(0)
    {
        log_ = smsc::logger::Logger::getInstance("smpp.stm");
    }

    const char* taskName(){return "StateMachine";}
    int Execute();

    /*
    static void addTransitOptional(int tag)
    {
        allowedUnknownOptionals.push_back(tag);
        smsc_log_debug(smsc::logger::Logger::getInstance("smpp.stm"), "Transitional tag added: %x", tag);
    }
     */

protected:
    void processSmResp( std::auto_ptr<SmppCommand> aucmd,
                        sessions::ActiveSession session = sessions::ActiveSession() );
    void processSm( std::auto_ptr<SmppCommand> aucmd, util::HRTiming* hrt );
    // void processDelivery( std::auto_ptr<SmppCommand> aucmd, util::HRTiming* hrt );
    // void processDataSm( std::auto_ptr<SmppCommand> aucmd, util::HRTiming* hrt );
    void processExpiredResps();
    void processAlertNotification( std::auto_ptr<SmppCommand> aucmd);
    // void processSubmitResp( std::auto_ptr<SmppCommand> cmd,
    // sessions::ActiveSession session = sessions::ActiveSession() );
    // void processDeliveryResp( std::auto_ptr<SmppCommand> cmd,
    // sessions::ActiveSession session = sessions::ActiveSession() );
    // void processDataSmResp( std::auto_ptr<SmppCommand> cmd,
    // sessions::ActiveSession session = sessions::ActiveSession() );

    // void SubmitResp( std::auto_ptr<SmppCommand> cmd,int status);
    // void DeliveryResp( std::auto_ptr<SmppCommand> cmd,int status);
    // void DataResp( std::auto_ptr<SmppCommand> cmd,int status);

    void registerEvent(int event, SmppEntity* src, SmppEntity* dst, const char* rid, int errCode);

    /// in case of success SmppCommand is taken
    uint32_t putCommand(CommandId cmdType, SmppEntity* src, SmppEntity* dst, router::RouteInfo& ri, std::auto_ptr<SmppCommand>& cmd );

    // not used ?
    // bool makeLongCall(SmppCommand& cx, SessionPtr& session);

    struct ResponseRegistry;

    void sendReceipt(std::auto_ptr<SmppCommand> cmd);

    int getUSSDOp( const char* where, SMS& sms, SmsCommand* smscmd ) const;

protected: // static
    static sync::Mutex expMtx_;
    static bool expProc_;
    static ResponseRegistry reg_;
    static std::vector<int> allowedUnknownOptionals;

protected:
    SmppCommandQueue*     queue_;
    SmppRouter*           routeMan_;
    smsc::logger::Logger* log_;
};

}//smpp
}//transport
}//scag


#endif
