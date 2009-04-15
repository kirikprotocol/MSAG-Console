#ifndef _SCAG_TRANSPORT_SMPP_IMPL_SMPPMANAGER2_H
#define _SCAG_TRANSPORT_SMPP_IMPL_SMPPMANAGER2_H

#include <map>
#include "scag/transport/smpp/base/SmppManager2.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/RefPtr.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "scag/config/base/ConfigListener2.h"
#include "SmppSocketManager2.h"
#include "scag/snmp/TrapRecord.h"

namespace scag2 {
namespace transport {
namespace smpp {

class SmppManagerImpl : public SmppManager, public ConfigListener
{
public:
    SmppManagerImpl( snmp::TrapRecordQueue* snmpqueue );
    virtual ~SmppManagerImpl();

    void Init(const char* cfgFile);
    void shutdown();

    // smppmanager
    void LoadRoutes(const char* cfgFile);
    void ReloadRoutes();

  //admin
  virtual void addSmppEntity(const SmppEntityInfo& info);
  virtual void updateSmppEntity(const SmppEntityInfo& info);
  virtual void disconnectSmppEntity(const char* sysId);
  virtual void deleteSmppEntity(const char* sysId);
  virtual SmppEntityAdminInfoList * getEntityAdminInfoList(SmppEntityType entType);

  //registragor
  virtual int registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch);
  virtual int registerSmscChannel(SmppChannel* ch);
  virtual void unregisterChannel(SmppChannel* ch);

  //queue
    virtual void putCommand(SmppChannel* ct, std::auto_ptr<SmppCommand> cmd );
    virtual bool getCommand(SmppCommand*& cmd);

    // lcm
    virtual void continueExecution(LongCallContextBase* lcmCtx, bool dropped);
    virtual bool makeLongCall( std::auto_ptr<SmppCommand>& cx, ActiveSession& session );

  //manager
  void  sendReceipt(Address& from, Address& to, int state, const char* msgId, const char* dst_sme_id, uint32_t netErrCode);

    virtual void pushCommand( SmppCommand* cmd );
    virtual unsigned pushSessionCommand( SmppCommand* cmd,
                                         int action = SCAGCommandQueue::PUSH );

  void reloadTestRoutes(const RouteConfig& rcfg);
    util::RefferGuard<router::RouteManager> getTestRouterInstance();
    void ResetTestRouteManager(router::RouteManager* manager);

  void getQueueLen(uint32_t& reqQueueLen, uint32_t& respQueueLen, uint32_t& lcmQueueLen);

    // cfg listener
  void configChanged();


  //SmppRouter
  virtual SmppEntity* RouteSms(router::SmeIndex srcidx, const smsc::sms::Address& source, const smsc::sms::Address& dest, router::RouteInfo& info)
  {
    {
      RouterRef ref;
      {
        smsc::core::synchronization::MutexGuard rsmg(routerSwitchMtx);
        ref=routeMan;
      }
      if(!ref->lookup(srcidx,source,dest,info))return 0;
    }
    MutexGuard mg(regMtx);
    SmppEntity** ptr=registry.GetPtr(info.smeSystemId);
    if(ptr)
    {
      if((*ptr)->info.enabled)
      {
        return *ptr;
      }else
      {
        return 0;
      }
    }
    MetaEntity** pme=metaRegistry.GetPtr(info.smeSystemId);
    if(!pme)return 0;
    MetaEntity& me=**pme;
    SmppEntity* rv=0;
    if(me.info.persistanceEnabled)
    {
      if(me.info.type==mtMetaService)
      {
        rv=me.getEntity(source);
      }else
      {
        rv=me.getEntity(dest);
      }
    }else
    {
      rv=me.getEntity();
    }
    if(rv)
    {
      info.smeSystemId=rv->getSystemId();
    }
    return rv;
  }

  virtual SmppEntity* getSmppEntity(const char* systemId, bool* isEnabled = 0) const
  {
    MutexGuard mg(regMtx);
    SmppEntity* const *ptr=registry.GetPtr(systemId);
    if (ptr) {
        if (isEnabled) *isEnabled = (*ptr)->info.enabled;
        return *ptr;
    } else {
        if (isEnabled) *isEnabled = false;
        return 0;
    }
  }

  void addMetaEntity(MetaEntityInfo info)
  {
    MutexGuard mg(regMtx);
    if(metaRegistry.Exists(info.systemId.c_str()))
    {
      throw Exception("Duplicate meta entity:%s",info.systemId.c_str());
    }
    MetaEntity* ptr=new MetaEntity;
    ptr->info=info;
    metaRegistry.Insert(info.systemId.c_str(),ptr);
  }

  void updateMetaEntity(MetaEntityInfo info)
  {
    MutexGuard mg(regMtx);
    MetaEntity** ptr=metaRegistry.GetPtr(info.systemId.c_str());
    if(!ptr)
    {
      throw Exception("updateMetaEntity::Meta entity not found:%s",info.systemId.c_str());
    }
    (*ptr)->info=info;
  }

  void deleteMetaEntity(const char* id)
  {
    MutexGuard mg(regMtx);
    MetaEntity** ptr=metaRegistry.GetPtr(id);
    if(!ptr)
    {
      throw Exception("deleteMetaEntity::Meta entity not found:%s",id);
    }
    if(!(*ptr)->ents.empty())
    {
      throw Exception("deleteMetaEntity::Meta entity isn't empty:%s",id);
    }
    metaRegistry.Delete(id);
  }

  void addMetaEndPoint(const char* metaId,const char* sysId)
  {
    MutexGuard mg(regMtx);
    MetaEntity** mptr=metaRegistry.GetPtr(metaId);
    if(!mptr)
    {
      throw Exception("addMetaEndPoint::Meta entity not found:%s",metaId);
    }
    SmppEntity** sptr=registry.GetPtr(sysId);
    if(!sptr)
    {
      throw Exception("addMetaEndPoint::Smpp entity not found:%s",sysId);
    }
    if(((*mptr)->info.type==mtMetaService && (*sptr)->info.type!=etService)||
       ((*mptr)->info.type==mtMetaSmsc && (*sptr)->info.type!=etSmsc))
    {
      throw Exception("addMetaEndPoint::incompatible type '%s' and '%s'",metaId,sysId);
    }
    MetaEntity &me=**mptr;
    MetaEntity::MetaEntsVector::iterator it;
    for(it=me.ents.begin();it!=me.ents.end();it++)
    {
      if(it->ptr==*sptr)
      {
        throw Exception("addMetaEndPoint::duplicate entity '%s' in meta entity '%s'",sysId,metaId);
      }
    }
    MetaEntity::MetaInfo mi;
    mi.ptr=*sptr;
    (*mptr)->ents.push_back(mi);
  }

  void removeMetaEndPoint(const char* metaId,const char* sysId)
  {
    MutexGuard mg(regMtx);
    MetaEntity** mptr=metaRegistry.GetPtr(metaId);
    if(!mptr)
    {
      throw Exception("removeMetaEndPoint::Meta entity not found:%s",metaId);
    }
    SmppEntity** sptr=registry.GetPtr(sysId);
    if(!sptr)
    {
      throw Exception("removeMetaEndPoint::Smpp entity not found:%s",sysId);
    }
    MetaEntity& me=**mptr;
    MetaEntity::MetaEntsVector::iterator it;
    for(it=me.ents.begin();it!=me.ents.end();it++)
    {
      if(it->ptr==*sptr)
      {
        me.ents.erase(it);
        return;
      }
    }
    throw Exception("removeMetaEndPoint::unexpected error - smpp entity not found in meta entity :(");
  }

  bool LoadEntityFromConfig(SmppEntityInfo& info,const char* sysId,SmppEntityType et);
  bool LoadMetaEntityFromConfig(MetaEntityInfo& info,const char* sysId);


    // -- helpers


  void StopProcessing()
  {
    {
        smsc::core::synchronization::MutexGuard mg(queueMon);
        if(!running) {
            queueMon.notifyAll();
            return;
        }
        running=false;
        queueMon.notifyAll();
    }
    smsc_log_debug(log, "SmppManager shutting down");
    Thread::Yield();
    // tp.stopNotify();
    tp.shutdown(0);
    sm.shutdown();
    smsc_log_debug(log, "SmppManager shutdown");
  }

protected:

    struct MetaEntity
    {

        struct MetaInfo{
            SmppEntity* ptr;
        };

        struct MappingValue;
        typedef std::vector<MetaInfo> MetaEntsVector;
        typedef std::map<Address,MappingValue> AbonentsMap;
        typedef std::multimap<time_t,AbonentsMap::iterator> TimeoutsMap;

        struct MappingValue
        {
            SmppEntity* ptr;
            MappingValue():ptr(0)
            {
            }
            MappingValue(SmppEntity* argPtr):ptr(argPtr)
            {
            }
            TimeoutsMap::iterator toit;
        };

    public:
        MetaEntity():lastEntity(0)
        {
            seed=(unsigned int)time(NULL);
        }

        SmppEntity* getStoredMapping(const Address& addr)
        {
            AbonentsMap::iterator it=storedMappings.find(addr);
            if(it==storedMappings.end())return 0;
            timeMap.erase(it->second.toit);
            timeMap.insert(TimeoutsMap::value_type(time(NULL)+expirationTimeout,it));
            return it->second.ptr;
        }

        void createStoredMapping(const Address& addr,SmppEntity* ptr)
        {
            AbonentsMap::iterator it=storedMappings.insert(AbonentsMap::value_type(addr,MappingValue(ptr))).first;
            it->second.toit=timeMap.insert(TimeoutsMap::value_type(time(NULL)+expirationTimeout,it));
        }

        void expireMappings()
        {
            time_t now=time(NULL);
            TimeoutsMap::iterator it;
            while(!timeMap.empty() && now>(it=timeMap.begin())->first)
            {
                storedMappings.erase(it->second);
                timeMap.erase(it);
            }
        }

        SmppEntity* getEntity(const Address& addr)
        {
            smsc::core::synchronization::MutexGuard mg(mtx);
            SmppEntity* ptr=getStoredMapping(addr);
            if(!ptr)
            {
                mtx.Unlock();
                try{
                    ptr=getEntity();
                    createStoredMapping(addr,ptr);
                }catch(...)
                    {
                        
                    }
                mtx.Lock();
            }
            expireMappings();
            return ptr;
        }

        SmppEntity* getEntity()
        {
            smsc::core::synchronization::MutexGuard mg(mtx);
            if(info.policy==bpRandom)
            {
                double val=rand_r(&seed);
                val/=RAND_MAX;
                lastEntity=val*ents.size();
            }
            for(unsigned i=0;i<ents.size();i++)
            {
                lastEntity++;
                if(lastEntity>=ents.size())
                {
                    lastEntity=0;
                }
                smsc::core::synchronization::MutexGuard mg2(ents[lastEntity].ptr->mtx);
                if(ents[lastEntity].ptr->bt==btNone || !ents[lastEntity].ptr->info.enabled)continue;
                return ents[lastEntity].ptr;
            }
            return 0;
        }

    public:
        static time_t expirationTimeout;

    public:
        MetaEntityInfo info;
        MetaEntsVector ents;
        unsigned int lastEntity;
        unsigned int seed;
        smsc::core::synchronization::Mutex mtx;
        AbonentsMap storedMappings;
        TimeoutsMap timeMap;
    };


protected:
    unsigned getUnsigned( const char* name, unsigned defval ) const;

protected:
    smsc::logger::Logger* log;
    smsc::logger::Logger* limitsLog;
    smsc::core::buffers::Hash<SmppEntity*> registry;
    smsc::core::buffers::Hash<MetaEntity*> metaRegistry;
    mutable smsc::core::synchronization::Mutex regMtx;
    SmppSocketManager sm;

    smsc::util::TimeSlotCounter<> licenseCounter;
    time_t lastLicenseExpTest;
    int licenseFileCheckHour;

    bool running;

    // command queues (owned)
    smsc::core::buffers::CyclicQueue< SmppCommand* > queue, lcmQueue, respQueue;
    smsc::core::synchronization::EventMonitor queueMon;
    time_t lastExpireProcess;

    typedef RefPtr<router::RouteManager,smsc::core::synchronization::Mutex> RouterRef;
    RouterRef routeMan;
    smsc::core::synchronization::Mutex routerSwitchMtx;
//  std::string routerConfigFile;

    Mutex routerSwitchMutex;
    util::Reffer<router::RouteManager>* testRouter_;

    smsc::core::threads::ThreadPool tp;

    int lastUid;
    int queueLimit;

    uint32_t lcmProcessingCount;
    uint32_t queuedCmdCount;
    std::string cfgFileName;

    snmp::TrapRecordQueue* snmpqueue_;    // not owned
};

}//smpp
}//transport
}//scag

#endif
