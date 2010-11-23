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
        cfg.minValidityTime = config.getInt("minValidityTime",10*60,60,2*3600);
        cfg.maxValidityTime = config.getInt("maxValidityTime",2*3600,cfg.minValidityTime,3*3600);
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_CONFIG,"S='%s' config exc: %s", name, e.what());
    }
}

} // namespace

namespace eyeline {
namespace informer {

InfosmeCoreV1::InfosmeCoreV1() :
log_(smsc::logger::Logger::getInstance("core")),
stopping_(false),
started_(false),
dlvMgr_(0),
finalLog_(0),
adminServer_(0),
dcpServer_(0),
alm_(0)
{
}


InfosmeCoreV1::~InfosmeCoreV1()
{
    smsc_log_info(log_,"--- destroying core ---");

    stop();

    smsc_log_info(log_,"--- destroying admin server ---");
    delete adminServer_;

    smsc_log_info(log_,"--- destroying dcp server ---");
    delete dcpServer_;

    smsc_log_info(log_,"--- destroying activity log miner ---");
    delete alm_;

    // detaching region senders
    smsc_log_info(log_,"--- destroying region senders ---");
    int regId;
    RegionSenderPtr* regsend;
    for ( IntHash< RegionSenderPtr >::Iterator i(regSends_); i.Next(regId,regsend); ) {
        smsc_log_debug(log_,"detaching regsend RS=%u", regionid_type(regId));
        (*regsend)->assignSender(0);
    }
    smsc_log_debug(log_,"removing all regsends");
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


void InfosmeCoreV1::init()
{
    smsc_log_info(log_,"--- initing core ---");

    const char* mainfilename = "config.xml";

    const char* section = "";
    const char* filename = mainfilename;

    try {
        std::auto_ptr<Config> cfg( Config::createFromFile(mainfilename));
        if (!cfg.get()) {
            throw InfosmeException(EXC_CONFIG,"config file '%s' is not found",mainfilename);
        }

        section = "informer";
        cfg.reset( cfg->getSubConfig(section,true) );

        cs_.init( *cfg );

        itp_.setMaxThreads(cs_.getInputTransferThreadCount());
        rtp_.setMaxThreads(cs_.getResendIOThreadCount());

        finalLog_ = new FinalLog();

        if (!dlvMgr_) {
            smsc_log_info(log_,"--- creating delivery mgr ---");
            dlvMgr_ = new DeliveryMgr(*this,cs_);
        }

        // create admin server
        if (!adminServer_) {
            smsc_log_info(log_,"--- creating admin server ---");
            adminServer_ = new admin::AdminServer();
            adminServer_->assignCore(this);
            adminServer_->Init( cfg->getString("adminHost"),
                                cfg->getInt("adminPort"),
                                cfg->getInt("adminHandlers") );
        }

        // load all users
        smsc_log_info(log_,"--- loading users ---");
        loadUsers("");

        // create smscs
        smsc_log_info(log_,"--- loading smscs ---");
        loadSmscs("");

        // create regions
        smsc_log_info(log_,"--- loading regions ---");
        loadRegions(anyRegionId);

        dlvMgr_->init();

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
        if (stopping_) return;
        {
            MutexGuard mg(startMon_);
            if (stopping_) return;
            smsc_log_info(log_,"--- stopping core ---");
            stopping_ = true;
            startMon_.notifyAll();
        }

        bindQueue_.notify();  // wake up bind queue
        itp_.stopNotify();
        rtp_.stopNotify();

        if (dcpServer_) dcpServer_->Stop();
        if (adminServer_) adminServer_->Stop();
        if (dlvMgr_) dlvMgr_->stop();

        // stop all smscs
        char* smscId;
        SmscSender* sender;
        for (Hash< SmscSender* >::Iterator i(&smscs_); i.Next(smscId,sender);) {
            sender->stop();
        }
        WaitFor();
    }
    smsc_log_info(log_,"--- core stopped ---");
}


void InfosmeCoreV1::addUser( const char* user )
{
    smsc_log_debug(log_,"== addUser(%s)",user ? user : "");
    UserInfoPtr ptr = getUserInfo(user);
    if (ptr.get()) throw InfosmeException(EXC_ALREADYEXIST,"user '%s' already exists",user);
    loadUsers(user);
}


void InfosmeCoreV1::deleteUser( const char* login )
{
    smsc_log_debug(log_,"== deleteUser(%s)",login ? login : "");
    if (!login) throw InfosmeException(EXC_LOGICERROR,"deluser NULL passed");
    UserInfoPtr user;
    {
        MutexGuard mg(userLock_);
        users_.Pop(login,user);
    }
    if (!user.get()) throw InfosmeException(EXC_NOTFOUND,"no such user '%s'",login);

    std::vector< DeliveryPtr > dlvs;
    user->getDeliveries( dlvs );
    for ( std::vector< DeliveryPtr >::iterator i = dlvs.begin(); i != dlvs.end(); ++i ) {
        (*i)->setState(DLVSTATE_CANCELLED);
        deleteDelivery(*user,(*i)->getDlvId());
    }
    smsc_log_warn(log_,"U='%s' FIXME should we dump stats of the user after all dlvs stopped?",login);
}


UserInfoPtr InfosmeCoreV1::getUserInfo( const char* login )
{
    smsc_log_debug(log_,"== getUserInfo(%s)",login ? login : "");
    if (!login) throw InfosmeException(EXC_LOGICERROR,"userid NULL passed");
    MutexGuard mg(userLock_);
    UserInfoPtr* ptr = users_.GetPtr(login);
    if (!ptr) return UserInfoPtr();
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
        users.push_back(*user);
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
    try {

        const char* userId = "selftestuser";
        smsc_log_debug(log_,"--- getting user '%s' ---",userId);
        UserInfoPtr user = getUserInfo(userId);
        if (!user.get()) {
            throw InfosmeException(EXC_NOTFOUND,"U='%s' is not found",userId);
        }

        DeliveryInfoData data;
        {
            data.name = "testdlv";
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
            data.retryPolicy = "";
            data.replaceMessage = false;
            data.svcType = "info";
            data.userData = "0xdeadbeef";
            data.sourceAddress = "10000";
            data.finalDlvRecords = true;
            data.finalMsgRecords = true;
        }
        smsc_log_debug(log_,"--- adding new delivery for U='%s' ---",userId);

        const dlvid_type dlvId = addDelivery(*user, data);
        smsc_log_debug(log_,"--- delivery added, D=%u ---", dlvId);

        smsc_log_debug(log_,"--- getting delivery D=%u ---", dlvId);
        DeliveryPtr dlv = getDelivery(*user,dlvId);
        if (!dlv.get()) {
            throw InfosmeException(EXC_NOTFOUND,"D=%u is not found",dlvId);
        }

        // adding glossary messages
        {
            smsc_log_debug(log_,"--- setting text glossary for D=%u ---", dlvId);
            std::vector< std::string > glotexts;
            glotexts.push_back("the first message");
            glotexts.push_back("the second message");
            dlv->setGlossary( glotexts );
        }

        {
            smsc_log_debug(log_,"--- adding messages to D=%u ---",dlvId);
            MessageList msgList;
            MessageLocker mlk;
            for ( int i = 0; i < 10000; ++i ) {
                ulonglong address;
                if ( i % 2 ) {
                    address = 79137650000ULL + i;
                } else {
                    address = 79537690000ULL + i;
                }
                mlk.msg.subscriber = addressToSubscriber(11,1,1,address);
                char userdata[30];
                char msgtext[50];
                sprintf(msgtext,"the unbound text #%u",i);
                MessageText(msgtext).swap(mlk.msg.text);
                sprintf(userdata,"msg#%d",i);
                mlk.msg.userData = userdata;
                msgList.push_back(mlk);
            }
            dlv->addNewMessages(msgList.begin(), msgList.end());
        }

        {
            smsc_log_debug(log_,"--- getting text glossary for D=%u ---", dlvId);
            std::vector< std::string > glotexts;
            dlv->getGlossary( glotexts );
        }

        /*
        {
            smsc_log_debug(log_,"--- dropping messages for D=%u ---", dlvId);
            std::vector<msgid_type> msgIds;
            for ( int i = 0; i < 5000; ++i ) {
                msgIds.push_back(i+1);
            }
            dlv->dropMessages(msgIds);
        }
         */

        {
            smsc_log_debug(log_,"--- changing delivery D=%u state ---", dlvId);
            dlv->setState(DLVSTATE_ACTIVE);
            smsc_log_debug(log_,"--- delivery activated ---");
        }

        /*
        {
            const char* smscId = "selftestsmsc";
            smsc_log_debug(log_,"--- destroying smsc '%s' ---", smscId);
            this->deleteSmsc(smscId);
        }
         */

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
        finalLog_->addDlvRecord(currentTime,bs.dlvId,
                                dlv.getUserInfo().getUserId(),
                                newState );
    }
    dlvMgr_->finishStateChange( currentTime, ymdTime, dlv );
    if ( bs.regIds.empty() ) return;
    bs.ignoreState = false;
    bindDeliveryRegions( bs );
}


void InfosmeCoreV1::addSmsc( const char* smscId )
{
    smsc_log_debug(log_,"== addSmsc(%s)",smscId ? smscId : "");
    if (!smscId) throw InfosmeException(EXC_LOGICERROR,"empty/null smscId passed");
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
    smsc_log_debug(log_,"== deleteSmsc(%s)",smscId ? smscId : "");
    updateSmsc(smscId,0);
}


void InfosmeCoreV1::updateDefaultSmsc( const char* smscId )
{
    smsc_log_debug(log_,"== updateDefaultSmsc(%s)",smscId ? smscId : "");
    MutexGuard mg(startMon_);
    if (!smscId || !smscId[0] || !isGoodAsciiName(smscId)) {
        throw InfosmeException(EXC_BADNAME,"invalid default smsc name '%s'",smscId ? smscId : "");
    }
    if (smscId == defaultSmscId_) return;
    SmscSender** ptr = smscs_.GetPtr(smscId);
    if (!ptr) {
        throw InfosmeException(EXC_NOTFOUND,"smsc '%s' not found",smscId);
    }
    RegionSenderPtr* regptr = regSends_.GetPtr(0);
    if (!regptr) {
        throw InfosmeException(EXC_LOGICERROR,"default RS is not found");
    }
    defaultSmscId_ = smscId;
    (*regptr)->assignSender(*ptr);
}


void InfosmeCoreV1::addRegion( regionid_type regionId )
{
    smsc_log_debug(log_,"== addRegion(R=%u)",regionId);
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
    return dlvMgr_->createDelivery(userInfo,info);
}


void InfosmeCoreV1::deleteDelivery( const UserInfo& userInfo,
                                    dlvid_type      dlvId )
{
    smsc_log_debug(log_,"== deleteDelivery(U='%s',D=%u)",userInfo.getUserId(),dlvId);
    BindSignal bs;
    bs.ignoreState = bs.bind = false;
    bs.dlvId = dlvId;
    // FIXME: check userinfo
    dlvMgr_->deleteDelivery(dlvId,bs.regIds);
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


int InfosmeCoreV1::Execute()
{
    {
        MutexGuard mg(startMon_);
        started_ = true;
    }
    smsc_log_info(log_,"starting main loop");
    while ( !stopping_ ) {

        BindSignal bs;
        while ( bindQueue_.Pop(bs) ) {
            if (bs.regIds.empty()) continue;
            bindDeliveryRegions(bs);
        }
        bindQueue_.waitForItem();

    }
    smsc_log_info(log_,"finishing main loop");
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


void InfosmeCoreV1::dumpUserStats( msgtime_type currentTime )
{
    FileGuard fg;
    char buf[200];
    char* bufpos;
    {
        struct tm now;
        const ulonglong ymd = msgTimeToYmd(currentTime,&now);
        sprintf(buf,"%04u.%02u.%02u/dlv%02u.log",
                now.tm_year + 1900, now.tm_mon+1, now.tm_mday, now.tm_hour );
        fg.create((getCS()->getStatPath()+buf).c_str(),0666,true);
        fg.seek(0,SEEK_END);
        if (fg.getPos()==0) {
            const char* header = "#1 MINSEC,USER,PAUSED,PLANNED,ACTIVE,FINISH,CANCEL\n";
            fg.write(header,strlen(header));
        }
        bufpos = buf + sprintf(buf,"%04u,",unsigned(ymd%10000));
    }
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
        (*i)->popIncrementalStats(ds);
        if ( ds.isEmpty() ) continue;
        char* p = bufpos + sprintf(bufpos,"%s,%u,%u,%u,%u,%u\n",
                                   (*i)->getUserId(),
                                   ds.paused,
                                   ds.planned,
                                   ds.active,
                                   ds.finished,
                                   ds.cancelled );
        fg.write(buf,p-buf);
    }
}


void InfosmeCoreV1::loadUsers( const char* userId )
{
    // FIXME: optimize smsc::util::config to load only one section
    std::vector< UserInfoPtr > uservec;
    try {
        if (!userId) throw InfosmeException(EXC_LOGICERROR,"loadUsers: NULL passed");
        smsc_log_info(log_,"loading user(s) '%s'",userId);
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
            unsigned priority = 1;
            try { priority = uc->getInt("priority"); } catch (...) {}
            unsigned speed = 1;
            try { speed = uc->getInt("smsPerSec"); } catch (...) {}
            const unsigned totaldlv = 100;
            uservec.push_back(UserInfoPtr(new UserInfo(i->c_str(),
                                                       uc->getString("password"),
                                                       priority,
                                                       speed,
                                                       totaldlv )));
            UserInfoPtr& user = uservec.back();
            if (roles.get()) {
                try {
                    if (uc->getBool("informer-admin")) {
                        user->addRole(USERROLE_ADMIN);
                    }
                } catch (...) {}
                try {
                    if (uc->getBool("informer-user")) {
                        user->addRole(USERROLE_USER);
                    }
                } catch (...) {}
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
            smsc_log_info(log_,"new user U='%s' added",(*i)->getUserId());
        } else {
            (*olduser)->update(**i);
            smsc_log_info(log_,"user U='%s' updated",(*i)->getUserId());
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
            updateSmsc( i->c_str(), &smscConfig );
        }

    } catch ( InfosmeException& e ) {
        throw;
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_CONFIG,"loadSmsc('%s') exc: %s",smscId,e.what());
    }
}


void InfosmeCoreV1::updateSmsc( const char*       smscId,
                                const SmscConfig* cfg )
{
    if (cfg) {
        // create/update
        MutexGuard mg(startMon_);
        SmscSender** ptr = smscs_.GetPtr(smscId);
        if (!ptr) {
            ptr = smscs_.SetItem(smscId,
                                 new SmscSender(*dlvMgr_,smscId,*cfg));
        } else if (*ptr) {
            (*ptr)->updateConfig(*cfg);
            // (*ptr)->waitUntilReleased();
        } else {
            *ptr = new SmscSender(*dlvMgr_,smscId,*cfg);
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
