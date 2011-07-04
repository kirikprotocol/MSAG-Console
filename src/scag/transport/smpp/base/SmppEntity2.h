#ifndef _SCAG_TRANSPORT_SMPP_SMPPENTITY2_H
#define _SCAG_TRANSPORT_SMPP_SMPPENTITY2_H

#include "SmppChannel2.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/Hash.hpp"
#include "SmppEntityInfo.h"
#include "util/timeslotcounter.hpp"
#include "scag/transport/OpId.h"
#include "logger/Logger.h"
#include "scag/util/RelockMutexGuard.h"
#include "scag/transport/smpp/router/route_types.h"
#include "scag/counter/Manager.h"
#include "scag/counter/TimeSnapshot.h"
#include "scag/counter/Accumulator.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace transport {
namespace smpp {

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;


struct SmppEntity
{
private:
    static const int sarTimeout = 300; // 5 min

    struct SarData {
        SmppEntity* dst;
        opid_type   opid;    // operation or invalidOpId()
        int         osar;    // output   sar
        int         count;   // number of chunks passed thru dst
        int         total;
        time_t      creationTime;
        SarData() :
        dst(0), opid(invalidOpId()), osar(0), count(0), total(0), creationTime(0) {}
        SarData( opid_type theopid, time_t curTime ) :
        dst(0), opid(theopid), osar(0), count(0), total(0), creationTime(curTime) {}
    };

public:
  SmppEntity()
    // incCnt(counter::Manager::getInstance().createCounter(cntTrafficName,
    // cntTrafficName))
  {
      log_ = smsc::logger::Logger::getInstance("smpp.ent");
      reset();
      seq = 0;
      slicingSeq8 = 0;
      slicingSeq16 = 0;
      connected = false;
      // queueCount->reset();
  }
  SmppEntity(const SmppEntityInfo& argInfo)
  {
      log_ = smsc::logger::Logger::getInstance("smpp.ent");
      reset();
      info=argInfo;
      {
          const char* cname = ( info.type == etSmsc ?
                                "sys.traffic.smpp.smsc" :
                                "sys.traffic.smpp.sme" );
          char buf[100];
          snprintf(buf,sizeof(buf),"%s.%s",cname,info.systemId.c_str());
          counter::Manager& mgr = counter::Manager::getInstance();
          counter::ObserverPtr o = mgr.getObserver(cname);
          incCnt = mgr.registerAnyCounter(new counter::TimeSnapshot(buf,5,100,o.get()));
          if (!incCnt.get()) {
              // smsc_log_error(log_,"entity '%s' cannot create counter '%s'",
              // info.systemId.c_str(),buf);
              throw exceptions::SCAGException("entity '%s' cannot create counter '%s'",
                                              info.systemId.c_str(), buf);
          }
      }
      seq = 1;
      slicingSeq8 = 0;
      slicingSeq16 = 0;
      {
          const char* cname = "sys.smpp.queue.in";
          char buf[100];
          snprintf(buf,sizeof(buf),"%s.%s",cname,info.systemId.c_str());
          counter::Manager& mgr = counter::Manager::getInstance();
          counter::ObserverPtr o = mgr.getObserver(cname);
          queueCount = mgr.registerAnyCounter(new counter::Accumulator(buf,o.get()));
          if (!queueCount.get()) {
              throw exceptions::SCAGException("entity '%s' cannot create counter '%s'",
                                              info.systemId.c_str(), buf);
          }
          // const unsigned queueLimit = (info.inQueueLimit>0 ? info.inQueueLimit : 1000000 );
          // if (queueLimit>0) queueCount->setMaxVal(queueLimit);
      }
      connected = false;
  }

    /// reset to an initial state, used at disconnect
    void reset()
    {
        {
            MutexGuard mg(mtx);
            bt = btNone;
            channel.reset(0);
            transChannel.reset(0);
            recvChannel.reset(0);
            info.type = etUnknown;
        }
        // {
        // NOTE: we don't reset seq nums.
        // MutexGuard mg(seqMtx);
        // seq = 0;
        // slicingSeq8 = 0;
        // slicingSeq16 = 0;
        // }
        {
            MutexGuard mg(sarMutex_);
            sarMapping_.Empty();
        }
    }


  ~SmppEntity()
  {
      // delMapping();
  }
    
  SmppBindType getBindType()const
  {
    return bt;
  }
  const char* getSystemId()const
  {
    return info.systemId.c_str();
  }
  void putCommand( std::auto_ptr<SmppCommand>& cmd )
  {
    MutexGuard mg(mtx);
    switch(bt)
    {
      case btNone:
        throw smsc::util::Exception("Attempt to putCommand to unbound enitity '%s'",info.systemId.c_str());
      case btReceiver:
        recvChannel->putCommand(cmd);
        break;
      case btTransmitter:
        transChannel->putCommand(cmd);
        break;
      case btRecvAndTrans:
        // switch(cmd->get_commandId())
        {
            ::abort();
        }
        break;
      case btTransceiver:
        channel->putCommand(cmd);
        break;
    }
  }
  /*
  void setUid(int argUid)
  {
    MutexGuard mg(mtx);
    switch(bt)
    {
      case btNone:
        throw smsc::util::Exception("Failed to set uid of unbound entity '%s'",info.systemId.c_str());
      case btReceiver:
        recvChannel->setUid(argUid);
        break;
      case btTransmitter:
        transChannel->setUid(argUid);
        break;
      case btRecvAndTrans:
        recvChannel->setUid(argUid);
        transChannel->setUid(argUid);
        break;
      case btTransceiver:
        channel->setUid(argUid);
        break;
    }
  }
   */
  int  getUid()
  {
    MutexGuard mg(mtx);
    switch(bt)
    {
      case btNone:
        throw smsc::util::Exception("Failed to get uid of unbound entity '%s'",info.systemId.c_str());
      case btReceiver:
        return recvChannel->getUid();
      case btTransmitter:
        return transChannel->getUid();
      case btRecvAndTrans:
        return transChannel->getUid();
      case btTransceiver:
        return channel->getUid();
    }
      throw smsc::util::Exception("unknown bt=%d in getUid of '%s'", bt, info.systemId.c_str() );
  }

  int getNextSeq()
  {
    MutexGuard mg(seqMtx);
    return ++seq;
  }
  
    int getNextSlicingSeq( uint8_t slicingType )
    {
        MutexGuard mg(seqMtx);
        if ( slicingType == router::SlicingType::UDH8 ) {
            return ++slicingSeq8;
        } else {
            return ++slicingSeq16;
        }
    }

  int getQueueCount()
  {
      return int(queueCount->getValue());
      // MutexGuard mg(cntMtx);
      // return queueCount;
  }

  void incQueueCount()
  {
      // MutexGuard mg(cntMtx);
      // queueCount++;
      queueCount->increment();
  }

  void decQueueCount()
  {
      // MutexGuard mg(cntMtx);
      // queueCount--;
      queueCount->increment(-1);
  }

    /// return existing mapping or invalidOpId().
    /// NOTE: sarmr contains: type*0x10000 + sar
    opid_type getSarMappingOpid( int sarmr, time_t currentTime ) {
        MutexGuard mg(sarMutex_);
        SarData* ptr = sarMapping_.GetPtr(sarmr);
        if (!ptr) return invalidOpId();
        else if ( currentTime > ptr->creationTime + sarTimeout ) {
            smsc_log_warn(log_,"too old mapping has been found tmo=%u, deleted", currentTime - ptr->creationTime );
            sarMapping_.Delete(sarmr);
            return invalidOpId();
        }
        return ptr->opid;
    }

    /// \return true if mapping was replaced.
    bool setSarMappingOpid(int sarmr, opid_type opid) {
        util::RelockMutexGuard mg(sarMutex_);
        SarData* ptr = sarMapping_.GetPtr(sarmr);
        const time_t currentTime = time(0);
        if (ptr) {
            SarData old = *ptr;
            *ptr = SarData(opid,currentTime);
            mg.Unlock();
            smsc_log_debug(log_,"multipart mapping (sme/sar=%s/%x) replaced: dst/opid/sar/cnt/tot=%s/%u/%x/%u/%u -> opid=%u",
                           getSystemId(),
                           sarmr,
                           old.dst ? old.dst->getSystemId() : "",
                           old.opid,
                           old.osar,
                           old.count,
                           old.total,
                           opid);
            return true;
        } else {
            sarMapping_.Insert(sarmr,SarData(opid,currentTime));
            mg.Unlock();
            smsc_log_debug(log_,"multipart mapping (sme/sar=%s/%x) created: opid=%u",
                           getSystemId(), sarmr, opid );
            return false;
        }
    }


    void delSarMapping( int sarmr ) {
        {
            MutexGuard mg(sarMutex_);
            sarMapping_.Delete(sarmr);
        }
        smsc_log_debug(log_,"multipart sms mapping on sme=%s deleted: sar=%x",
                       getSystemId(), sarmr );
    }


    /// return a sarmr, that should be used on output
    int countSlicedOnOutput( SmppEntity* dst, SmppCommand* cmd )
    {
        // get slicing data
        int sarmr, currentIndex, lastIndex;
        SMS& sms = *cmd->get_sms();
        SmppCommand::getSlicingParameters(sms,sarmr,currentIndex,lastIndex);
        if ( uint8_t(sarmr/0x10000) == router::SlicingType::NONE ) 
            return router::SlicingType::NONE * 0x10000;

        const char* what = "used";
        bool replaced = false;
        SarData old;
        int result, counted;
        opid_type opid;
        const time_t currentTime = time(0);
        time_t oldTime;
        bool exception = false;
        {
            // check if we already have this sms
            MutexGuard mg(sarMutex_);
            SarData* ptr = sarMapping_.GetPtr(sarmr);
            if (! ptr) {
                what = "created";
                ptr = & sarMapping_.Insert(sarmr,SarData(invalidOpId(),currentTime));
            }
            do {
                oldTime = ptr->creationTime;
                if ( ptr->dst != 0 ) {
                    if ( ptr->dst == dst && ptr->total == lastIndex ) {
                        // fully compared
                        break; 
                    } else if ( oldTime + sarTimeout < currentTime ) {
                        // expired
                        what = "tmo-replaced";
                    } else {
                        // not expired but different dst/total found!
                        // it may mean that a packet has been redirected to a different dst!
                        what = "replaced";
                        exception = true;
                    }
                    replaced = true;
                    old = *ptr;
                    ptr->opid = invalidOpId();
                    ptr->creationTime = currentTime;
                }
                ptr->dst = dst;
                ptr->count = 0;
                ptr->total = lastIndex;
                // obtain a new output sar
                ptr->osar = dst->getNextSlicingSeq( sarmr/0x10000 ) | 
                    ( sarmr & 0xffff0000 );
            } while ( false );

            // increment parts count
            counted = ++ptr->count;
            result = ptr->osar;
            opid = ptr->opid;
            if ( ptr->count >= ptr->total && opid == invalidOpId() ) {
                // no operation, need to destroy
                sarMapping_.Delete(sarmr);
                what = "destroyed";
            }
        } // mutex scope
        if ( replaced ) {
            smsc_log_warn(log_,"multipart mapping (sme/sar=%s/%x) %s: dst/opid/sar/cnt/tot=%s/%u/%x/%u/%u -> %s/%u/%x/%u/%u dt=%u",
                          getSystemId(), sarmr, what,
                          old.dst ? old.dst->getSystemId() : "",
                          old.opid,
                          old.osar,
                          old.count,
                          old.total,
                          dst->getSystemId(),
                          opid,
                          result,
                          counted,
                          lastIndex,
                          unsigned(currentTime-oldTime) );
            if ( exception ) {
                throw smsc::util::Exception("different dst found for multipart: dst1=%s dst2=%s",
                                            old.dst ? old.dst->getSystemId() : "",
                                            dst->getSystemId() );
            }
        } else {
            smsc_log_debug(log_,"multipart mapping (sme/sar=%s/%x) %s: dst/opid/sar/cnt/tot=%s/%u/%x/%u/%u dt=%u",
                           getSystemId(), sarmr,
                           what,
                           dst->getSystemId(),
                           opid,
                           result,
                           counted,
                           lastIndex,
                           unsigned(currentTime-oldTime) );
        }
        return result;
    }


    /// check the sliced destination, returns false if there is non-expired
    /// mapping with a different dst, otherwise returns true.
    bool checkSlicedDestination( uint32_t sarmr, SmppEntity* dst, time_t currentTime )
    {
        MutexGuard mg(sarMutex_);
        SarData* ptr = sarMapping_.GetPtr(sarmr);
        if (ptr && ptr->dst != 0) {
            if (ptr->dst != dst) {
                if (ptr->creationTime+sarTimeout < currentTime) {
                    // expired
                    sarMapping_.Delete(sarmr);
                    return true;
                }
                // not expired, different dst
                return false;
            }
        }
        return true;
    }


    bool isEnabledAndBound() {
        smsc::core::synchronization::MutexGuard mg(mtx);
        return ( info.enabled && bt != btNone );
    }


    /// @return old value of 'enabled' flag
    bool updateInfo( const SmppEntityInfo& newInfo )
    {
        smsc::core::synchronization::MutexGuard mg(mtx);
        bool oldEnabled = info.enabled;
        info = newInfo;
        return oldEnabled;
    }


    void disconnect()
    {
        smsc::core::synchronization::MutexGuard mg(mtx);
        switch(bt)
        {
        case btTransceiver:
            channel->disconnect();
            break;
        case btTransmitter:
            transChannel->disconnect();
            break;
        case btRecvAndTrans:
            transChannel->disconnect();
            //fallthru
        case  btReceiver:
            recvChannel->disconnect();
            break;
        default:
            // nothing
            break;
        }
    }


    bool getAdminInfo( SmppEntityType entType, SmppEntityAdminInfo& ai ) {
        smsc::core::synchronization::MutexGuard mg(mtx);
        if ( info.type != entType ) return false;
        ai.systemId = getSystemId();
        ai.bindType = bt;
        ai.host = info.host;
        ai.port = info.port;
        ai.connected = connected;
        return true;
    }


    /// check if entity is allowed to connect, and then connect
    /// @param password may be NULL (do not check password)
    /// @return:
    /// NULL              - successfully connected;
    /// "" (empty string) - already connected;
    /// non-empty         - failure reason static text.
    const char* checkAndBindChannel( SmppEntityType eType,
                                     SmppBindType   bindType,
                                     SmppChannel*   ch,
                                     int            argUid,
                                     const char*    password )
    {
        MutexGuard mg(mtx);
        if (!info.enabled) { return "disabled"; }
        if (password && info.password != password) { return "password mismatch"; }
        if (info.type == etUnknown) { return "was disabled"; }
        if (info.type != eType) { return "type mismatch"; }
        if ((info.bindType==btReceiver && bindType!=btReceiver)||
            (info.bindType==btTransmitter && bindType!=btTransmitter)) {
            return "bind type mismatch";
        }

        if((bt==btReceiver && bindType==btReceiver)||
           (bt==btTransmitter && bindType==btTransmitter)||
           (bt!=btNone))
        {
            // already connected
            return "";
        }

        if(bt==btNone) {
            bt=bindType;
        } else {
            // upgrade transmitter/receiver to recv and trans
            bt=btRecvAndTrans;
        }

        switch (bindType) {
        case btTransceiver: channel.reset(ch); break;
        case btReceiver : recvChannel.reset(ch); break;
        case btTransmitter: transChannel.reset(ch); break;
        default: throw smsc::util::Exception("logic error: cant have bindType=%d",bindType);
        }
        // info.host = get
        connected = true;
        if (eType == etService) {
            info.host = ch->getPeer();
        }
        switch(bt) {
        case btNone:
            throw smsc::util::Exception("Failed to set uid of unbound entity '%s'",info.systemId.c_str());
        case btReceiver:
            recvChannel->setUid(argUid);
            break;
        case btTransmitter:
            transChannel->setUid(argUid);
            // fall thru
        case btRecvAndTrans:
            recvChannel->setUid(argUid);
            // transChannel->setUid(argUid);
            break;
        case btTransceiver:
            channel->setUid(argUid);
            break;
        }
        return 0; // ok
    }


    /// unbind the channel.
    /// return true if successful
    bool unbindChannel( SmppChannel* ch ) {
        MutexGuard mg(mtx);
        if ( bt == btRecvAndTrans ) {
            const SmppBindType chbt = ch->getBindType();
            if ( chbt == btReceiver ) {
                bt = btTransmitter;
            } else if ( chbt == btTransmitter ) {
                bt = btReceiver;
            } else {
                return false;
            }
        } else {
            bt = btNone;
        }
        connected = false;
        if ( info.type == etService ) { info.host = ""; }
        return true;
    }

public:
    SmppEntityInfo info; // must be changed under mutex
    SmppBindType bt;
    counter::CounterPtrAny incCnt;

protected:
    Mutex mtx;
    SmppChannelPtr channel;
    SmppChannelPtr recvChannel;
    SmppChannelPtr transChannel;
    bool connected;

protected:
    int seq;
    uint8_t  slicingSeq8;
    uint16_t slicingSeq16;

  Mutex seqMtx;
    smsc::logger::Logger* log_;

    /// mapping of isar -> {opid,osar,count}
    smsc::core::synchronization::Mutex        sarMutex_;
    smsc::core::buffers::IntHash< SarData >   sarMapping_;

    counter::CounterPtrAny queueCount;
};

}//smpp
}//transport
}//scag

#endif
