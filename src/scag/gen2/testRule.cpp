#include <time.h>
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "core/buffers/IntHash64.hpp"
#include "core/threads/ThreadPool.hpp"
#include "informer/io/IOConverter.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/TmpBuf.h"
#include "informer/io/HexDump.h"
#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/impl/TemplateManagerImpl.h"
#include "scag/lcm/impl/LongCallManagerImpl.h"
#include "scag/pvss/api/core/client/Client.h"
#include "scag/re/impl/RuleEngine2.h"
#include "scag/sessions/base/SessionStore2.h"
#include "scag/sessions/base/SessionManager2.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/transport/smpp/base/SmppRouter2.h"
#include "scag/transport/smpp/base/SmppManager2.h"
#include "scag/transport/smpp/impl/SmppSocket2.h"
#include "scag/transport/smpp/impl/SmppStateMachine2.h"
#include "scag/transport/smpp/router/route_manager.h"
#include "scag/util/io/Print.h"
#include "smpp/smpp.h"

using namespace scag2::re;
using namespace scag2::lcm;
using namespace scag2::bill;
using namespace scag2::transport::smpp;
using namespace smsc::core::synchronization;
using namespace eyeline::informer;

// ========================================================

class FakeConfigManager : public scag2::config::ConfigManager
{
public:
    virtual void reloadConfig(scag2::config::ConfigType type) { abort(); }
    virtual void reloadAllConfigs() { abort(); }

    virtual scag2::config::SmppManConfig& getSmppManConfig() { abort(); }
    virtual scag2::config::RouteConfig& getRouteConfig() { abort(); }
    virtual scag2::config::StatManConfig& getStatManConfig() { abort(); }
    virtual scag2::config::BillingManagerConfig& getBillManConfig() { abort(); }
    virtual scag2::config::SessionManagerConfig& getSessionManConfig() { abort(); }
    virtual scag2::config::HttpManagerConfig& getHttpManConfig() { abort(); }
    virtual scag2::config::PersClientConfig& getPersClientConfig() { abort(); }
    virtual scag2::config::LongCallManagerConfig& getLongCallManConfig() { abort(); }
    virtual scag2::config::LicenseInfo& getLicense() { abort(); }
    virtual void checkLicenseFile() { abort(); }
    
    virtual scag2::config::Config* getConfig() { return 0; }

protected:
    virtual void registerListener( scag2::config::ConfigType t, scag2::config::ConfigListener* l ) {}
    virtual void removeListener( scag2::config::ConfigType t ) {}
};

// ========================================================

class FakeInfrastructure : public scag2::bill::Infrastructure
{
    typedef smsc::core::buffers::IntHash< uint32_t > IntIntMap;
    typedef smsc::core::buffers::Hash< uint32_t >    StrIntMap;
public:
    FakeInfrastructure() :
    log_(smsc::logger::Logger::getInstance("infra")),
    service_hash(0),
    mask_hash(0),
    category_hash(0),
    media_type_hash(0),
    category_str_hash(0),
    media_type_str_hash(0),
    tariff_hash(0),
    currentMediaIdx_(0),
    currentCatIdx_(0)
    {
        service_hash = new IntIntMap();
        mask_hash = new StrIntMap();
        category_hash = new IntIntMap();
        media_type_hash = new IntIntMap();
        category_str_hash = new StrIntMap();
        media_type_str_hash = new StrIntMap();
        tariff_hash = new smsc::core::buffers::IntHash<TariffRec>();
    }

    ~FakeInfrastructure()
    {
        delete service_hash;
        delete mask_hash;
        delete category_hash;
        delete media_type_hash;
        delete category_str_hash;
        delete media_type_str_hash;
        delete tariff_hash;
    }

    virtual void ReloadProviderMap() { abort(); }
    virtual void ReloadOperatorMap() { abort(); }
    virtual void ReloadTariffMatrix() { abort(); }

    virtual uint32_t GetProviderID(uint32_t service_id) 
    {
        const uint32_t* ptr = service_hash->GetPtr(service_id);
        return ptr ? *ptr : 0U;
    }

    virtual uint32_t GetOperatorID(Address addr)
    {
        std::string a = addr.toString();
        int maskidx = a.size();
        const uint32_t* ptr = 0;
        while ( 0 == (ptr = mask_hash->GetPtr(a.c_str())) && maskidx > 5 ) {
            a[--maskidx] = '?';
        }
        return ptr ? *ptr : 0;
    }

    virtual uint32_t GetMediaTypeID(const std::string& media_type_str)
    {
        const uint32_t* ptr = media_type_str_hash->GetPtr(media_type_str.c_str());
        return ptr ? *ptr : 0;
    }
    virtual uint32_t GetCategoryID(const std::string& category_str)
    {
        const uint32_t* ptr = category_str_hash->GetPtr(category_str.c_str());
        return ptr ? *ptr : 0;
    }
    virtual TariffRec* GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt)
    {
        try {
            const uint32_t id = TariffRec::makeHashKeyChecked( category_hash->Get(category),
                                                               media_type_hash->Get(mt),
                                                               operator_id );
            return new TariffRec(tariff_hash->Get(id));
        } catch (...) {
            return 0;
        }
    }
    virtual bool GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt, TariffRec& tr)
    {
        try {
            const uint32_t id = TariffRec::makeHashKeyChecked( category_hash->Get(category),
                                                               media_type_hash->Get(mt),
                                                               operator_id );
            tr = tariff_hash->Get(id);
            return true;
        } catch (...) {
            return false;
        }
    }

    void registerService( uint32_t serviceId, uint32_t providerId )
    {
        service_hash->Insert(serviceId,providerId);
        smsc_log_debug(log_,"service %d registered for provider %d",serviceId,providerId);
    }

    void registerOperator( uint32_t operatorId, const char** masks )
    {
        for ( const char** p = masks; *p != 0; ++p ) {
            smsc::sms::Address a(*p);
            mask_hash->Insert(a.toString().c_str(), operatorId);
        }
        if ( log_->isDebugEnabled() ) {
            std::string mstr;
            for ( const char** p = masks; *p != 0; ++p ) {
                mstr += " ";
                mstr += *p;
            }
            smsc_log_debug(log_,"operator %d registered with masks:%s",operatorId,mstr.c_str());
        }
    }

    void registerMediaType( uint32_t mediaTypeId, const char* medianame )
    {
        assert( !media_type_hash->GetPtr(mediaTypeId) );
        media_type_hash->Insert( mediaTypeId, ++currentMediaIdx_ );
        media_type_str_hash->Insert( medianame, currentMediaIdx_ );
    }

    void registerCategory( uint32_t categoryId, const char* catname )
    {
        assert( !category_hash->GetPtr(categoryId) );
        category_hash->Insert( categoryId, ++currentCatIdx_ );
        category_str_hash->Insert( catname, currentCatIdx_ );
    }

    void registerTariffRecord( uint32_t operatorId,
                               const char* serviceNumber,
                               uint32_t mediaTypeId,
                               uint32_t categoryId,
                               uint32_t bill_type,
                               const char* price,
                               const char* currency )
    {
        const uint32_t mt = media_type_hash->Get(mediaTypeId);
        const uint32_t cat = category_hash->Get(categoryId);
        TariffRec tr(serviceNumber, price, currency, categoryId, mediaTypeId, bill_type );
        const uint32_t id = TariffRec::makeHashKeyChecked(cat,mt,operatorId);
        tariff_hash->Insert(id,tr);
    }

private:
    smsc::logger::Logger* log_;
    IntIntMap* service_hash;
    StrIntMap* mask_hash;
    IntIntMap* category_hash;
    IntIntMap* media_type_hash;
    StrIntMap* category_str_hash;
    StrIntMap* media_type_str_hash;
    smsc::core::buffers::IntHash<TariffRec>* tariff_hash;
    unsigned currentMediaIdx_;
    unsigned currentCatIdx_;
};


class FakeBillingManager : public scag2::bill::BillingManager
{
public:
    FakeBillingManager() : log_(smsc::logger::Logger::getInstance("billman")) {}
    
    virtual billid_type Open( BillOpenCallParams& openCallParams,
                              scag2::lcm::LongCallContext* lcmCtx = NULL)
    {
        smsc_log_info(log_,"Open()");
        abort();
        return 0;
    }

    virtual void Commit( billid_type billId,
                         scag2::lcm::LongCallContext* lcmCtx = NULL )
    {
        smsc_log_info(log_,"Commin()");
        abort();
    }
    virtual void Rollback( billid_type billId,
                           bool timeout,
                           scag2::lcm::LongCallContext* lcmCtx = NULL)
    {
        smsc_log_info(log_,"Rollback()");
        abort();
    }
    virtual void CommitTransit( BillCloseCallParams& closeCallParams,
                                scag2::lcm::LongCallContext* lcmCtx = NULL )
    {
        smsc_log_info(log_,"CommitTransit()");
        abort();
    }
    virtual void RollbackTransit( BillCloseCallParams& closeCallParams,
                                  scag2::lcm::LongCallContext* lcmCtx = NULL )
    {
        smsc_log_info(log_,"RollbackTransit()");
        abort();
    }
    virtual void Check( BillCheckCallParams& checkCallParams,
                        scag2::lcm::LongCallContext* lcmCtx )
    {
        smsc_log_info(log_,"Check()");
        abort();
    }
    virtual void Info(billid_type billId,
                      BillingInfoStruct& bis,
                      TariffRec& tariffRec)
    {
        smsc_log_info(log_,"Info()");
        abort();
    }
    virtual void Info( EwalletInfoCallParams& infoParams,
                       scag2::lcm::LongCallContext* lcmCtx )
    {
        smsc_log_info(log_,"Info()");
        abort();
    }
    virtual void Transfer( BillTransferCallParams& transferParams,
                           scag2::lcm::LongCallContext* lcmCtx )
    {
        smsc_log_info(log_,"Transfer()");
        abort();
    }
    virtual void Stop()
    {
        smsc_log_info(log_,"Stop()");
        abort();
    }

    virtual Infrastructure& getInfrastructure()
    {
        return infra_;
    }

    inline FakeInfrastructure& getFI() { return infra_; }

private:
    smsc::logger::Logger* log_;
    FakeInfrastructure    infra_;
};


// ========================================================


class FakeSmppChannel : public SmppChannel
{
public:
    FakeSmppChannel( const char* sId ) :
        log_(0),
        bindType(btTransceiver), systemId(sId) 
        {
            char buf[30];
            snprintf(buf,sizeof(buf),"smpp.%s",sId);
            log_ = smsc::logger::Logger::getInstance(buf);
        }

    virtual SmppBindType getBindType() const {
        return bindType;
    }
    virtual const char* getSystemId() const {
        return systemId.c_str();
    }

    virtual void putCommand(std::auto_ptr<SmppCommand> cmd) {
        PduGuard pdu;
        pdu = cmd->makePdu();
        int sz = calcSmppPacketLength(pdu);
        eyeline::informer::TmpBuf<char,1024> tmpbuf;
        tmpbuf.setSize(sz);
        SmppStream st;
        assignStreamWith(&st,tmpbuf.get(),sz,false);
        if (!fillSmppPdu(&st,pdu)) {
            smsc_log_warn(log_,"failed to create buffer from pdu");
            return;
        }
        eyeline::informer::HexDump hd;
        eyeline::informer::HexDump::string_type dump;
        hd.hexdump(dump,tmpbuf.get(),sz);
        smsc_log_info(log_,"out to '%s': %s",systemId.c_str(),hd.c_str(dump));
        scag::util::PrintStdString pss;
        dump_pdu(pdu.pdu,&pss);
        smsc_log_info(log_,"dump: %s",pss.buf());
    }

    virtual void disconnect()
    {
        smsc_log_info(log_,"disconnected");
    }

public:
    smsc::logger::Logger* log_;
    SmppBindType bindType;
    std::string systemId;
};


// ========================================================


class FakeRouteManager : public SmppRouter
{
public:
    FakeRouteManager() : currentUid_(0) {}

    ~FakeRouteManager() {
        shutdown();
        char* smeid;
        SmppEntity* ent;
        for ( smsc::core::buffers::Hash< SmppEntity* >::Iterator i(&registry);
              i.Next(smeid,ent); ) {
            delete ent;
        }
        registry.Empty();
        smelist_.clear();
    }

    void shutdown()
    {
        for ( std::vector< SmppChannelPtr >::iterator i = smelist_.begin();
              i != smelist_.end(); ++i ) {
            SmppEntity** ent = registry.GetPtr((*i)->getSystemId());
            if (!ent) continue;
            (*i)->disconnect();
            (*ent)->unbindChannel(i->get());
        }
    }

    virtual SmppEntity* RouteSms( router::SmeIndex srcidx,
                                  const smsc::sms::Address& source,
                                  const smsc::sms::Address& dest,
                                  router::RouteInfo& info,
                                  std::vector< std::string >* traceit = 0)
    {
        {
            MutexGuard mg(regMtx);
            if (!realManager_.lookup(srcidx,source,dest,info,traceit)) {
                return 0;
            }
        }
        bool enabled;
        SmppEntity* rv = getSmppEntity( info.smeSystemId.c_str(), &enabled );
        if (!enabled) rv = 0;
        return rv;
    }


    void addRoute( scag::transport::smpp::router::RouteInfo& rinfo )
    {
        MutexGuard mg(regMtx);
        realManager_.addRoute(rinfo);
    }


    void commitRoutes( std::vector< std::string >& dump )
    {
        MutexGuard mg(regMtx);
        realManager_.commit();
        realManager_.dumpInto( dump );
    }


    virtual SmppEntity* getSmppEntity(const char* sysId, bool* isEnabled = 0) const
    {
        MutexGuard mg(regMtx);
        SmppEntity* const *ptr=registry.GetPtr(sysId);
        if (ptr) {
            if (isEnabled) *isEnabled = (*ptr)->info.enabled;
            return *ptr;
        } else {
            if (isEnabled) *isEnabled = false;
            return 0;
        }
    }


    SmppChannel& registerSmppEntity( const char* smeid, SmppEntityType et )
    {
        MutexGuard mg(regMtx);
        SmppEntity** ptr = registry.GetPtr(smeid);
        if (ptr) {
            throw smsc::util::Exception("sme %s already registered",smeid);
        }
        std::auto_ptr<SmppChannel> ch( new FakeSmppChannel(smeid) );

        // generate smppentityinfo
        SmppEntityInfo info;
        info.type = et;
        info.systemId = ch->getSystemId();
        info.password = "<no password>";
        info.bindSystemId = "<no bindsysid>";
        info.bindPassword = "<no bindpass>";
        info.timeOut = 100;
        info.bindType = ch->getBindType();
        info.host = "localhost";
        info.enabled = true;

        SmppEntity* entity = new SmppEntity(info);
        entity->unbindChannel(0);
        const char* fail = entity->checkAndBindChannel(et,ch->getBindType(),ch.get(),++currentUid_,0);
        if ( fail ) {
            delete entity;
            throw smsc::util::Exception("sme connect failed: %s",fail);
        }
        registry.Insert( ch->getSystemId(), entity );
        SmppChannel* ret = ch.get();
        smelist_.push_back( SmppChannelPtr(ch.release()));
        return *ret;
    }


    SmppChannel& getSme( const char* smeid )
    {
        SmppChannel* res = 0;
        MutexGuard mg(regMtx);
        for ( std::vector< SmppChannelPtr >::iterator i = smelist_.begin(), ie = smelist_.end();
              i != ie; ++i ) {
            if ( 0 == strcmp(i->get()->getSystemId(),smeid) ) {
                return **i;
            }
        }
        throw smsc::util::Exception("sme '%s' is not found",smeid);
    }

private:
    mutable smsc::core::synchronization::Mutex  regMtx;
    smsc::core::buffers::Hash< SmppEntity* >    registry;
    scag::transport::smpp::router::RouteManager realManager_;
    std::vector< SmppChannelPtr >               smelist_;
    int currentUid_;
};


// ========================================================

class FakeSmppManager : public SmppManager, public SCAGCommandQueue
{
public:
    FakeSmppManager( FakeBillingManager& fbm,
                     RuleEngineImpl&     ruleEngine ) :
    running(true), queueCmdCount_(0), lcmCount_(0),
    fbm_(fbm), ruleEngine_(ruleEngine),
    log_(smsc::logger::Logger::getInstance("smppman"))
    {}


    ~FakeSmppManager() {
        shutdown();
    }

    FakeBillingManager& getFBM() const { return fbm_; }

    RuleEngineImpl& getRE() const { return ruleEngine_; }

    // --- not used
    virtual int registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch) { abort(); return 0; }
    virtual int registerSmscChannel(SmppChannel* ch) { abort(); return 0; }
    virtual void unregisterChannel(SmppChannel* ch) { abort(); }
    virtual void addSmppEntity(const SmppEntityInfo& info) { abort(); }
    virtual void updateSmppEntity(const SmppEntityInfo& info) { abort(); }
    virtual void disconnectSmppEntity(const char* sysId) { abort(); }
    virtual void deleteSmppEntity(const char* sysId) { abort(); }
    virtual void ReloadRoutes() { abort(); }
    virtual SmppEntityAdminInfoList * getEntityAdminInfoList(SmppEntityType entType) { abort(); return 0; }
    virtual bool LoadEntityFromConfig(SmppEntityInfo& info,const char* sysId,SmppEntityType et) { abort(); return false; }
    virtual bool LoadMetaEntityFromConfig(MetaEntityInfo& info,const char* sysId) { abort(); return false; }
    virtual void addMetaEntity(MetaEntityInfo info) { abort(); }
    virtual void updateMetaEntity(MetaEntityInfo info) { abort(); }
    virtual void deleteMetaEntity(const char* id) { abort(); }
    virtual void addMetaEndPoint(const char* metaId,const char* sysId) { abort(); }
    virtual void removeMetaEndPoint(const char* metaId,const char* sysId) { abort(); }
    virtual SmppEntity* RouteSms( router::SmeIndex srcidx,
                                  const smsc::sms::Address& source,
                                  const smsc::sms::Address& dest,
                                  router::RouteInfo& info,
                                  std::vector< std::string >* traceit ) { abort(); return 0; }
    virtual SmppEntity* getSmppEntity(const char* sysId, bool* isEnabled = 0) const { abort(); return 0; }

    virtual void LoadRoutes(const char* cfgFile) { abort(); }

    virtual void  sendReceipt( Address& from,
                               Address& to,
                               int state, 
                               const char* msgId,
                               const char* dst_sme_id,
                               uint32_t netErrCode ) { abort(); }

    /// accept partially processed commands from LCM
    virtual void pushCommand( SmppCommand* cmd ) { abort(); }

    /// if success cx is cleared and session is left locked, true is returned.
    virtual bool makeLongCall( std::auto_ptr<scag2::transport::smpp::SmppCommand>& cx,
                               scag2::sessions::ActiveSession& session ) 
    {
        SmppCommand* cmd = cx.get();
        LongCallContext& lcmCtx = session->getLongCallContext();
        lcmCtx.stateMachineContext = cx.release();
        lcmCtx.initiator = this;
        // cmd->setSession(session);
        // 
        bool b = LongCallManager::Instance().call( &lcmCtx );
        if (b) {
            session.leaveLocked();
            MutexGuard mg(queueMon);
            lcmCount_++;
            smsc_log_debug(log_,"lcmCount increased %d",lcmCount_);
        } else {
            cx.reset( cmd );
            lcmCtx.stateMachineContext = 0;
        }
        return b;
    }

    virtual void reloadTestRoutes(const RouteConfig& rcfg,
                                  std::vector< std::string >* traceit ) {
        abort();
    }

    virtual scag2::util::RefferGuard<router::RouteManager> getTestRouterInstance() {
        abort();
    }
    // virtual void ResetTestRouteManager(router::RouteManager* manager) = 0;
    virtual void getQueueLen( uint32_t& reqQueueLen,
                              uint32_t& respQueueLen,
                              uint32_t& lcmQueueLen) {
        abort();
    }
    // --- end of unused list


    virtual void continueExecution( LongCallContextBase* lcmCtx, bool dropped )
    {
        std::auto_ptr<SmppCommand> cx
            (reinterpret_cast<SmppCommand*>(lcmCtx->stateMachineContext));
        lcmCtx->stateMachineContext = 0;
        lcmCtx->continueExec = ! dropped;
        
        Session* session = cx->getSession();
        __require__( session );
        smsc_log_debug( log_, "continueExec(cmd=%p,sess=%p/%s,drop=%d)", cx.get(),
                        session, session->sessionKey().toString().c_str(),
                        dropped ? 1 : 0 );
        if ( dropped ) {
            // we need to get session here to force unlocking
            ActiveSession as = scag2::sessions::SessionManager::Instance()
                .getSession( session->sessionKey(), cx, false );
            __require__( as.get() );
        }

        MutexGuard mg(queueMon);
        lcmCount_--;
        smsc_log_debug(log_,"lcmCount decreased %d",lcmCount_);

        if (!dropped)
        {
            smsc_log_debug(log_,"cmd pushed into lcmQueue");
            lcmQueue.Push( cx.release() );
            queueMon.notify();
        }
    }

    // iface SCAGCommandQueue
    virtual unsigned pushSessionCommand( SmppCommand* cmd, int action = PUSH )
    {
        return pushCommand(cmd,action);
    }

    virtual unsigned pushCommand( SCAGCommand* cmd, int action = PUSH )
    {
        if (!cmd) return unsigned(-1);
        if (cmd->getType() != SMPP) { abort(); }
        MutexGuard mg(queueMon);
        if ( action == SCAGCommandQueue::RESERVE ) {
            return ++queueCmdCount_;
        }
        if ( action == SCAGCommandQueue::PUSH ) {
            abort();
        }
        if ( queueCmdCount_ > 0 ) --queueCmdCount_;
        SmppCommand* smppCmd = static_cast< SmppCommand* >(cmd);
        if ( smppCmd->isResp() ) {
            respQueue.Push( smppCmd );
        } else {
            queue.Push( smppCmd );
        }
        queueMon.notify();
        return queueCmdCount_;
    }


    void start()
    {
        {
            MutexGuard mg(queueMon);
            running = true;
        }
        stateMachinePool_.startTask( new StateMachine(this,&router_) );
    }
    

    virtual void shutdown()
    {
        smsc_log_info(log_,"shutdown invoked");
        do {
            MutexGuard mg(queueMon);
            if ( queueCmdCount_ > 0 ||
                 queue.Count() > 0 ||
                 respQueue.Count() > 0 ||
                 lcmQueue.Count() > 0 ||
                 lcmCount_ > 0 ) {
                smsc_log_debug(log_,"queueCmd=%d queue=%d respQueue=%d lcmQueue=%d lcm=%d",
                               int(queueCmdCount_),
                               int(queue.Count()),
                               int(respQueue.Count()),
                               int(lcmQueue.Count()),
                               int(lcmCount_));
                queueMon.wait(100);
                continue;
            }
            running = false;
            queueMon.notify();
            stateMachinePool_.stopNotify();
            break;
        } while ( true );
        stateMachinePool_.shutdown(0);
        router_.shutdown();
    }


    SmppChannel& registerSme(const char* smeid, SmppEntityType et)
    {
        return router_.registerSmppEntity(smeid,et);
    }


    SmppChannel& getSme( const char* smeid )
    {
        return router_.getSme(smeid);
    }

    void registerRoute( const SmppChannel& src,
                        const char* srcmask,
                        const char* dstmask,
                        const SmppChannel& dst,
                        int   serviceId )
    {
        scag::transport::smpp::router::RouteInfo rinfo;
        rinfo.srcSmeSystemId = src.getSystemId();
        rinfo.smeSystemId = dst.getSystemId();
        rinfo.srcSubj = std::string("mask:") + srcmask;
        rinfo.source = smsc::sms::Address( srcmask );
        rinfo.dstSubj = std::string("mask:") + dstmask;
        rinfo.dest = smsc::sms::Address( dstmask );
        rinfo.transit = false;
        rinfo.statistics = true;
        rinfo.hideMessage = false;
        rinfo.serviceId = serviceId;
        rinfo.enabled = true;
        rinfo.slicing = scag::transport::smpp::router::SlicingType::NONE;
        rinfo.slicingRespPolicy = scag::transport::smpp::router::SlicingRespPolicy::ALL;
        rinfo.routeId = std::string(src.getSystemId()) + " -> " + dst.getSystemId();
        router_.addRoute( rinfo );
    }


    void commitRoutes( std::vector< std::string >& dump )
    {
        router_.commitRoutes( dump );
    }


    virtual void putCommand( SmppChannel& ch,
                             std::auto_ptr<SmppCommand> cmd )
    {
        SmppEntity* entPtr = router_.getSmppEntity(ch.getSystemId());
        if (!entPtr) {
            throw smsc::util::Exception("Unknown system id:%s",ch.getSystemId());
        }

        cmd->setEntity(entPtr);
        smsc_log_debug(log_,"cmd=%p put sme='%s'",cmd.get(),entPtr->getSystemId());

        const int i = cmd->getCommandId();
        MutexGuard mg(queueMon);
        if ( i == DELIVERY_RESP || i == SUBMIT_RESP || i == DATASM_RESP ) {
            respQueue.Push( cmd.release() );
            queueMon.notify();
        } else if ( i == DELIVERY || i == SUBMIT || i == DATASM ) {
            queue.Push(cmd.release());
            queueMon.notify();
        } else {
            std::auto_ptr<SmppCommand> resp = mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::THROTTLED);
            ch.putCommand(resp);
        }
    }

    virtual bool getCommand( SmppCommand*& cmd ) 
    {
        MutexGuard mg(queueMon);
        if (running && !queue.Count() && !lcmQueue.Count() && !respQueue.Count() ) {
            queueMon.wait(100);
            // break;
            // return false;
        }

        if (lcmQueue.Count()) {
            lcmQueue.Pop(cmd);
        } else if (respQueue.Count()) {
            respQueue.Pop(cmd);
        } else if (queue.Count()) {
            queue.Pop(cmd);
        } else {
            return false;
        }
        return true;
    }

private:
    std::auto_ptr<SmppCommand> mkErrResp( int cmdId, int dlgId, int errCode )
    {
        switch(cmdId) {
        case SUBMIT: return SmppCommand::makeSubmitSmResp("",dlgId,errCode);
        case DELIVERY: return SmppCommand::makeDeliverySmResp("",dlgId,errCode);
        case DATASM: return SmppCommand::makeDataSmResp("",dlgId,errCode);
        default: throw smsc::util::Exception("Unsupported commandId:%d",cmdId);
        }
    }

private:
    bool running;
    smsc::core::synchronization::EventMonitor queueMon;
    smsc::core::buffers::CyclicQueue< SmppCommand* > queue, lcmQueue, respQueue;
    FakeRouteManager router_;
    unsigned queueCmdCount_, lcmCount_;
    smsc::core::threads::ThreadPool stateMachinePool_;
    FakeBillingManager& fbm_;
    RuleEngineImpl&     ruleEngine_;
    smsc::logger::Logger* log_;
};


// ========================================================

class StringParser : public FileReader::RecordReader
{
public:
    StringParser( FakeSmppManager& man,
                  int argc,
                  const char** argv ) :
    log_(smsc::logger::Logger::getInstance("main")),
    smppManager_(man), argc_(argc), argv_(argv) {}

    virtual bool isStopping() {
        return false;
    }

    virtual size_t recordLengthSize() const { return 0; }

    virtual size_t readRecordLength( size_t filePos,
                                     char* buf,
                                     size_t buflen ) {
        const char* found = 
            static_cast<const char*>(memchr(buf,'\n',buflen));
        if (!found) {
            return buflen+1;
        }
        return found - buf + 1;
    }

    virtual bool readRecordData( size_t filePos, char* buf, size_t buflen )
    {
        buf[buflen-1] = '\0';
        if ( buf[0] == '#' ) return false;
        parse(buf);
        return true;
    }

    void parse( const char* str )
    {
        std::vector< std::string > s = splitString(str,-8);
        if ( s.empty() ) return;
        if ( s[0] == "service" && checkArgs(s,2) ) {
            const uint32_t serviceId = getInt(s[1]);
            const uint32_t providerId = getInt(s[2]);
            smppManager_.getFBM().getFI().registerService(serviceId,providerId);
        } else if ( s[0] == "operator" && checkArgs(s,3) ) {
            const uint32_t operatorId = getInt(s[1]);
            std::vector<const char*> masks;
            for ( std::vector< std::string >::const_iterator i = s.begin()+2;
                  i != s.end(); ++i ) {
                masks.push_back(i->c_str());
            }
            masks.push_back(0);
            smppManager_.getFBM().getFI().registerOperator(operatorId,&*masks.begin());
        } else if ( s[0] == "media" && checkArgs(s,2) ) {
            const uint32_t mediaId = getInt(s[1]);
            smppManager_.getFBM().getFI().registerMediaType( mediaId, s[2].c_str() );
        } else if ( s[0] == "category" && checkArgs(s,2) ) {
            const uint32_t catId = getInt(s[1]);
            smppManager_.getFBM().getFI().registerCategory( catId, s[2].c_str() );
        } else if ( s[0] == "tariff" && checkArgs(s,7) ) {
            const uint32_t operId = getInt(s[1]);
            const uint32_t mediaId = getInt(s[3]);
            const uint32_t catId = getInt(s[4]);
            uint32_t billType;
            if ( s[5] == "inman" ) {
                billType = INMAN;
            } else if ( s[5] == "none" ) {
                billType = NONE;
            } else if ( s[5] == "inmansync" ) {
                billType = INMANSYNC;
            } else if ( s[5] == "ewallet" ) {
                billType = EWALLET;
            } else if ( s[5] == "stat" ) {
                billType = STAT;
            } else {
                throw InfosmeException(EXC_LOGICERROR,"billing type '%s' is not supported",s[5].c_str());
            }
            smppManager_.getFBM().getFI().registerTariffRecord( operId,
                                                                s[2].c_str(),
                                                                mediaId,
                                                                catId,
                                                                billType,
                                                                s[6].c_str(),
                                                                s[7].c_str());
        } else if ( s[0] == "sme" && checkArgs(s,1) ) {
            smppManager_.registerSme( s[1].c_str(), etService );
        } else if ( s[0] == "smsc" && checkArgs(s,1) ) {
            smppManager_.registerSme( s[1].c_str(), etSmsc );
        } else if ( s[0] == "route" && checkArgs(s,5) ) {
            SmppChannel& sme1 = smppManager_.getSme(s[1].c_str());
            SmppChannel& sme2 = smppManager_.getSme(s[4].c_str());
            const uint32_t svcId = getInt(s[5]);
            smppManager_.registerRoute( sme1,
                                        s[2].c_str(),
                                        s[3].c_str(),
                                        sme2,
                                        svcId );
        } else if ( s[0] == "rule" && checkArgs(s,2) ) {
            const uint32_t svcId = getInt(s[1]);
            const std::string rule = getStr(s[2]);
            smppManager_.getRE().
                loadRuleFile( RuleKey::create(scag::transport::SMPP, svcId),
                              rule.c_str() );
            smsc_log_info(log_,"rule %d file '%s' is loaded",svcId,rule.c_str());
        } else if ( s[0] == "start" ) {
            std::vector< std::string > dump;
            smppManager_.commitRoutes( dump );
            std::string final;
            for ( std::vector< std::string >::const_iterator it = dump.begin(), ie = dump.end();
                  it != ie; ++it ) {
                final += "   \n" + *it;
            }
            smsc_log_info(log_,"routes commited, dump follows: %s",final.c_str());
            smppManager_.start();
            smsc_log_info(log_,"smppmanager started");
        } else if ( s[0] == "sms" && checkArgs(s,3) ) {
            SmppChannel& sme1 = smppManager_.getSme(s[1].c_str());
            std::vector< std::string > words( splitString(str,2) );

            eyeline::informer::TmpBuf<char,1024> smsbuffer( words[2].size()+1 );
            strcpy(smsbuffer.get(),words[2].c_str());
            ToBuf::stripHexDump(smsbuffer.get());
        
            eyeline::informer::TmpBuf<char,1024> buffer( strlen(smsbuffer.get())/2 );
            ToBuf tb(buffer.get(),buffer.getSize());
            tb.fillFromHexDump( smsbuffer.get() );
        
            smsc::smpp::SmppStream s;
            assignStreamWith(&s,buffer.get(),tb.getPos(),true);
            PduGuard pdu(smsc::smpp::fetchSmppPdu(&s));

            smsc_log_info(log_,"--- smpp pdu created, dump follows ---");

            scag::util::PrintStdString pss;
            dump_pdu(pdu.pdu,&pss);
            smsc_log_info(log_,"dump: %s",pss.buf());

            std::auto_ptr<SmppCommand> cmd( new SmppCommand(pdu) );

            smsc_log_info(log_,"--- smpp command created ---");

            smppManager_.putCommand( sme1, cmd );

        } else if ( s[0] == "wait" && checkArgs(s,1) ) {
            const uint32_t wt = getInt(s[1]);
            timespec ts;
            ts.tv_sec = wt / 1000;
            ts.tv_nsec = long(wt % 1000) * 1000000;
            nanosleep(&ts,0);
        } else if ( s[0] == "stop" ) {
            smppManager_.shutdown();
        } else {
            throw InfosmeException(EXC_NOTIMPL,"not supported keyword '%s'",s[0].c_str());
        }
    }

private:
    bool checkArgs( const std::vector< std::string >& v, size_t args ) const
    {
        if ( v.size() <= args ) {
            throw InfosmeException(EXC_IOERROR, "wrong number of arguments in %s", v[0].c_str() );
        }
        return true;
    }


    uint32_t getInt( const std::string& s ) const
    {
        char* endptr;
        unsigned long v = strtoul(s.c_str(),&endptr,10);
        if ( *endptr != '\0' ) {
            throw InfosmeException(EXC_IOERROR, "wrong argument '%s', must be numeric", s.c_str());
        }
        return uint32_t(v);
    }


    std::string getStr( const std::string& s ) const
    {
        if ( s[0] != '$' ) return s;
        static const std::string argvstr( "$argv[" );
        static const std::string envstr("$env[");
        if ( s.size() > argvstr.size() &&
             0 == strncmp(s.c_str(),argvstr.c_str(),argvstr.size()) &&
             s[s.size()-1] == ']' ) {
            std::string sub( s, argvstr.size(), s.size()-argvstr.size()-1);
            const uint32_t idx = getInt(sub);
            if ( argc_ <= int(idx) ) {
                throw InfosmeException( EXC_IOERROR, "wrong index in '%s'",s.c_str());
            }
            return argv_[idx];
        } else if ( s.size() > envstr.size() &&
                    0 == strncmp(s.c_str(),envstr.c_str(),envstr.size()) &&
                    s[s.size()-1] == ']' ) {
            std::string sub(s,envstr.size(),s.size()-envstr.size()-1);
            const char* p = getenv(sub.c_str());
            if ( !p ) {
                throw InfosmeException( EXC_IOERROR, "item is not in env: '%s'",s.c_str());
            }
            return p;
        }
        return s;
    }


    std::vector< std::string > splitString( const char* s, int maxsplit = 0 ) const
    {
        std::vector< std::string > res;
        for ( const char* p = s; *p != '\0'; ) {
            while ( *p == ' ' || *p == '\t' || *p == '\n' ) {
                ++p;
            }
            const char* start = p;
            if ( *p == '\0' ) break;
            if ( maxsplit < 0 ) {
                if ( int(res.size()) >= -maxsplit ) {
                    // drop the tail
                    break;
                }
            } else if ( maxsplit > 0 ) {
                if ( int(res.size()) >= maxsplit ) {
                    res.push_back( std::string(start) );
                    break;
                }
            }
            while ( *p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' ) {
                ++p;
            }
            res.push_back( std::string(start,size_t(p-start)) );
        }
        return res;
    }

private:
    smsc::logger::Logger* log_;
    FakeSmppManager&      smppManager_;
    const int             argc_;
    const char**          argv_;
};


// ========================================================

class FakeSessionStore : public scag2::sessions::SessionStore
{
public:
    FakeSessionStore() : queue_(0),
        totalSessions_(0), lockedSessions_(0), stopping_(false) {}

    ~FakeSessionStore()
    {
        int64_t key;
        Session* val;
        for ( smsc::core::buffers::IntHash64< Session* >::Iterator it(cache_);
              it.Next(key,val); ) {
            delete val;
        }
        cache_.Empty();
    }

    void init( SCAGCommandQueue* queue )
    {
        MutexGuard mg(mon_);
        queue_ = queue;
        stopping_ = false;
    }

    virtual void releaseSession( Session& session )
    {
        const SessionKey& key = session.sessionKey();
        uint32_t cmd = session.currentCommand();
        if (!cmd) { abort(); }
        MutexGuard mg(mon_);
        if ( session.getLongCallContext().continueExec ) {
            session.getLongCallContext().continueExec = false;
        }
        Session** v = cache_.GetPtr(key.toIndex());
        if ( !v || *v != &session ) {
            abort();
        }

        SCAGCommand* nextcmd = session.popCommand();
        if (nextcmd) {
            setCommandSession(*nextcmd,&session);
            // nextuid = nextcmd->getSerial();
        } else {
            --lockedSessions_;
        }
    }


    virtual void moveLock( Session& s, SCAGCommand* cmd )
    {
        if ( !cmd || !s.currentCommand() ) return;
        if ( s.currentCommand() == cmd->getSerial() ) return; // already set
        Session* olds = cmd->getSession();
        if ( olds && olds != &s ) {
            abort();
        }
        MutexGuard mg(mon_);
        s.setCurrentCommand( cmd->getSerial() );
    }


    void stop() {
        MutexGuard mg(mon_);
        stopping_ = true;
        mon_.notify();
    }

    ActiveSession fetchSession( const SessionKey& key,
                                std::auto_ptr<SCAGCommand>& cmd,
                                bool create = true ) 
    {
        MutexGuard mg(mon_);

        Session* session = cmd->getSession();
        if (session) {
            if (session->currentCommand() != cmd->getSerial()) {
                abort();
            }
        }

        while ( !session ) {

            if (stopping_) { break; }

            Session** v = cache_.GetPtr(key.toIndex());
            if ( v ) {
                // found a ptr to session
                session = *v;
                if ( session ) {
                    // session exists
                    if ( !session->currentCommand() ) {
                        // is free
                        ++lockedSessions_;
                        session->setCurrentCommand( cmd->getSerial() );
                        break;
                    } else if ( session->currentCommand() == cmd->getSerial() ) {
                        // is already mine
                        break;
                    }
                    // is locked
                    queue_->pushCommand( cmd.get(), SCAGCommandQueue::RESERVE );
                    session->appendCommand( cmd.release() );
                    session = 0;
                    break;
                }
            }

            if (!create ) {
                // creation is not allowed
                session = 0;
                break;
            }

            if (!v) {
                // session ptr is not found
                session = cache_.Insert( key.toIndex(), new Session(key) );
            } else {
                session = *v = new Session(key);
            }

            session->setCurrentCommand( cmd->getSerial() );
            ++totalSessions_;
            ++lockedSessions_;
            break;
        }

        if (session) {
            return makeLockedSession(*session,*cmd.get());
        } else {
            return ActiveSession();
        }
    }

    void getSessionsCount( uint32_t& sc, uint32_t& sldc, uint32_t& slkc )
    {
        sldc = sc = totalSessions_;
        slkc = lockedSessions_;
    }

private:

    inline ActiveSession makeLockedSession( Session&     s,
                                            SCAGCommand& c )
    {
        Session* olds = c.getSession();
        const uint32_t oldc = s.currentCommand();
        setCommandSession(c,&s);
        // should be already set
        // s.setCurrentCommand(&c);
        if ( olds && olds != &s ) {
            ::abort();
        }
        if ( oldc && oldc != c.getSerial() ) {
            ::abort();
        }
        s.setLastAccessTime( time(0) );
        return ActiveSession(*this,s);
    }

private:
    EventMonitor mon_;
    smsc::core::buffers::IntHash64< Session* > cache_;
    SCAGCommandQueue* queue_;
    unsigned          totalSessions_;
    unsigned          lockedSessions_;
    bool              stopping_;
};


class FakeSessionManager : public scag2::sessions::SessionManager
{
public:
    FakeSessionManager() {}


    void init( SCAGCommandQueue& cmdqueue )
    {
        store_.init( &cmdqueue );
    }


    virtual void getSessionsCount( uint32_t& sessionsCount,
                                   uint32_t& sessionsLoadedCount,
                                   uint32_t& sessionsLockedCount )
    {
        store_.getSessionsCount(sessionsCount,
                                sessionsLoadedCount,
                                sessionsLockedCount);
    }


    virtual void Stop()
    {
        store_.stop();
    }


    virtual ActiveSession fetchSession( const SessionKey& key,
                                        std::auto_ptr<SCAGCommand>& cmd,
                                        bool create = true ) 
    {
        if ( !cmd.get() ) return ActiveSession();
        return store_.fetchSession( key, cmd, create );
    }



private:
    FakeSessionStore store_;
};


// ========================================================

class FakeStatistics : public scag2::stat::Statistics
{
public:
    FakeStatistics() : log_(smsc::logger::Logger::getInstance("statman")) {}

    virtual void registerEvent(const scag2::stat::SmppStatEvent& se) {
        smsc_log_info(log_,"smpp src='%s' dst='%s' route='%s' smpp=%d",
                      se.srcId, se.dstId, se.routeId, se.errCode );
    }
    virtual void registerEvent(const scag2::stat::HttpStatEvent& se) {
    }

    virtual bool checkTraffic(std::string routeId,
                              scag2::stat::CheckTrafficPeriod period, int64_t value)
    {
        return true;
    }

    virtual void registerSaccEvent( scag2::stat::SaccTrafficInfoEvent* ev)
    {
        smsc_log_info(log_,"sacc traffic skey='%s' svc=%d op=%d dir=%c",
                      ev->Header.pSessionKey.c_str(),
                      ev->Header.iServiceId,
                      ev->Header.iOperatorId,
                      ev->cDirection ? 'O' : 'I');
    }
    virtual void registerSaccEvent( scag2::stat::SaccBillingInfoEvent* ev)
    {
        smsc_log_info(log_,"sacc bill skey='%s' svc=%d op=%d media=%d cat=%d sum=%s unit=%s",
                      ev->Header.pSessionKey.c_str(),
                      ev->Header.iServiceId,
                      ev->Header.iOperatorId,
                      ev->iMediaResourceType,
                      ev->iPriceCatId,
                      ev->fBillingSumm.c_str(),
                      ev->pBillingCurrency.c_str());
    }
    virtual void registerSaccEvent( scag2::stat::SaccAlarmEvent* ev) {}
    virtual void registerSaccEvent( scag2::stat::SaccAlarmMessageEvent* ev) {}
    virtual void Start() {}
    virtual void Stop() {}
private:
    smsc::logger::Logger* log_;
};

// ========================================================

class FakePvssClient : public scag2::pvss::core::client::Client, protected smsc::core::threads::Thread
{
    struct ReqHand 
    {
        ReqHand() {}
        ReqHand( scag2::pvss::Request* r, ResponseHandler* h ) : req(r), handler(h) {}
        scag2::pvss::Request*     req;
        ResponseHandler* handler;
    };

public:
    FakePvssClient() : started_(false), log_(smsc::logger::Logger::getInstance("pvssclient")) {}

    virtual ~FakePvssClient() {
        shutdown();
    }

    void startup() {
        smsc_log_info(log_,"startup");
        started_ = true;
        Start();
    }
    
    void shutdown() {
        smsc_log_info(log_,"shutdown");
        started_ = false;
        queue_.notify();
        WaitFor();
    }
    
    bool canProcessRequest( scag2::pvss::PvssException* exc = 0 ) {
        if (!started_) {
            if (exc) *exc = scag2::pvss::PvssException( scag2::pvss::StatusType::SERVER_SHUTDOWN,
                                                        "not started" );
            return false;
        }
        return true;
    }

    virtual std::auto_ptr<scag2::pvss::Response> processRequestSync(std::auto_ptr<scag2::pvss::Request>& request)
    {
        smsc_log_info(log_,"processSync %s", request->toString().c_str() );
        struct SimpleWaiter : public scag2::pvss::core::client::Client::ResponseHandler
        {
            SimpleWaiter() : resp(0), type(scag2::pvss::StatusType::OK) {}
            void wait() {
                MutexGuard mg(mon);
                while (!resp && type == scag2::pvss::StatusType::OK) {
                    mon.wait(300);
                }
                if (type != scag2::pvss::StatusType::OK) {
                    throw scag2::pvss::PvssException(exc,type);
                }
            }
            virtual void handleResponse( std::auto_ptr<scag2::pvss::Request> req,
                                         std::auto_ptr<scag2::pvss::Response> response) {
                MutexGuard mg(mon);
                resp = response.release();
                mon.notify();
            }
            virtual void handleError(const scag2::pvss::PvssException& exception,
                                     std::auto_ptr<scag2::pvss::Request> request) 
            {
                MutexGuard mg(mon);
                exc = exception.what();
                type = exception.getType();
                if (type == scag2::pvss::StatusType::OK) type = scag2::pvss::StatusType::UNKNOWN;
                mon.notify();
            }

            EventMonitor mon;
            scag2::pvss::Response*   resp;
            std::string              exc;   // if resp == 0
            scag2::pvss::StatusType::Type type;
        };
        SimpleWaiter sw;
        processRequestAsync(request,sw);
        sw.wait();
        smsc_log_debug(log_,"processSync result %s",sw.resp->toString().c_str());
        return std::auto_ptr<scag2::pvss::Response>(sw.resp);
    }

    virtual void processRequestAsync( std::auto_ptr<scag2::pvss::Request>& request,
                                      ResponseHandler& handler )
    {
        smsc_log_info(log_,"processAsync %s", request->toString().c_str() );
        if (!started_) {
            throw scag2::pvss::PvssException(scag2::pvss::StatusType::NOT_CONNECTED,"shutdown");
        }
        queue_.Push(ReqHand(request.release(),&handler));
    }

protected:
    int Execute()
    {
        while (started_) {
            queue_.waitForItem();
            ReqHand rh;
            if (!queue_.Pop(rh)) continue;
            std::auto_ptr<scag2::pvss::Request> req(rh.req);
            try {
                throw scag2::pvss::PvssException( scag2::pvss::StatusType::SERVER_BUSY, "busy");
                // rh.handler->handleResponse(req,resp);
            } catch ( scag2::pvss::PvssException& e ) {
                rh.handler->handleError(e,req);
            } catch ( std::exception& e ) {
                rh.handler->handleError(scag2::pvss::PvssException(scag2::pvss::StatusType::UNKNOWN,
                                                                   "unknown"),req);
            }
        }
        ReqHand rh;
        const scag2::pvss::PvssException e(scag2::pvss::StatusType::NOT_CONNECTED,"shutdown");
        while ( queue_.Pop(rh) ) {
            std::auto_ptr<scag2::pvss::Request> req(rh.req);
            rh.handler->handleError(e,req);
        }
        return 0;
    }

private:
    bool started_;
    smsc::core::buffers::FastMTQueue< ReqHand > queue_;
    smsc::logger::Logger* log_;
};

// ========================================================

int main( int argc, const char** argv )
{
    if (argc<2) {
        fprintf(stderr,"usage: %s rule.xml\n",argv[0]);
        return -1;
    }

    // Logger::initForTest( Logger::LEVEL_DEBUG );
    Logger::Init();
    Logger* mainlog = Logger::getInstance("mainlog");

    const std::string rulePath = argv[1];

    smsc_log_info(mainlog,"rule path: %s",rulePath.c_str());

    {
        assert(new scag2::counter::impl::HashCountManager(new scag2::counter::impl::TemplateManagerImpl()));
    }

    assert( new FakeConfigManager );

    smsc_log_info(mainlog,"--- starting msag rule tester ---");

    {
        assert( new FakeStatistics() );
    }

    smsc_log_info(mainlog,"--- statistics inited ---");

    FakeBillingManager* fbm = new FakeBillingManager();

    /*
    const uint32_t serviceId = 1;
    const uint32_t providerId = 333;
    const uint32_t operatorId = 444;
    const uint32_t mediaTypeId = 17;
    const uint32_t categoryId = 87;

    // billingManager.registerProvider( providerId, "Eyeline" );
    FakeInfrastructure& fi = fbm->getFI();
    fi.registerService( serviceId, providerId );
    const char* operatorMasks[] = { "+79?????????", 0 };
    fi.registerOperator( operatorId, operatorMasks );
    fi.registerMediaType( mediaTypeId, "JAVAGAME" );
    fi.registerCategory( categoryId, "REGULAR" );
    fi.registerTariffRecord( operatorId, ".5.0.MTC.COM",
                             mediaTypeId, categoryId,
                             scag2::bill::infrastruct::STAT,
                             "0.2", "USD" );
     */
        
    smsc_log_info(mainlog,"--- billing manager created ---");

    FakeSessionManager* fsm = new FakeSessionManager();

    FakePvssClient* pvssClient = new FakePvssClient;
    LongCallManagerImpl* lcm = new LongCallManagerImpl(pvssClient);
    lcm->init(1);
    pvssClient->startup();

    smsc_log_info(mainlog,"--- fake session manager created ---");

    RuleEngineImpl* ruleEngine = new RuleEngineImpl();
    ruleEngine->init("");
    smsc_log_info(mainlog,"--- rule engine inited ---");

    // create a state machine
    FakeSmppManager* smppManager = new FakeSmppManager( *fbm, *ruleEngine );
    fsm->init(*smppManager);

    smsc_log_info(mainlog,"--- smpp manager created ---");

    {
        StringParser stringParser(*smppManager, argc, argv);
        FileGuard fg(fileno(stdin));
        {
            FileReader fr(fg);
            eyeline::informer::TmpBuf<char,1024> tbuf;
            fr.readRecords(tbuf,stringParser);
        }
        fg.release();
    }

    /*
    stringParser.parse( "service 1 333" );
    stringParser.parse( "operator 444 +791????????? +792?????????" );
    stringParser.parse( "media 17 JAVAGAME" );
    stringParser.parse( "category 87 REGULAR" );
    stringParser.parse( "tariff 444 .5.0.MTC 17 87 stat 0.2 USD" );
    stringParser.parse( "sme sme1" );
    stringParser.parse( "smsc smsc1" );
    stringParser.parse( "route sme1 .0.1.??? .1.1.79????????? smsc1 1" );
    stringParser.parse( "route sme1 .5.0.??? .1.1.79????????? smsc1 1" );
    stringParser.parse( "route sme1 .5.0.MTC.??? .1.1.79????????? smsc1 1" );
    stringParser.parse( "route smsc1 .1.1.79????????? .0.1.??? sme1 1" );
    stringParser.parse( "route smsc1 .1.1.79????????? .5.0.??? sme1 1" );
    stringParser.parse( "route smsc1 .1.1.79????????? .5.0.MTC.??? sme1 1" );
    stringParser.parse( "rule 1 $argv[1]");
    stringParser.parse( "start" );
    stringParser.parse( "sms sme1 \
00 00 00 53 00 00 00 04 00 00 00 00 00 00 00 04 \
49 6E 66 6F 00 05 00 4D 54 \
43 2E 43 4F 4D \
00 01 01 37 39 32 32 30 30 30 30 30 31 39 00 00 00 00 00 31 31 30 38 31 39 30 35 30 38 \
31 34 30 30 30 2B 00 01 00 03 00 0C 68 65 6C 6C 6F 2C 20 77 6F 72\n\t\t 6C 64" );
    stringParser.parse( "wait 10000" );
    stringParser.parse( "stop" );
     */

    // SmppChannel& sme1 = smppManager->registerSme( "sme1", etService );
    // SmppChannel& smsc1 = smppManager->registerSme( "smsc1", etSmsc );

    // smsc_log_info(mainlog,"--- smpp channels registered ---");

    /*
    {
        smppManager->registerRoute( sme1, ".0.1.???", ".1.1.79?????????", smsc1, serviceId );
        smppManager->registerRoute( sme1, ".5.0.???", ".1.1.79?????????", smsc1, serviceId );
        smppManager->registerRoute( sme1, ".5.0.MTC.???", ".1.1.79?????????", smsc1, serviceId );
        smppManager->registerRoute( smsc1, ".1.1.79?????????", ".0.1.???", sme1, serviceId );
        smppManager->registerRoute( smsc1, ".1.1.79?????????", ".5.0.???", sme1, serviceId );
        smppManager->registerRoute( smsc1, ".1.1.79?????????", ".5.0.MTC.???", sme1, serviceId );
        smsc_log_info(mainlog,"--- smpp routes registered ---");
        std::vector< std::string > dump;
        smppManager->commitRoutes( dump );
        std::string final;
        for ( std::vector< std::string >::const_iterator it = dump.begin();
              it != dump.end(); ++it )
        {
            final += "  \n" + *it;
        }
        smsc_log_info(mainlog,"dump: %s",final.c_str());
    }
     */

    // ruleEngine->loadRuleFile( RuleKey::create(scag::transport::SMPP, serviceId), rulePath );

    // smsc_log_info(mainlog,"--- rule loaded ---");

    // smppManager->start();

    // smsc_log_info(mainlog,"--- smpp manager started ---");

    /*
    {
        // create a SmppCommand, and put it into fake queue
        const char* sms = "00 00 00 53 00 00 00 04 00 00 00 00 00 00 00 04 \
49 6E 66 6F 00 05 00 4D 54 \
43 2E 43 4F 4D \
00 01 01 37 39 32 32 30 30 30 30 30 31 39 00 00 00 00 00 31 31 30 38 31 39 30 35 30 38 \
31 34 30 30 30 2B 00 01 00 03 00 0C 68 65 6C 6C 6F 2C 20 77 6F 72\n\t\t 6C 64";

        eyeline::informer::TmpBuf<char,1024> smsbuffer( strlen(sms)+1 );
        strcpy(smsbuffer.get(),sms);
        eyeline::informer::ToBuf::stripHexDump(smsbuffer.get());

        
        eyeline::informer::TmpBuf<char,1024> buffer( strlen(smsbuffer.get())/2 );
        eyeline::informer::ToBuf tb(buffer.get(),buffer.getSize());
        tb.fillFromHexDump( smsbuffer.get() );
        
        smsc::smpp::SmppStream s;
        assignStreamWith(&s,buffer.get(),tb.getPos(),true);
        PduGuard pdu(smsc::smpp::fetchSmppPdu(&s));

        smsc_log_info(mainlog,"--- smpp pdu created, dump follows ---");

        scag::util::PrintStdString pss;
        dump_pdu(pdu.pdu,&pss);
        smsc_log_info(mainlog,"dump: %s",pss.buf());

        std::auto_ptr<SmppCommand> cmd( new SmppCommand(pdu) );

        smsc_log_info(mainlog,"--- smpp command created ---");

        smppManager->putCommand( sme1, cmd );

        smsc_log_info(mainlog,"--- smpp command put into smpp queue sme1='%s' ---", sme1.getSystemId());
    }
     */

    // now, we need somehow to wait until the command is processed...
    // timespec ts = { 10, 0};
    // nanosleep(&ts,0);
    // smppManager->shutdown();
    // smsc_log_info(mainlog,"--- smpp manager stopped ---");
    return 0;
}
