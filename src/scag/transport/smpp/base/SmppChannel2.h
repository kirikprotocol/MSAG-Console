#ifndef _SCAG_TRANSPORT_SMPP_SMPPCHANNEL_H
#define _SCAG_TRANSPORT_SMPP_SMPPCHANNEL_H

#include "logger/Logger.h"
#include "SmppCommand2.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/buffers/FixedLengthString.hpp"
#include "informer/io/EmbedRefPtr.h"

namespace scag2 {
namespace transport {
namespace smpp {

class SmppChannel {
    friend class eyeline::informer::EmbedRefPtr< SmppChannel >;

protected:
    SmppChannel() : log_(smsc::logger::Logger::getInstance("smpp.sock")), ref_(0) {}

public:
    virtual ~SmppChannel() {}
  virtual SmppBindType getBindType()const=0;
  virtual const char* getSystemId()const=0;

    virtual void putCommand(std::auto_ptr<SmppCommand> cmd)=0;
  virtual void disconnect()=0;
  void setUid(int argUid){uid=argUid;}
  int  getUid(){return uid;}
  const char* getPeer() const { return peername_.c_str(); }

  /// note: unsynced
  inline unsigned refCount() const { return ref_; }

protected:
    void ref() {
        smsc::core::synchronization::MutexGuard mg(refLock_);
        ++ref_;
        smsc_log_debug(log_,"ref +1 = %u",ref_);
    }
    
    void unref() {
        {
            smsc::core::synchronization::MutexGuard mg(refLock_);
            --ref_;
            smsc_log_debug(log_,"ref -1 = %u",ref_);
            if (ref_>0) {
                return;
            }
        }
        delete this;
    }

protected:
    smsc::logger::Logger*                      log_;
    int uid;
    smsc::core::buffers::FixedLengthString<32> peername_;
private:
    smsc::core::synchronization::Mutex refLock_;
    unsigned                           ref_;
};

typedef eyeline::informer::EmbedRefPtr< SmppChannel >  SmppChannelPtr;

}//smpp
}//transport
}//scag

#endif
