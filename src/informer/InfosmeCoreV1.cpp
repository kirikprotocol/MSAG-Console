#include "InfosmeCoreV1.h"
#include "SmscSender.h"

namespace eyeline {
namespace informer {

InfosmeCoreV1::InfosmeCoreV1() :
log_(0), stopping_(true), started_(false)
{
    log_ = smsc::logger::Logger::getInstance("core");
}


InfosmeCoreV1::~InfosmeCoreV1()
{
    smsc_log_info(log_,"corev1 dtor");
    tp_.shutdown(0);
    smsc_log_info(log_,"leaving corev1 dtor");
}


void InfosmeCoreV1::stop()
{
    {
        if (stopping_) return;
        MutexGuard mg(startMon_);
        if (stopping_) return;
        smsc_log_info(log_,"stop() received");
        stopping_ = true;
        startMon_.notifyAll();
    }
    tp_.stopNotify();
    MutexGuard mg(startMon_);
    while (started_) {
        startMon_.wait(100);
    }
    smsc_log_debug(log_,"leaving stop()");
}


void InfosmeCoreV1::start()
{
    if (started_) return;
    MutexGuard mg(startMon_);
    if (started_) return;
    stopping_ = false;
    Start();
}


void InfosmeCoreV1::init( const smsc::util::config::ConfigView& cfg )
{
    smsc_log_info(log_,"FIXME: configuring InfosmeCore");
}


int InfosmeCoreV1::Execute()
{
    {
        MutexGuard mg(startMon_);
        started_ = true;
        stopping_ = false;
    }
    smsc_log_info(log_,"starting main loop");
    while ( !stopping_ ) {

        smsc_log_debug(log_,"main loop pass");
        MutexGuard mg(startMon_);
        startMon_.wait(1000);
        // FIXME: flush statistics
    }
    smsc_log_info(log_,"finishing main loop");
    MutexGuard mg(startMon_);
    started_ = false;
    return 0;
}


/*
void InfosmeCoreV1::notifySmscFinished( const std::string& smscId )
{
    MutexGuard mg(startMon_);
    SmscSender** ptr = smscs_.GetPtr(smscId.c_str());
    if (!ptr || !*ptr) return;
    tp_.startTask( new SmscConnector(**ptr) );
}
 */


void InfosmeCoreV1::updateSmsc( const std::string&          smscId,
                                const smsc::sme::SmeConfig* cfg )
{
    // FIXME
    if (cfg) {
        // create/update
        SmscSender* p = 0;
        SmscSender** ptr = 0;
        MutexGuard mg(startMon_);
        try {
            ptr = smscs_.GetPtr(smscId.c_str());
            if (!ptr) {
                p = new SmscSender(*this,smscId,*cfg);
                ptr = smscs_.SetItem(smscId.c_str(),p);
            } else if (*ptr) {
                (*ptr)->updateConfig(*cfg);
                // (*ptr)->waitUntilReleased();
            } else {
                p = new SmscSender(*this,smscId,*cfg);
                *ptr = p;
            }
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"smscsender create error: %s", e.what());
            if (p) {
                smscs_.Delete(smscId.c_str());
                delete p;
            }
        }
        if (ptr && *ptr) tp_.startTask(*ptr);
    } else {
        // delete the smsc
        MutexGuard mg(startMon_);
        SmscSender* ptr = 0;
        if (smscs_.Pop(smscId.c_str(),ptr) && ptr) {
            ptr->stop();
            ptr->waitUntilReleased();
            delete ptr;
        }
    }
    // notifySmscFinished(smscId);
}

}
}
