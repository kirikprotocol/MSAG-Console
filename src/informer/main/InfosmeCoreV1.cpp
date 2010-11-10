#include <memory>
#include "informer/admin/AdminServer.hpp"
#include "informer/dcp/DcpServer.hpp"
#include "DeliveryMgr.h"
#include "InfosmeCoreV1.h"
#include "RegionLoader.h"
#include "informer/data/UserInfo.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/RelockMutexGuard.h"
#include "informer/sender/RegionSender.h"
#include "informer/sender/SmscSender.h"
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
                     const Config& config )
{
    try {
        smsc::sme::SmeConfig& rv = cfg.smeConfig;
        rv.host = config.getString("host");
        rv.sid = config.getString("sid");
        rv.port = config.getInt("port");
        rv.timeOut = config.getInt("timeout");
        try {
            rv.password = config.getString("password");
        } catch (HashInvalidKeyException&) {}
        try {
            const std::string systemType = config.getString("systemType");
            rv.setSystemType(systemType);
        } catch (HashInvalidKeyException&) {}
        try {
            rv.interfaceVersion = config.getInt("interfaceVersion");
        } catch (HashInvalidKeyException&) {}
        try {
            const std::string ar = config.getString("rangeOfAddress");
            rv.setAddressRange(ar);
        } catch (HashInvalidKeyException&) {}
        try {
            cfg.ussdPushOp = config.getInt("ussdPushTag");
        } catch (HashInvalidKeyException&) {
            cfg.ussdPushOp = -1;
        }
        try {
            cfg.ussdPushVlrOp = config.getInt("ussdPushVlrTag");
        } catch (HashInvalidKeyException&) {
            cfg.ussdPushVlrOp = -1;
        }
    } catch ( InfosmeException& e ) {
        throw;
    } catch ( HashInvalidKeyException& e ) {
        throw InfosmeException(EXC_CONFIG,"exc in smsc '%s': param '%s' not found", name, e.getKey());
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_CONFIG,"exc in smsc '%s': %s", name, e.what());
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
adminServer_(0),
dcpServer_(0)
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

    // detaching region senders
    smsc_log_info(log_,"--- destroying region senders ---");
    int regId;
    RegionSenderPtr* regsend;
    for ( IntHash< RegionSenderPtr >::Iterator i(regSends_); i.Next(regId,regsend); ) {
        smsc_log_debug(log_,"detaching regsend RS=%u", regionid_type(regId));
        (*regsend)->assignSender(0,RegionPtr());
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

        cs_.init( cfg->getString("storePath"),
                  cfg->getString("statPath") );

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
        const char* smscfilename = "smsc.xml";
        filename = smscfilename;

        smsc_log_info(log_,"--- loading smscs ---");
        std::auto_ptr< Config > scfg( Config::createFromFile(smscfilename));
        if (!scfg.get()) {
            throw InfosmeException(EXC_CONFIG,"config file '%s' is not found",smscfilename);
        }
        section = "SMSCConnectors";
        scfg.reset( scfg->getSubConfig(section,true) );
        const char* defConn = scfg->getString("default");
        std::auto_ptr< CStrSet > connNames(scfg->getRootSectionNames());
        if ( connNames->find(defConn) == connNames->end() ) {
            throw InfosmeException(EXC_CONFIG,"default SMSC '%s' does not match any section",defConn);
        }
        defaultSmscId_ = defConn;
        for ( CStrSet::iterator i = connNames->begin(); i != connNames->end(); ++i ) {
            smsc_log_info(log_,"processing smsc S='%s'",i->c_str());
            std::auto_ptr< Config > sect(scfg->getSubConfig(i->c_str(),true));
            SmscConfig smscConfig;
            readSmscConfig(i->c_str(), smscConfig, *sect.get());
            updateSmsc( i->c_str(), &smscConfig );
        }

        // create regions
        smsc_log_info(log_,"--- loading regions ---");
        reloadRegions();

        dlvMgr_->init();

        filename = mainfilename;
        section = "informer";
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
        throw InfosmeException(EXC_CONFIG,"exc in core init: %s", e.what());
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
            bindQueue_.notify();  // wake up bind queue
            itp_.stopNotify();
            rtp_.stopNotify();
            startMon_.notifyAll();
        }
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
    UserInfoPtr ptr = getUserInfo(user);
    if (ptr.get()) throw InfosmeException(EXC_ALREADYEXIST,"user '%s' already exists",user);
    loadUsers(user);
}


void InfosmeCoreV1::deleteUser( const char* login )
{
    if (!login) throw InfosmeException(EXC_LOGICERROR,"deluser NULL passed");
    UserInfoPtr user;
    {
        MutexGuard mg(startMon_);
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
    if (!login) throw InfosmeException(EXC_LOGICERROR,"userid NULL passed");
    MutexGuard mg(startMon_);
    UserInfoPtr* ptr = users_.GetPtr(login);
    if (!ptr) return UserInfoPtr();
    return *ptr;
}


void InfosmeCoreV1::getUsers( std::vector< UserInfoPtr >& users )
{
    users.reserve( users.size() + users_.GetCount() + 2 );
    MutexGuard mg(startMon_);
    char* userId;
    UserInfoPtr* user;
    for ( Hash< UserInfoPtr >::Iterator i(&users_); i.Next(userId,user); ) {
        users.push_back(*user);
    }
}


void InfosmeCoreV1::updateUserInfo( const char* login )
{
    UserInfoPtr ptr = getUserInfo(login);
    if (!ptr.get()) throw InfosmeException(EXC_NOTFOUND,"user '%s' is not found",login);
    loadUsers(login);
}


void InfosmeCoreV1::updateSmsc( const std::string& smscId,
                                const SmscConfig*  cfg )
{
    if (cfg) {
        // create/update
        SmscSender* p = 0;
        SmscSender** ptr = 0;
        MutexGuard mg(startMon_);
        try {
            ptr = smscs_.GetPtr(smscId.c_str());
            if (!ptr) {
                p = new SmscSender(*dlvMgr_,smscId,*cfg);
                ptr = smscs_.SetItem(smscId.c_str(),p);
            } else if (*ptr) {
                (*ptr)->updateConfig(*cfg);
                // (*ptr)->waitUntilReleased();
            } else {
                p = new SmscSender(*dlvMgr_,smscId,*cfg);
                *ptr = p;
            }
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"smscsender create error: %s", e.what());
            if (p) {
                smscs_.Delete(smscId.c_str());
                delete p;
            }
        }
        if (ptr && *ptr && started_) {
            (*ptr)->start();
        }
    } else {
        // delete the smsc
        MutexGuard mg(startMon_);
        SmscSender* ptr = 0;
        if (smscs_.Pop(smscId.c_str(),ptr) && ptr) {
            ptr->stop();
            // ptr->waitUntilReleased();
            delete ptr;
        }
    }
}


void InfosmeCoreV1::selfTest()
{
    smsc_log_debug(log_,"--- selfTest started ---");

    const char* userId = "bukind";
    smsc_log_debug(log_,"--- getting user '%s' ---",userId);
    UserInfoPtr user = getUserInfo(userId);
    if (!user.get()) {
        throw InfosmeException(EXC_NOTFOUND,"U='%s' is not found",userId);
    }
    smsc_log_debug(log_,"--- user '%s' got ---",userId);

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
        data.userData = "0xdeadbeaf";
        data.sourceAddress = "10000";
    }
    smsc_log_debug(log_,"--- adding new delivery ---");
    const dlvid_type dlvId = addDelivery(*user, data);
    smsc_log_debug(log_,"--- delivery D=%u added ---", dlvId);

    DeliveryPtr dlv = getDelivery(*user,dlvId);
    if (!dlv.get()) {
        throw InfosmeException(EXC_NOTFOUND,"D=%u is not found",dlvId);
    }
    smsc_log_debug(log_,"--- delivery D=%u got, setting delivery texts ---", dlvId);

    // adding glossary messages
    {
        std::vector< std::string > glotexts;
        glotexts.push_back("the first message");
        glotexts.push_back("the second message");
        dlv->setGlossary( glotexts );
    }
    smsc_log_debug(log_,"--- glossary updated, adding messages ---");
    {
        MessageList msgList;
        MessageLocker mlk;
        mlk.msg.subscriber = addressToSubscriber(11,1,1,79137654079ULL);
        mlk.msg.text.reset(new MessageText(0,1));
        mlk.msg.userData = "myfirstmsg";
        msgList.push_back(mlk);
        mlk.msg.subscriber = addressToSubscriber(11,1,1,79537699490ULL);
        mlk.msg.text.reset(new MessageText("the unbound message",0));
        mlk.msg.userData = "thesecondone";
        msgList.push_back(mlk);
        dlv->addNewMessages(msgList.begin(), msgList.end());
        smsc_log_debug(log_,"--- messages added, setting active state ---");
        dlv->setState(DLVSTATE_ACTIVE);
        smsc_log_debug(log_,"--- delivery activated ---");
    }
    smsc_log_debug(log_,"--- selfTest finished ---");
}


void InfosmeCoreV1::reloadRegions()
{
    // reading region file
    RegionLoader rl("regions.xml",defaultSmscId_.c_str());

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
            (*rs)->assignSender(*smsc,*ptr);
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
    if (log_->isDebugEnabled()) {
        smsc_log_debug(log_,"D=%u finish state change, state=%s, bind=%d, regs=[%s]",
                       bs.dlvId, dlvStateToString(dlv.getState()), bs.bind,
                       formatRegionList(bs.regIds.begin(),bs.regIds.end()).c_str() );
    }
    dlvMgr_->finishStateChange( currentTime, ymdTime, dlv );
    if ( bs.regIds.empty() ) return;
    bs.ignoreState = false;
    bindDeliveryRegions( bs );
}


void InfosmeCoreV1::addSmsc( const char* smscId )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: addSmsc");
}


void InfosmeCoreV1::updateSmsc( const char* smscId )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: updateSmsc");
}


void InfosmeCoreV1::deleteSmsc( const char* smscId )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: deleteSmsc");
}


void InfosmeCoreV1::updateDefaultSmsc( const char* smscId )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: updateDefaultSmsc");
}


void InfosmeCoreV1::addRegion( regionid_type regionId )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: addRegion");
}


void InfosmeCoreV1::updateRegion( regionid_type regionId )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: updateRegion");
}


void InfosmeCoreV1::deleteRegion( regionid_type regionId )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: deleteRegion");
}


dlvid_type InfosmeCoreV1::addDelivery( UserInfo& userInfo,
                                       const DeliveryInfoData& info )
{
    return dlvMgr_->createDelivery(userInfo,info);
}


void InfosmeCoreV1::deleteDelivery( const UserInfo& userInfo,
                                    dlvid_type      dlvId )
{
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
        fg.create((cs_.getStatPath()+buf).c_str(),0666,true);
        fg.seek(0,SEEK_END);
        if (fg.getPos()==0) {
            const char* header = "# MINSEC,USER,PAUSED,PLANNED,ACTIVE,FINISH,CANCEL\n";
            fg.write(header,strlen(header));
        }
        bufpos = buf + sprintf(buf,"%04u,",unsigned(ymd%10000));
    }
    std::vector< UserInfoPtr > users;
    {
        MutexGuard mg(startMon_);
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
        (*i)->popIncrementalStats(cs_,ds);
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
        throw InfosmeException(EXC_CONFIG,"loadUsers: %s",e.what());
    }

    smsc_log_info(log_,"users.xml has been read (%u users), applying",unsigned(uservec.size()));
    MutexGuard mg(startMon_);
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
    startMon_.notify();
}


}
}
