#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include "InfosmeCore.h"
#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/buffers/Hash.hpp"
#include "sme/SmppBase.hpp"

namespace smsc {
namespace informer {

class SmscSender;

class InfosmeCoreV1 : public InfosmeCore
{
public:
    InfosmeCoreV1();

    virtual ~InfosmeCoreV1();

    /// notify to stop, invoked from main
    virtual void stop();

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    virtual void configure( const smsc::util::config::ConfigView& cfg );

    /// enter main loop, exit via 'stop()'
    virtual int Execute();

    // smsc has just been stopped
    // virtual void notifySmscFinished( const std::string& smscId );

    /// this methods has several functions:
    /// 1. create smsc: new smscId, valid cfg;
    /// 2. update smsc: old smscId, valid cfg;
    /// 3. delete smsc: old smscId, cfg=0.
    void updateSmsc( const std::string& smscId,
                     const smsc::sme::SmeConfig* cfg );

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
