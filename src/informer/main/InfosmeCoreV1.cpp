#include <memory>
#include "informer/admin/AdminServer.hpp"
#include "informer/dcp/DcpServer.hpp"
#include "informer/alm/ActivityLogMiner.hpp"
#include "DeliveryMgr.h"
#include "InfosmeCoreV1.h"
#include "RegionLoader.h"
#include "informer/data/UserInfo.h"
#include "informer/data/FinalLog.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/RelockMutexGuard.h"
#include "informer/sender/RegionSender.h"
#include "informer/sender/SmscSender.h"
#include "informer/io/ConfigWrapper.h"
#include "util/config/Config.h"
#include "scag/pvss/api/core/client/impl/ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/GetResponse.h"
#include "core/buffers/FastMTQueue.hpp"

using namespace smsc::util::config;

namespace {

using namespace eyeline::informer;

struct SortUserById
{
    bool operator () ( const UserInfoPtr& a, const UserInfoPtr& b ) const
    {
        return ::strcmp(a->getUserId(), b->getUserId()) < 0;
    }
};

void readSmscConfig( const char*   name,
                     SmscConfig&   cfg,
                     const Config& conf )
{
    std::string logname = std::string("s.") + name;
    const ConfigWrapper config(conf,smsc::logger::Logger::getInstance(logname.c_str()));
    try {
        smsc::sme::SmeConfig& rv = cfg.smeConfig;
        rv.host = config.getString("host","localhost");
        rv.sid = config.getString("sid");
        rv.port = config.getInt("port",0,1,0,false);
        rv.timeOut = config.getInt("timeout",3,1,10);
        cfg.interConnectPeriod = config.getInt("interConnectPeriod",30,10,120);
        rv.password = config.getString("password");
        rv.setSystemType(config.getString("systemType",""));
        rv.interfaceVersion = config.getInt("interfaceVersion",rv.interfaceVersion,1,0);
        rv.setAddressRange(config.getString("rangeOfAddress",""));
        cfg.ussdPushOp = config.getInt("ussdPushTag",-1,1,0);
        cfg.ussdPushVlrOp = config.getInt("ussdPushVlrTag",-1,1,0);
        cfg.minValidityTime = config.getInt("minValidityTime",2*60,60,2*3600);
        cfg.maxValidityTime = config.getInt("maxValidityTime",2*3600,cfg.minValidityTime,3*3600);
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_CONFIG,"S='%s' config exc: %s", name, e.what());
    }
}

} // namespace

namespace eyeline {
namespace informer {

namespace {
using namespace scag2::pvss;
class PvssBlockRequest : public ProfileRequest
{
public:
    PvssBlockRequest( const ProfileKey& pkey,
                      ProfileCommand* cmd,
                      Message& msg,
                      DeliveryActivator::PvssNotifyee& pn ) :
    ProfileRequest(pkey,cmd), msg_(&msg), pn_(pn) {}

    virtual ~PvssBlockRequest() {
    }

    inline Message& getMsg() { return *msg_; }
    inline DeliveryActivator::PvssNotifyee& getPN() { return pn_; }

    virtual PvssBlockRequest* clone() const {
        throw InfosmeException(EXC_NOTIMPL, "pvss block request clone() is forbidden");
    }

private:
    Message* msg_;
    DeliveryActivator::PvssNotifyee& pn_;
};

}


class InfosmeCoreV1::PvssRespHandler :
protected smsc::core::threads::Thread,
public scag2::pvss::core::client::Client::ResponseHandler
{
public:
    struct NM {
        NM() : ipn(0), msg(0) {}
        NM( DeliveryActivator::PvssNotifyee* i, Message* m ) : ipn(i), msg(m) {}
        DeliveryActivator::PvssNotifyee* ipn;
        Message*                         msg;
    };

    PvssRespHandler( InfosmeCoreV1& core ) :
    core_(core), log_(smsc::logger::Logger::getInstance("pvss")),
    stopping_(false) {}

    virtual ~PvssRespHandler() {
        stop();
        WaitFor();
    }

    virtual void start() { Start(); }

    virtual void stop() {
        stopping_ = true;
        queue_.notify();
        WaitFor();
    }


    virtual int Execute()
    {
        NM nm;
        while (!stopping_) {
            queue_.waitForItem();
            while (queue_.Pop(nm)) {
                core_.startPvssCheck(*nm.ipn,*nm.msg);
            }
        }
        while (queue_.Pop(nm)) {
            core_.startPvssCheck(*nm.ipn,*nm.msg);
        }
        return 0;
    }


    virtual void handleResponse( std::auto_ptr<scag2::pvss::Request>  req,
                                 std::auto_ptr<scag2::pvss::Response> resp )
    {
        PvssBlockRequest* pbr = static_cast< PvssBlockRequest* >(req.get());
        if (!pbr) {
            smsc_log_warn(log_,"no req");
            return;
        }
        do {
            if (!resp.get()) {
                smsc_log_warn(log_,"no resp (to be retried)");
                break;
            }
            if (resp->getStatus() == scag2::pvss::StatusType::PROPERTY_NOT_FOUND ) {
                smsc_log_debug(log_,"PVSS property not found, ok");
                pbr->getMsg().timeLeft = 1;
                pbr->getPN().notify();
                return;
            }
            if (resp->getStatus() != scag2::pvss::StatusType::OK ) {
                smsc_log_warn(log_,"resp has bad status=%u (to be retried)",resp->getStatus());
                break;
            }

            GetResponse* grp = static_cast<GetResponse*>
                (static_cast<ProfileResponse*>(resp.get())->getResponse());
            if ( grp->getProperty().getBoolValue() ) {
                // property exists, blocking
                smsc_log_debug(log_,"blocked by pvss");
                pbr->getMsg().timeLeft = 0;
            } else {
                smsc_log_debug(log_,"allowed by pvss");
                pbr->getMsg().timeLeft = 1;
            }
            pbr->getPN().notify();
            return;

        } while (false);
        // resubmit
        queue_.Push(NM(&pbr->getPN(),&pbr->getMsg()));
    }


    virtual void handleError(const scag2::pvss::PvssException& exc,
                             std::auto_ptr<scag2::pvss::Request> req)
    {
        PvssBlockRequest* pbr = static_cast< PvssBlockRequest* >(req.get());
        if (!pbr) {
            smsc_log_warn(log_,"no req");
            return;
        }
        smsc_log_warn(log_,"failed (to be retried) exc: %s",exc.what());
        queue_.Push( NM(&pbr->getPN(),&pbr->getMsg()) );
    }

private:
    InfosmeCoreV1&                               core_;
    smsc::logger::Logger*                        log_;
    smsc::core::buffers::FastMTQueue< NM >       queue_;
    bool                                         stopping_;
};


// =======================================================================

InfosmeCoreV1::InfosmeCoreV1( unsigned maxsms ) :
log_(smsc::logger::Logger::getInstance("core")),
cs_(maxsms),
started_(false),
dlvMgr_(0),
finalLog_(0),
adminServer_(0),
dcpServer_(0),
alm_(0),
pvss_(0),
pvssHandler_(0)
{
}


InfosmeCoreV1::~InfosmeCoreV1()
{
    smsc_log_info(log_,"--- destroying core ---");

    stop();

    if (pvss_) {
        smsc_log_info(log_,"--- destroying pvss client ---");
        delete pvss_;
        delete pvssHandler_;
    }

    if (adminServer_) {
        smsc_log_info(log_,"--- destroying admin server ---");
        delete adminServer_;
    }

    if (dcpServer_) {
        smsc_log_info(log_,"--- destroying dcp server ---");
        delete dcpServer_;
    }

    if (alm_) {
        smsc_log_info(log_,"--- destroying activity log miner ---");
        delete alm_;
    }

    // detaching region senders
    smsc_log_info(log_,"--- destroying region senders ---");
    int regId;
    RegionSenderPtr* regsend;
    for ( IntHash< RegionSenderPtr >::Iterator i(regSends_); i.Next(regId,regsend); ) {
        smsc_log_debug(log_,"detaching regsend RS=%u", regionid_type(regId));
        (*regsend)->assignSender(0);
    }
    regSends_.Empty();

    smsc_log_info(log_,"--- destroying smscs ---");
    char* smscId;
    SmscSender* sender;
    for ( Hash< SmscSender* >::Iterator i(&smscs_); i.Next(smscId,sender); ) {
        smsc_log_debug(log_,"destroying smsc '%s'",smscId);
        delete sender;
    }

    smsc_log_info(log_,"--- destroying delivery mgr ---");
    delete dlvMgr_;

    smsc_log_info(log_,"--- destroying final log ---");
    delete finalLog_;

    smsc_log_info(log_,"--- destroying regions ---");
    regions_.Empty();

    smsc_log_info(log_,"--- destroying users ---");
    users_.Empty();

    smsc_log_info(log_,"--- core destroyed ---");
}


void InfosmeCoreV1::init( bool archive )
{
    smsc_log_info(log_,"--- initing core ---");

    const char* mainfilename = "config.xml";

    const char* section = "";
    const char* filename = mainfilename;

    try {
        std::auto_ptr<Config> maincfg( Config::createFromFile(mainfilename));
        if (!maincfg.get()) {
            throw InfosmeException(EXC_CONFIG,"config file '%s' is not found",mainfilename);
        }

        section = "informer";
        std::auto_ptr<Config> cfg(maincfg->getSubConfig(section,true));

        cs_.init( *cfg, archive );

        itp_.setMaxThreads(cs_.getInputTransferThreadCount());
        rtp_.setMaxThreads(cs_.getResendIOThreadCount());

        if (!archive) { finalLog_ = new FinalLog(); }

        if (!dlvMgr_) {
            smsc_log_info(log_,"--- creating delivery mgr ---");
            dlvMgr_ = new DeliveryMgr(*this,cs_);
        }

        // create admin server
        if (!adminServer_) {
            smsc_log_info(log_,"--- creating admin server ---");
            adminServer_ = new admin::AdminServer();
            // NOTE: moved below
            // adminServer_->assignCore(this);
            adminServer_->Init( cfg->getString("adminHost"),
                                cfg->getInt("adminPort"),
                                cfg->getInt("adminHandlers") );
        }

        // load all users
        smsc_log_info(log_,"--- loading users ---");
        loadUsers("");

        // create smscs
        if (!archive) {
            smsc_log_info(log_,"--- loading smscs ---");
            loadSmscs("");

            // create regions
            smsc_log_info(log_,"--- loading regions ---");
            loadRegions(anyRegionId);

            try {
                // creating pvss client
                smsc_log_info(log_,"--- creating pvss client ---");
                const char* pvssSectName = "pvss";
                if (!maincfg->findSection(pvssSectName)) {
                    throw InfosmeException(EXC_CONFIG,"subsection '%s' is not found",pvssSectName);
                }
                std::auto_ptr<Config> pcfg(maincfg->getSubConfig(pvssSectName,true));
                std::auto_ptr<scag2::pvss::core::client::ClientConfig> 
                    pvssConfig(new scag2::pvss::core::client::ClientConfig);
                ConfigWrapper cwrap(*pcfg.get(),smsc::logger::Logger::getInstance("pvss"));
                pvssConfig->setPort(cwrap.getInt("asyncPort",0,1024,100000,false));
                pvssConfig->setHost(cwrap.getString("host"));
                pvssConfig->setEnabled(cwrap.getBool("enabled",true));
                if ( !pvssConfig->isEnabled() ) {
                    throw InfosmeException(EXC_CONFIG,"disabled in config");
                }

                pvssConfig->setConnectionsCount(cwrap.getInt("connections",3,1,10));
                pvssConfig->setChannelQueueSizeLimit(cwrap.getInt("queueSize",100,50,1000));
                pvssConfig->setPacketSizeLimit(cwrap.getInt("maxPacketSize",1000,100,10000));
                pvssConfig->setIOTimeout(cwrap.getInt("ioTimeout",100,50,1000));
                pvssConfig->setInactivityTime(cwrap.getInt("inactiveTimeout",60000,10000,100000));
                pvssConfig->setConnectTimeout(cwrap.getInt("connectTimeout",60000,1000,100000));
                pvssConfig->setProcessTimeout(cwrap.getInt("processTimeout",500,100,10000));
                pvssConfig->setMaxReaderChannelsCount(cwrap.getInt("connPerThread",5,1,20));
                pvssConfig->setMaxWriterChannelsCount(pvssConfig->getMaxReaderChannelsCount());
                pvssConfig->setReadersCount(cwrap.getInt("ioThreads",2,1,10));
                pvssConfig->setWritersCount(pvssConfig->getReadersCount());
                pvssConfig->setStatisticsInterval(cwrap.getInt("statInterval",60000,5000,3600000));

                std::auto_ptr<scag2::pvss::Protocol> 
                    pvssProto( new scag2::pvss::pvap::PvapProtocol );
                
                pvssHandler_ = new PvssRespHandler(*this);
                pvss_ = new scag2::pvss::core::client::ClientCore( pvssConfig.release(),
                                                                   pvssProto.release() );
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"PvssClient (to be disabled) exc: %s",e.what());
                delete pvssHandler_; pvssHandler_ = 0;
                delete pvss_; pvss_ = 0;
            }
        }

        dlvMgr_->init();

        if (adminServer_) {
            adminServer_->assignCore(this);
        }

        filename = mainfilename;
        section = "informer";

        if (!alm_) {
            alm::ActivityLogMiner* alm;
            alm_ = alm = new alm::ActivityLogMiner();
            alm->init(getCS()->getStorePath(),
                      cfg->getInt("almRequestTimeout"));
        }

        if (!dcpServer_) {
            dcpServer_ = new dcp::DcpServer();
            dcpServer_->assignCore(this);
            dcpServer_->Init( cfg->getString("dcpHost"),
                              cfg->getInt("dcpPort"),
                              cfg->getInt("dcpHandlers") );
        }

    } catch ( InfosmeException& e ) {
        throw;
    } catch ( HashInvalidKeyException& e ) {
        throw InfosmeException(EXC_CONFIG,"exc reading '%s', section '%s': param '%s' not found",
                               filename, section, e.getKey() );
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_CONFIG,"core init exc: %s", e.what());
    }
}


void InfosmeCoreV1::start()
{
    if (started_) return;
    MutexGuard mg(startMon_);
    if (started_) return;
    if (pvssHandler_) { pvssHandler_->start(); }
    if (pvss_) { pvss_->startup(); }
    dlvMgr_->start();
    Start();
    // start all smsc
    char* smscId;
    SmscSender* ptr;
    for ( smsc::core::buffers::Hash<SmscSender*>::Iterator i(&smscs_);
          i.Next(smscId,ptr);) {
        ptr->start();
    }
}


void InfosmeCoreV1::stop()
{
    {
        if (getCS()->isStopping()) return;
        {
            MutexGuard mg(startMon_);
            if (getCS()->isStopping()) return;
            smsc_log_info(log_,"--- stopping core ---");
            cs_.setStopping();
            startMon_.notifyAll();
        }

        bindQueue_.notify();  // wake up bind queue
        itp_.stopNotify();
        rtp_.stopNotify();

        if (dcpServer_) dcpServer_->Stop();
        if (adminServer_) adminServer_->Stop();
        if (dlvMgr_) dlvMgr_->stop();
        if (pvss_) {
            smsc_log_debug(log_,"--- stopping pvss ---");
            pvss_->shutdown();
        }
        if (pvssHandler_) {
            smsc_log_debug(log_,"--- stopping pvss handler ---");
            pvssHandler_->stop();
        }

        // stop all smscs
        char* smscId;
        SmscSender* sender;
        for (Hash< SmscSender* >::Iterator i(&smscs_); i.Next(smscId,sender);) {
            sender->stop();
        }
        smsc_log_debug(log_,"--- waiting for self thread ---");
        WaitFor();
    }
    smsc_log_info(log_,"--- core stopped ---");
}


void InfosmeCoreV1::addUser( const char* user )
{
    smsc_log_debug(log_,"== addUser(%s)",user ? user : "");
    UserInfoPtr ptr = getUserInfo(user);
    if (ptr.get()) {
        throw InfosmeException(EXC_ALREADYEXIST,"user '%s' already exists",user);
    }
    loadUsers(user);
}


void InfosmeCoreV1::deleteUser( const char* login )
{
    smsc_log_debug(log_,"== deleteUser(%s)",login ? login : "");
    if (!login) throw InfosmeException(EXC_LOGICERROR,"deluser NULL passed");
    if ( getCS()->isArchive() ) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    UserInfoPtr user;
    {
        MutexGuard mg(userLock_);
        UserInfoPtr* ptr = users_.GetPtr(login);
        if (ptr) { user = *ptr; }
    }
    if (!user || user->isDeleted() ) {
        throw InfosmeException(EXC_NOTFOUND,"no such user '%s'",login);
    }

    std::vector< DeliveryPtr > dlvs;
    user->getDeliveries( dlvs );
    for ( std::vector< DeliveryPtr >::iterator i = dlvs.begin(); i != dlvs.end(); ++i ) {
        (*i)->setState(DLVSTATE_CANCELLED);
        deleteDelivery(*user,(*i)->getDlvId());
    }
    user->setDeleted(true);
}


UserInfoPtr InfosmeCoreV1::getUserInfo( const char* login )
{
    smsc_log_debug(log_,"== getUserInfo(%s)",login ? login : "");
    if (!login) throw InfosmeException(EXC_LOGICERROR,"userid NULL passed");
    MutexGuard mg(userLock_);
    UserInfoPtr* ptr = users_.GetPtr(login);
    if (!ptr || (*ptr)->isDeleted() ) return UserInfoPtr();
    return *ptr;
}


void InfosmeCoreV1::getUsers( std::vector< UserInfoPtr >& users )
{
    smsc_log_debug(log_,"== getUsers()");
    users.reserve( users.size() + users_.GetCount() + 2 );
    MutexGuard mg(userLock_);
    char* userId;
    UserInfoPtr* user;
    for ( Hash< UserInfoPtr >::Iterator i(&users_); i.Next(userId,user); ) {
        if (user && !(*user)->isDeleted()) {
            users.push_back(*user);
        }
    }
}


void InfosmeCoreV1::updateUserInfo( const char* login )
{
    smsc_log_debug(log_,"== updateUserInfo(%s)",login ? login : "");
    UserInfoPtr ptr = getUserInfo(login);
    if (!ptr.get()) throw InfosmeException(EXC_NOTFOUND,"user '%s' is not found",login);
    loadUsers(login);
}


void InfosmeCoreV1::selfTest()
{
    smsc_log_debug(log_,"--- selfTest started ---");
    if ( getCS()->isArchive() ) {
        // special self test
        return;
    }

    try {

        const char* userId = "selftestuser";
        smsc_log_debug(log_,"--- getting user '%s' ---",userId);
        UserInfoPtr user = getUserInfo(userId);
        if (!user.get()) {
            throw InfosmeException(EXC_NOTFOUND,"U='%s' is not found",userId);
        }

        smsc_log_info(log_,"--- selfTest started ---");

        DeliveryInfoData data;
        {
            data.name = "Моя рассылка";
            data.priority = 1;
            data.transactionMode = false;
            data.startDate = "";
            data.endDate = "";
            data.activePeriodStart = "";
            data.activePeriodEnd = "";
            //data.validityDate = "";
            data.validityPeriod = "01:00:00";
            data.flash = false;
            data.useDataSm = false;
            data.deliveryMode = DLVMODE_SMS;
            data.owner = userId;
            data.retryOnFail = true;
            data.retryPolicy = "1s:*";
            data.replaceMessage = false;
            data.svcType = "info";
            data.userData = "0xdeadbeef";
            data.sourceAddress = "10000";
            data.finalDlvRecords = true;
            data.finalMsgRecords = true;
        }
        smsc_log_info(log_,"--- adding new delivery for U='%s' ---",userId);

        const dlvid_type dlvId = addDelivery(*user, data);
        smsc_log_info(log_,"--- delivery added, D=%u ---", dlvId);

        smsc_log_info(log_,"--- getting delivery D=%u ---", dlvId);
        DeliveryPtr dlv = getDelivery(*user,dlvId);
        if (!dlv.get()) {
            throw InfosmeException(EXC_NOTFOUND,"D=%u is not found",dlvId);
        }

        // adding glossary messages
        {
            smsc_log_info(log_,"--- setting text glossary for D=%u ---", dlvId);
            std::vector< std::string > glotexts;
            glotexts.push_back("русское глоссари #0");
            glotexts.push_back("русское глоссари #1");
            glotexts.push_back("русское глоссари #2");
            glotexts.push_back("русское глоссари #3");
            glotexts.push_back("русское глоссари #4");
            glotexts.push_back("русское глоссари #5");
            glotexts.push_back("русское глоссари #6");
            glotexts.push_back("русское глоссари #7");
            glotexts.push_back("русское глоссари #8");
            glotexts.push_back("русское глоссари #9");
            dlv->setGlossary( glotexts );
        }

        {
            smsc_log_info(log_,"--- adding messages to D=%u ---",dlvId);
            for ( int pass = 0; pass < 5; ++pass ) {
                MessageList msgList;
                if (getCS()->isStopping()) break;
                for ( int j = 0; j < 1000; ++j ) {

                    const int i = pass*1000 + j;
                    
                    ulonglong address;
                    // if ( i % 2 ) {
                    // address = 79130000000ULL + i;
                    // } else {
                        address = 79530000000ULL + i;
                    // }
                    msgList.push_back(MessageLocker());
                    MessageLocker& mlk = msgList.back();
                    mlk.msg.subscriber = addressToSubscriber(11,1,1,address);
                    char userdata[30];
                    sprintf(userdata,"msg#%d",i);
                    mlk.msg.userData = userdata;
                    if ( i / 10 % 2 ) {
                        char msgtext[50];
                        sprintf(msgtext,"русский текст #%u",i);
                        MessageText(msgtext).swap(mlk.msg.text);
                    } else {
                        MessageText(0,i%10).swap(mlk.msg.text);
                    }
                }
                dlv->addNewMessages(msgList.begin(), msgList.end());

                if ( pass == 0 ) {
                    smsc_log_debug(log_,"--- changing delivery D=%u state ---", dlvId);
                    dlv->setState(DLVSTATE_ACTIVE);
                    smsc_log_info(log_,"--- delivery activated ---");
                }
            }
        }

        {
            smsc_log_info(log_,"--- getting text glossary for D=%u ---", dlvId);
            std::vector< std::string > glotexts;
            dlv->getGlossary( glotexts );
        }

        {
            smsc_log_info(log_,"--- pausing the delivery D=%u ---",dlvId);
            dlv->setState(DLVSTATE_PAUSED);
        }

        {
            smsc_log_info(log_,"--- archivating the delivery D=%u ---",dlvId);
            deleteDelivery( *user, dlvId, true );
        }

    } catch ( std::exception& e ) {
        smsc_log_debug(log_,"--- selftest failed, exc: %s",e.what());
    }
    smsc_log_debug(log_,"--- selfTest finished ---");
}


void InfosmeCoreV1::loadRegions( regionid_type regId )
{
    // reading region file
    RegionLoader rl("regions.xml", defaultSmscId_.c_str(), regId );

    MutexGuard mg(startMon_); // guaranteed that there is no sending
    do {
        std::auto_ptr<Region> r(rl.popNext());
        if (!r.get()) break;

        const regionid_type regionId = r->getRegionId();

        // find smscconn
        const std::string& smscId = r->getSmscId();
        SmscSender** smsc = smscs_.GetPtr(smscId.c_str());
        if (!smsc || !*smsc) {
            throw InfosmeException(EXC_CONFIG,"S='%s' is not found for R=%u",smscId.c_str(),regionId);
        }

        RegionPtr* ptr = regions_.GetPtr(regionId);

        // update masks
        rf_.updateMasks(ptr ? ptr->get() : 0, *r.get());

        if (!ptr) {
            smsc_log_debug(log_,"creating R=%u for S='%s'",regionId,smscId.c_str());
            ptr = &regions_.Insert(regionId,RegionPtr(r.release()));
        } else {
            smsc_log_debug(log_,"updating R=%u for S='%s'",regionId,smscId.c_str());
            (*ptr)->swap( *r.get() );
        }

        RegionSenderPtr* rs = regSends_.GetPtr(regionId);
        if (!rs) {
            rs = &regSends_.Insert(regionId,RegionSenderPtr(new RegionSender(**smsc,*ptr)));
        } else {
            (*rs)->assignSender(*smsc);
        }

    } while (true);
}


void InfosmeCoreV1::deliveryRegions( dlvid_type dlvId,
                                     std::vector<regionid_type>& regIds,
                                     bool bind )
{
    smsc_log_debug(log_,"pushing %sbind signal D=%u regions:[%s]",
                   bind ? "" : "un", unsigned(dlvId),
                   formatRegionList(regIds.begin(), regIds.end()).c_str() );
    BindSignal bs;
    bs.dlvId = dlvId;
    bs.regIds.swap(regIds);
    bs.bind = bind;
    bs.ignoreState = bind;
    bindQueue_.Push(bs);
}


void InfosmeCoreV1::finishStateChange( msgtime_type    currentTime,
                                       ulonglong       ymdTime,
                                       BindSignal&     bs,
                                       const Delivery& dlv )
{
    const DlvState newState = dlv.getState();
    if (log_->isDebugEnabled()) {
        smsc_log_debug(log_,"D=%u finish state change, state=%s, bind=%d, regs=[%s]",
                       bs.dlvId, dlvStateToString(newState), bs.bind,
                       formatRegionList(bs.regIds.begin(),bs.regIds.end()).c_str() );
    }
    if (dlv.getDlvInfo().wantFinalDlvRecords() &&
        (newState == DLVSTATE_FINISHED || newState == DLVSTATE_ACTIVE) ) {
        FinalLog::getFinalLog()->addDlvRecord(currentTime,bs.dlvId,
                                              dlv.getUserInfo().getUserId(),
                                              newState );
    }
    dlvMgr_->finishStateChange( currentTime, ymdTime, dlv );
    if ( bs.regIds.empty() ) return;
    bs.ignoreState = false;
    bindDeliveryRegions( bs );
}


void InfosmeCoreV1::startPvssCheck( PvssNotifyee& pn, Message& msg )
{
    if (!pvss_) {
        msg.timeLeft = 1;
        pn.notify();
        return;
    }
    scag2::pvss::PvssException exc;

    int pass = 0;
    // static smsc::core::synchronization::EventMonitor emon;
    while ( true ) {
        if ( getCS()->isStopping() ) {
            msg.timeLeft = 1;
            pn.notify();
            break;
        }
        static smsc::core::synchronization::EventMonitor emon;
        try {

            if ( ! pvss_->canProcessRequest(&exc) ) {
                MutexGuard mg(emon);
                emon.wait(100);
                continue;
            }

            char buf[30];
            uint8_t ton,npi,len;
            const uint64_t addr = subscriberToAddress(msg.subscriber,len,ton,npi);
            sprintf(buf,".%u.%u.%*.*llu",ton,npi,len,len,ulonglong(addr));
            ProfileKey pkey;
            pkey.setAbonentKey(buf);

            scag2::pvss::GetCommand* cmd = new scag2::pvss::GetCommand;
            cmd->setVarName("infosme_black_list");
            std::auto_ptr< scag2::pvss::Request > 
                preq( new PvssBlockRequest(pkey,cmd,msg,pn) );
            pvss_->processRequestAsync(preq,*pvssHandler_);
            break;

        } catch ( std::exception& e ) {
            if (!pass) {
                smsc_log_debug(log_,"PVSS start proc exc: %s", e.what() );
                ++pass;
            }
            MutexGuard mg(emon);
            emon.wait(100);
        }
    }
}


void InfosmeCoreV1::addSmsc( const char* smscId )
{
    smsc_log_debug(log_,"== addSmsc(%s)",smscId ? smscId : "");
    if (!smscId) throw InfosmeException(EXC_LOGICERROR,"empty/null smscId passed");
    if ( getCS()->isArchive() ) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    {
        MutexGuard mg(startMon_);
        SmscSender** ptr = smscs_.GetPtr(smscId);
        if (ptr) {
            throw InfosmeException(EXC_ALREADYEXIST,"smsc '%s' already exists",smscId);
        }
    }
    if (!isGoodAsciiName(smscId)) {
        throw InfosmeException(EXC_BADNAME,"bad smsc name '%s'",smscId);
    }
    loadSmscs(smscId);
}


void InfosmeCoreV1::updateSmsc(const char* smscId)
{
    smsc_log_debug(log_,"== updateSmsc(%s)",smscId ? smscId : "");
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    if (!smscId) throw InfosmeException(EXC_LOGICERROR,"empty/null smscId passed");
    {
        MutexGuard mg(startMon_);
        SmscSender** ptr = smscs_.GetPtr(smscId);
        if (!ptr) {
            throw InfosmeException(EXC_NOTFOUND,"smsc '%s' not found",smscId);
        }
    }
    if (!isGoodAsciiName(smscId)) {
        throw InfosmeException(EXC_BADNAME,"bad smsc name '%s'",smscId);
    }
    loadSmscs(smscId);
}


void InfosmeCoreV1::deleteSmsc( const char* smscId )
{
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    smsc_log_debug(log_,"== deleteSmsc(%s)",smscId ? smscId : "");
    updateSmsc(smscId,0,0);
}


void InfosmeCoreV1::updateDefaultSmsc( const char* smscId )
{
    smsc_log_debug(log_,"== updateDefaultSmsc(%s)",smscId ? smscId : "");
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    MutexGuard mg(startMon_);
    if (!smscId || !smscId[0] || !isGoodAsciiName(smscId)) {
        throw InfosmeException(EXC_BADNAME,"invalid default smsc name '%s'",smscId ? smscId : "");
    }
    if (smscId == defaultSmscId_) return;
    SmscSender** ptr = smscs_.GetPtr(smscId);
    if (!ptr) {
        throw InfosmeException(EXC_NOTFOUND,"smsc '%s' not found",smscId);
    }
    RegionPtr* rptr = regions_.GetPtr(0);
    if (!rptr) {
        throw InfosmeException(EXC_NOTFOUND,"default region is not found");
    }
    RegionSenderPtr* regptr = regSends_.GetPtr(0);
    if (!regptr) {
        throw InfosmeException(EXC_LOGICERROR,"default RS is not found");
    }
    defaultSmscId_ = smscId;
    (*rptr)->setSmscId(smscId);
    (*regptr)->assignSender(*ptr);
}


void InfosmeCoreV1::addRegion( regionid_type regionId )
{
    smsc_log_debug(log_,"== addRegion(R=%u)",regionId);
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    if (regionId == anyRegionId) {
        throw InfosmeException(EXC_LOGICERROR,"invalid regionid=%u invoked",regionId);
    }
    // check if region does not exist
    {
        MutexGuard mg(startMon_);
        RegionPtr* ptr = regions_.GetPtr( regionId );
        if (ptr) {
            throw InfosmeException(EXC_ALREADYEXIST,"region %u/'%s' already exist",
                                   regionId, (*ptr)->getName().c_str() );
        }
    }
    loadRegions( regionId );
}


void InfosmeCoreV1::updateRegion( regionid_type regionId )
{
    smsc_log_debug(log_,"== updateRegion(R=%u)",regionId);
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    if (regionId == anyRegionId) {
        throw InfosmeException(EXC_LOGICERROR,"invalid regionid=%u invoked",regionId);
    }
    // check if region exists
    {
        MutexGuard mg(startMon_);
        RegionPtr* ptr = regions_.GetPtr(regionId);
        if (!ptr || !*ptr ) {
            throw InfosmeException(EXC_NOTFOUND,"region %u/'%s' not found",
                                   regionId, (*ptr)->getName().c_str() );
        } else if ( (*ptr)->isDeleted() ) {
            throw InfosmeException(EXC_NOTFOUND,"region %u/'%s' is already deleted",
                                   regionId, (*ptr)->getName().c_str() );
        }
    }
    loadRegions(regionId);
}


void InfosmeCoreV1::deleteRegion( regionid_type regionId )
{
    smsc_log_debug(log_,"== deleteRegion(R=%u)",regionId);
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    if (regionId == anyRegionId || regionId == defaultRegionId ) {
        throw InfosmeException(EXC_LOGICERROR,"invalid regionid=%u invoked",regionId);
    }
    {
        MutexGuard mg(startMon_);
        RegionPtr* ptr = regions_.GetPtr(regionId);
        if (!ptr || !*ptr ) {
            throw InfosmeException(EXC_NOTFOUND,"region %u/'%s' not found",
                                   regionId, (*ptr)->getName().c_str() );
        } else if ( (*ptr)->isDeleted() ) {
            throw InfosmeException(EXC_NOTFOUND,"region %u/'%s' is already deleted",
                                   regionId, (*ptr)->getName().c_str() );
        }
        (*ptr)->setDeleted(true);
        rf_.updateMasks( ptr->get(), **ptr );
    }
}


dlvid_type InfosmeCoreV1::addDelivery( UserInfo& userInfo,
                                       const DeliveryInfoData& info )
{
    smsc_log_debug(log_,"== addDelivery(U='%s')",userInfo.getUserId());
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    return dlvMgr_->createDelivery(userInfo,info);
}


void InfosmeCoreV1::deleteDelivery( const UserInfo& userInfo,
                                    dlvid_type      dlvId,
                                    bool            moveToArchive )
{
    smsc_log_debug(log_,"== deleteDelivery(U='%s',D=%u)",userInfo.getUserId(),dlvId);
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    BindSignal bs;
    bs.ignoreState = bs.bind = false;
    bs.dlvId = dlvId;

    DeliveryImplPtr ptr;
    if ( !dlvMgr_->getDelivery(dlvId,ptr) ) {
        throw InfosmeException(EXC_NOTFOUND,"no such delivery %u",dlvId);
    }
    if ( &(ptr->getUserInfo()) != &userInfo &&
         !userInfo.hasRole(USERROLE_ADMIN)) {
        throw InfosmeException(EXC_ACCESSDENIED,"access denied to delivery %u",dlvId);
    }
    if ( moveToArchive &&
         ptr->getState() == DLVSTATE_ACTIVE ) {
        throw InfosmeException(EXC_ACCESSDENIED,"cannot archivate an active delivery %u",dlvId);
    }
    dlvMgr_->deleteDelivery(dlvId,bs.regIds,moveToArchive);
    bindDeliveryRegions(bs);
}


DeliveryPtr InfosmeCoreV1::getDelivery( const UserInfo& userInfo,
                                        dlvid_type      dlvId )
{
    smsc_log_debug(log_,"== getDelivery(U='%s',D=%u)",userInfo.getUserId(),dlvId);
    DeliveryImplPtr ptr;
    if (!dlvMgr_->getDelivery(dlvId,ptr)) {
        throw InfosmeException(EXC_NOTFOUND,"no such delivery %u",dlvId);
    }
    if ( &(ptr->getUserInfo()) != &userInfo &&
         !userInfo.hasRole(USERROLE_ADMIN)) {
        throw InfosmeException(EXC_ACCESSDENIED,"access denied to delivery %u",dlvId);
    }
    return ptr;
}


int InfosmeCoreV1::sendTestSms( const char*        sourceAddr,
                                personid_type      subscriber,
                                const char*        text,
                                bool               isFlash,
                                DlvMode            deliveryMode )
{
    // find region
    if ( getCS()->isArchive()) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    MutexGuard mg(startMon_);
    const regionid_type rId = rf_.findRegion(subscriber);
    RegionPtr* region = regions_.GetPtr(rId);
    if (!region) {
        uint8_t len, ton, npi;
        const uint64_t addr = subscriberToAddress(subscriber,len,ton,npi);
        throw InfosmeException(EXC_NOTFOUND,"Region %u corresponding to .%u.%u.%*.*llu is not found",
                               rId, ton, npi, len, len, ulonglong(addr));
    }
    const std::string smscId = region->get()->getSmscId();
    smsc_log_debug(log_,"R=%u is connected to S='%s'", rId, smscId.c_str());
    SmscSender** sender = smscs_.GetPtr(smscId.c_str());
    if ( !sender ) {
        throw InfosmeException(EXC_NOTFOUND,"Smsc '%s' is not found",smscId.c_str());
    }
    return (*sender)->sendTestSms( sourceAddr,
                                   subscriber,
                                   text,
                                   isFlash,
                                   deliveryMode );
}


int InfosmeCoreV1::Execute()
{
    {
        MutexGuard mg(startMon_);
        started_ = true;
    }
    smsc_log_info(log_,"starting main loop");
    while ( !getCS()->isStopping() ) {

        BindSignal bs;
        while ( bindQueue_.Pop(bs) ) {
            if (bs.regIds.empty()) continue;
            bindDeliveryRegions(bs);
        }
        bindQueue_.waitForItem();

    }
    smsc_log_debug(log_,"finishing main loop");
    MutexGuard mg(startMon_);
    started_ = false;
    return 0;
}


void InfosmeCoreV1::bindDeliveryRegions( const BindSignal& bs )
{
    if (bs.regIds.empty()) return;

    typedef std::vector<regionid_type> regIdVector;
    smsc_log_debug(log_,"%sbinding D=%u with [%s]",
                   bs.bind ? "" : "un", unsigned(bs.dlvId),
                   formatRegionList(bs.regIds.begin(),bs.regIds.end()).c_str());

    std::vector< RegionSenderPtr > rsp;
    if (! bs.bind) {
        // unbind from senders
        rsp.reserve(bs.regIds.size());
        {
            MutexGuard mg(startMon_);
            for (regIdVector::const_iterator i = bs.regIds.begin();
                 i != bs.regIds.end(); ++i) {
                RegionSenderPtr* rs = regSends_.GetPtr(*i);
                if (!rs || !rs->get()) {
                    smsc_log_warn(log_,"RS=%u is not found",unsigned(*i));
                    continue;
                }
                rsp.push_back(*rs);
            }
        }
        for ( std::vector<RegionSenderPtr>::iterator i = rsp.begin();
              i != rsp.end(); ++i ) {
            (*i)->removeDelivery(bs.dlvId);
        }

        // get the delivery
        DeliveryImplPtr dlv;
        if (!dlvMgr_->getDelivery(bs.dlvId,dlv)) { return; }
        if ( dlv->getState() == DLVSTATE_ACTIVE) {
            smsc_log_debug(log_,"D=%u checking finalize",bs.dlvId);
            dlv->checkFinalize();
        }
        return;
    }

    DeliveryImplPtr dlv;
    if (!dlvMgr_->getDelivery(bs.dlvId,dlv)) {
        throw InfosmeException(EXC_NOTFOUND,"D=%u is not found",bs.dlvId);
    }

    // check state again
    if (!bs.ignoreState && dlv->getState() != DLVSTATE_ACTIVE) {
        smsc_log_warn(log_,"D='%u' trying to bind regions of inactive delivery",bs.dlvId);
        return;
    }

    rsp.reserve(bs.regIds.size());
    for ( regIdVector::const_iterator i = bs.regIds.begin();
          i != bs.regIds.end(); ++i ) {
        MutexGuard mg(startMon_);
        RegionPtr* ptr = regions_.GetPtr(*i);
        if (!ptr || !ptr->get()) {
            // no such region
            smsc_log_warn(log_,"R=%u is not found",unsigned(*i));
            continue;
        }
        RegionSenderPtr* rs = regSends_.GetPtr(*i);
        if (!rs || !rs->get()) {
            // no such region sender
            smsc_log_warn(log_,"RS=%u is not found",unsigned(*i));
            continue;
        }
        rsp.push_back(*rs);
    }

    for ( std::vector< RegionSenderPtr >::iterator i = rsp.begin();
          i != rsp.end(); ++i ) {
        const regionid_type regId = (*i)->getRegionId();
        RegionalStoragePtr rptr = dlv->getRegionalStorage(regId,true);
        if (!rptr.get()) {
            smsc_log_warn(log_,"D=%u cannot create R=%u",unsigned(bs.dlvId),unsigned(regId));
            continue;
        }
        (*i)->addDelivery(*rptr.get());
    }
}


void InfosmeCoreV1::initUserStats()
{
    char* userid;
    UserInfoPtr* ptr;
    UserDlvStats ds;
    MutexGuard mg(userLock_);
    for ( smsc::core::buffers::Hash<UserInfoPtr>::Iterator i(&users_); i.Next(userid,ptr); ) {
        ptr->get()->popDlvStats(ds);
    }
    smsc_log_debug(log_,"user stats cleared");
}


void InfosmeCoreV1::dumpUserStats( msgtime_type currentTime )
{
    FileGuard fg;
    char buf[200];
    const ulonglong ymd = msgTimeToYmd(currentTime);
    char* bufpos = buf + sprintf(buf,"%04u,",unsigned(ymd % 10000));

    std::vector< UserInfoPtr > users;
    {
        MutexGuard mg(userLock_);
        users.reserve(users_.GetCount());
        char* userid;
        UserInfoPtr* ptr;
        for ( smsc::core::buffers::Hash<UserInfoPtr>::Iterator i(&users_); i.Next(userid,ptr); ) {
            users.push_back(*ptr);
        }
    }
    // sorting users
    std::sort( users.begin(), users.end(), ::SortUserById() );
    for ( std::vector< UserInfoPtr >::iterator i = users.begin();
          i != users.end(); ++i ) {
        UserDlvStats ds;
        (*i)->popDlvStats(ds);
        if ( ds.isEmpty() ) continue;
        if (!fg.isOpened()) {
            char fpath[100];
            const unsigned dayhour = unsigned(ymd/10000);
            sprintf(fpath,"%04u.%02u.%02u/dlv%02u.log",
                    dayhour / 1000000,
                    dayhour / 10000 % 100,
                    dayhour / 100 % 100,
                    dayhour % 100);
            fg.create((getCS()->getStatPath()+fpath).c_str(),0666,true);
            fg.seek(0,SEEK_END);
            if (fg.getPos()==0) {
                const char* header = "#1 MINSEC,USER,PAUSED,PLANNED,ACTIVE,FINISH,CANCEL,CREATED,DELETED\n";
                fg.write(header,strlen(header));
            }
        }
        char* p = bufpos + sprintf(bufpos,"%s,%u,%u,%u,%u,%u,%u,%u\n",
                                   (*i)->getUserId(),
                                   ds.paused,
                                   ds.planned,
                                   ds.active,
                                   ds.finished,
                                   ds.cancelled,
                                   ds.created,
                                   ds.deleted );
        fg.write(buf,p-buf);
    }
}


void InfosmeCoreV1::loadUsers( const char* userId )
{
    std::vector< UserInfoPtr > uservec;
    try {
        if (!userId) throw InfosmeException(EXC_LOGICERROR,"loadUsers: NULL passed");
        if (userId[0] == '\0') {
            smsc_log_info(log_,"loading all users");
        } else {
            smsc_log_info(log_,"loading user '%s'",userId);
        }
        if (userId[0] != '\0' && !isGoodAsciiName(userId)) {
            throw InfosmeException(EXC_BADNAME,"invalid userId '%s'",userId);
        }
        std::auto_ptr<Config> users(Config::createFromFile("users.xml"));
        users.reset( users->getSubConfig("USERS",true) );
        if (!users.get()) {
            throw InfosmeException(EXC_CONFIG,"config has no section USERS");
        }
        std::auto_ptr<CStrSet> sections;
        if (userId[0] != '\0') {
            sections.reset(new CStrSet);
            sections->insert(userId);
        } else {
            sections.reset( users->getRootSectionNames());
            if (!sections.get()) {
                smsc_log_warn(log_,"no users in users.xml");
                return;
            }
            uservec.reserve( sections->size() );
        }
        smsc_log_debug(log_,"trying to load %u users",sections->size());
        for ( CStrSet::iterator i = sections->begin(); i != sections->end(); ++i ) {
            if (!isGoodAsciiName(i->c_str())) {
                throw InfosmeException(EXC_BADNAME,"invalid user '%s'",i->c_str());
            }
            std::auto_ptr<Config> uc(users->getSubConfig(i->c_str(),true));
            if (!uc.get()) {
                throw InfosmeException(EXC_CONFIG,"user '%s' is not found",i->c_str());
            }
            smsc_log_debug(log_,"trying to load U='%s'",i->c_str());
            std::auto_ptr<Config> roles;
            try { roles.reset(uc->getSubConfig("ROLES",true)); } catch (...) {}
            // reading user
            ConfigWrapper cwrap(*uc,log_);
            const unsigned priority = cwrap.getInt("priority",1,1,100);
            const unsigned speed = cwrap.getInt("smsPerSec",1,1,1000);
            const unsigned totaldlv = cwrap.getInt("totalDeliveries",100,0,20000);
            const std::string password = cwrap.getString("password");
            uservec.push_back(UserInfoPtr(new UserInfo(i->c_str(),
                                                       password.c_str(),
                                                       priority,
                                                       speed,
                                                       totaldlv )));
            UserInfoPtr& user = uservec.back();
            if (roles.get()) {
                ConfigWrapper rolecfg(*roles,log_);
                if ( rolecfg.getBool("informer-admin",false) ) {
                    user->addRole(USERROLE_ADMIN);
                }
                if ( rolecfg.getBool("informer-user",true) ) {
                    user->addRole(USERROLE_USER);
                }
            } else {
                user->addRole(USERROLE_USER);
            }
        }
    } catch ( InfosmeException& ) {
        throw;
    } catch (std::exception& e) {
        throw InfosmeException(EXC_CONFIG,"loadUsers('%s'), exc: %s",userId,e.what());
    }

    smsc_log_info(log_,"users.xml has been read (%u users), applying",unsigned(uservec.size()));
    MutexGuard mg(userLock_);
    for ( std::vector<UserInfoPtr>::iterator i = uservec.begin(); i != uservec.end(); ++i ) {
        UserInfoPtr* olduser = users_.GetPtr((*i)->getUserId());
        if (!olduser) {
            users_.Insert((*i)->getUserId(),*i);
            const UserInfo& u = **i;
            smsc_log_info(log_,"new user U='%s' added: maxdlv=%u prio=%u speed=%u isadmin=%u",
                          u.getUserId(),
                          u.getMaxTotalDeliveries(),
                          u.getPriority(),
                          u.getSpeed(),
                          u.hasRole(USERROLE_ADMIN));
        } else {
            if ( (*olduser)->isDeleted() ) {
                smsc_log_info(log_,"deleted user U='%s' is restored");
                (*olduser)->setDeleted(false);
            }
            (*olduser)->update(**i);
            const UserInfo& u = **i;
            smsc_log_info(log_,"user U='%s' updated: maxdlv=%u prio=%u speed=%u isadmin=%u",
                          u.getUserId(),
                          u.getMaxTotalDeliveries(),
                          u.getPriority(),
                          u.getSpeed(),
                          u.hasRole(USERROLE_ADMIN));
        }
    }
}


void InfosmeCoreV1::loadSmscs( const char* smscId )
{
    const char* smscfilename = "smsc.xml";
    const char* section = "SMSCConnectors";
    try {
        std::auto_ptr< Config > scfg( Config::createFromFile(smscfilename));
        if (!scfg.get()) {
            throw InfosmeException(EXC_CONFIG,"config file '%s' is not found",smscfilename);
        }
        scfg.reset( scfg->getSubConfig(section,true) );

        std::auto_ptr< CStrSet > connNames;
        if (smscId && smscId[0]) {
            if (!scfg->findSection(smscId)) {
                throw InfosmeException(EXC_CONFIG,"smsc '%s' not found in config",smscId);
            }
            connNames.reset(new CStrSet);
            connNames->insert(smscId);
        } else {
            connNames.reset(scfg->getRootSectionNames());
            const char* defConn = scfg->getString("default");
            if ( connNames->find(defConn) == connNames->end() ) {
                throw InfosmeException(EXC_CONFIG,"default SMSC '%s' does not match any section",defConn);
            }
            defaultSmscId_ = defConn;
        }

        for ( CStrSet::iterator i = connNames->begin(); i != connNames->end(); ++i ) {
            smsc_log_info(log_,"processing smsc S='%s'",i->c_str());
            std::auto_ptr< Config > sect(scfg->getSubConfig(i->c_str(),true));
            SmscConfig smscConfig;
            readSmscConfig(i->c_str(), smscConfig, *sect.get());
            std::auto_ptr< Config > retryConfig
                (sect->getSubConfig("retryPolicies",true));
            updateSmsc( i->c_str(), &smscConfig, retryConfig.get() );
        }

    } catch ( InfosmeException& e ) {
        throw;
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_CONFIG,"loadSmsc('%s') exc: %s",smscId,e.what());
    }
}


void InfosmeCoreV1::updateSmsc( const char*       smscId,
                                const SmscConfig* cfg,
                                Config*           retryConfig )
{
    if (cfg) {
        // create/update
        MutexGuard mg(startMon_);
        SmscSender** ptr = smscs_.GetPtr(smscId);
        if (!ptr) {
            ptr = smscs_.SetItem(smscId,
                                 new SmscSender(*dlvMgr_,smscId,
                                                *cfg,
                                                retryConfig));
        } else if (*ptr) {
            (*ptr)->updateConfig(*cfg,retryConfig);
            // (*ptr)->waitUntilReleased();
        } else {
            *ptr = new SmscSender(*dlvMgr_,smscId,*cfg,retryConfig);
        }
        if (ptr && *ptr && started_) {
            (*ptr)->start();
        }
    } else {
        // delete the smsc
        SmscSender* ptr = 0;
        {
            MutexGuard mg(startMon_);
            if (!smscs_.Pop(smscId,ptr)) {
                throw InfosmeException(EXC_NOTFOUND,"smsc '%s' is not found",smscId);
            }
            ptr->stop();
            std::vector< regionid_type > regIds;
            ptr->getRegionList(regIds);
            for ( std::vector< regionid_type >::const_iterator i = regIds.begin();
                  i != regIds.end(); ++i ) {
                smsc_log_debug(log_,"removing RS=%u for S='%s' from core", *i, smscId);
                regSends_.Delete(*i);
            }
        }
        delete ptr;
    }
}


}
}
