#ifndef __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE2_H__

#include <vector>
#include "SmppCommandQueue2.h"
#include "SmppRouter2.h"
#include "SmppCommand2.h"
#include "scag/sessions/Session2.h"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace transport {
namespace smpp {

namespace router = scag::transport::smpp::router;

namespace thr = smsc::core::threads;
using namespace sessions;

class StateMachine: public thr::ThreadedTask
{
public:
    StateMachine( SmppCommandQueue* argQueue, SmppRouter* argRouteMan ) :
    queue_(argQueue), routeMan_(argRouteMan), log_(0)
    {
        log_ = smsc::logger::Logger::getInstance("statmach");
    }

    const char* taskName(){return "StateMachine";}
    int Execute();

    static void addTransitOptional(int tag)
    {
        allowedUnknownOptionals.push_back(tag);
        smsc_log_debug(smsc::logger::Logger::getInstance("statmach"), "Transitional tag added: %x", tag);
    }

protected:
    void processSubmit( std::auto_ptr<SmppCommand> aucmd);
    void processDelivery( std::auto_ptr<SmppCommand> aucmd);
    void processDataSm( std::auto_ptr<SmppCommand> aucmd);
    void processExpiredResps();
    void processAlertNotification( std::auto_ptr<SmppCommand> aucmd);
    void processSubmitResp( std::auto_ptr<SmppCommand> cmd,
                            ActiveSession session = ActiveSession() );
    void processDeliveryResp( std::auto_ptr<SmppCommand> cmd,
                              ActiveSession session = ActiveSession() );
    void processDataSmResp( std::auto_ptr<SmppCommand> cmd,
                            ActiveSession session = ActiveSession() );

    void SubmitResp( std::auto_ptr<SmppCommand> cmd,int status);
    void DeliveryResp( std::auto_ptr<SmppCommand> cmd,int status);
    void DataResp( std::auto_ptr<SmppCommand> cmd,int status);

    void registerEvent(int event, SmppEntity* src, SmppEntity* dst, const char* rid, int errCode);

    /// in case of success SmppCommand is taken
    uint32_t putCommand(CommandId cmdType, SmppEntity* src, SmppEntity* dst, router::RouteInfo& ri, std::auto_ptr<SmppCommand>& cmd );

    // not used ?
    // bool makeLongCall(SmppCommand& cx, SessionPtr& session);

    struct ResponseRegistry;

    void sendReceipt(std::auto_ptr<SmppCommand> cmd);

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
