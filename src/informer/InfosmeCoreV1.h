#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include "InfosmeCore.h"
#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/threads/Thread.hpp"
#include "core/buffers/Hash.hpp"

namespace eyeline {
namespace informer {

class SmscSender;
class SmscConfig;

class InfosmeCoreV1 : public InfosmeCore, public smsc::core::threads::Thread
{
public:
    InfosmeCoreV1();

    virtual ~InfosmeCoreV1();

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    virtual void init( const smsc::util::config::ConfigView& cfg );

    /// notify to stop, invoked from main
    virtual void start();
    virtual void stop();

    // smsc has just been stopped
    // virtual void notifySmscFinished( const std::string& smscId );

    /// this methods has several functions:
    /// 1. create smsc: new smscId, valid cfg;
    /// 2. update smsc: old smscId, valid cfg;
    /// 3. delete smsc: old smscId, cfg=0.
    void updateSmsc( const std::string& smscId, const SmscConfig* cfg );

protected:
    /// enter main loop, exit via 'stop()'
    virtual int Execute();
    static void readSmscConfig( SmscConfig& cfg,
                                const smsc::util::config::ConfigView& cv );

private:
    smsc::logger::Logger*                      log_;
    smsc::core::synchronization::EventMonitor  startMon_;
    bool                                       stopping_;
    bool                                       started_;
    smsc::core::threads::ThreadPool            tp_;
    smsc::core::buffers::Hash< SmscSender* >   smscs_; // owned
};

} // informer
} // smsc

#endif
