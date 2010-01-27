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
#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace transport {
namespace smpp {

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;

/*
struct UMRUSR
{
  IntHash<int> UMRtoUSR;
  IntHash<int> USRtoUMR;
};
 */


struct SmppEntity
{
private:
    // static const char* cntTrafficSme;// = "sys.traffic.smpp.sme";
    // static const char* cntTrafficSmsc;// = "sys.traffic.smpp.smsc";
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
    // incCnt(counter::Manager::getInstance().createCounter("sys.traffic.smpp",
    // ""))
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
          incCnt = counter::Manager::getInstance().createCounter(cname,buf);
          if (!incCnt.get()) {
              // smsc_log_error(log_,"entity '%s' cannot create counter '%s'",
              // info.systemId.c_str(),buf);
              throw exceptions::SCAGException("entity '%s' cannot create counter '%s'",
                                              info.systemId.c_str(), buf);
          }
          const unsigned sendLimit = (info.sendLimit>0)?info.sendLimit:100000;
          if (sendLimit>0) {
              incCnt->setMaxVal(incCnt->getBaseInterval()*sendLimit);
          }
      }
      seq = 1;
      slicingSeq8 = 0;
      slicingSeq16 = 0;
      {
          const char* cname = "sys.smpp.queue.in";
          char buf[100];
          snprintf(buf,sizeof(buf),"%s.%s",cname,info.systemId.c_str());
          queueCount = counter::Manager::getInstance().createCounter(cname,buf);
          if (!queueCount.get()) {
              throw exceptions::SCAGException("entity '%s' cannot create counter '%s'",
                                              info.systemId.c_str(), buf);
          }
          const unsigned queueLimit = (info.inQueueLimit>0 ? info.inQueueLimit : 1000000 );
          if (queueLimit>0) queueCount->setMaxVal(queueLimit);
      }
      connected = false;
  }

    /// reset to an initial state, used at disconnect
    void reset()
    {
        bt = btNone;
        channel = 0;
        transChannel = 0;
        recvChannel = 0;
        {
            // NOTE: we don't reset seq nums.
            // MutexGuard mg(seqMtx);
            // seq = 0;
            // slicingSeq8 = 0;
            // slicingSeq16 = 0;
        }
        MutexGuard mg(sarMutex_);
        sarMapping_.Empty();
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


public:
  SmppEntityInfo info;
  SmppBindType bt;
  Mutex mtx;
  SmppChannel* channel;
  SmppChannel* recvChannel;
  SmppChannel* transChannel;
  bool connected;
  // smsc::util::TimeSlotCounter<> incCnt;
    counter::CounterPtrAny incCnt;

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
