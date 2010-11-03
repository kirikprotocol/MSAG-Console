#include <memory>
#include "DeliveryMgr.h"
#include "InfosmeCoreV1.h"
#include "RegionLoader.h"
#include "informer/data/UserInfo.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/RelockMutexGuard.h"
#include "informer/sender/RegionSender.h"
#include "informer/sender/SmscSender.h"
#include "util/config/ConfString.h"
#include "util/config/ConfigView.h"
#include "informer/admin/AdminServer.hpp"

using namespace smsc::util::config;

namespace {

using namespace eyeline::informer;

std::string cgetString( const ConfigView& cv, const char* tag, const char* what )
{
    std::auto_ptr<char> str(cv.getString(tag,what));
    return std::string(str.get());
}

struct SortUserById
{
    bool operator () ( const UserInfoPtr& a, const UserInfoPtr& b ) const
    {
        return ::strcmp(a->getUserId(), b->getUserId()) < 0;
    }
};

} // namespace

namespace eyeline {
namespace informer {

void InfosmeCoreV1::readSmscConfig( SmscConfig& cfg, const ConfigView& config )
{
    smsc::sme::SmeConfig& rv = cfg.smeConfig;
    rv.host = ::cgetString(config,"host","SMSC host was not defined");
    rv.sid = ::cgetString(config,"sid","infosme id was not defined");
    rv.port = config.getInt("port","SMSC port was not defined");
    rv.timeOut = config.getInt("timeout","connect timeout was not defined");
    try {
        rv.password = ::cgetString(config,"password","InfoSme password wasn't defined !");
    } catch (ConfigException&) {}
    try {
        const std::string systemType = ::cgetString(config,"systemType","InfoSme system type wasn't defined !");
        rv.setSystemType(systemType);
    } catch (ConfigException&) {}
    try {
        rv.interfaceVersion = config.getInt("interfaceVersion","InfoSme interface version wasn't defined!");
    } catch (ConfigException&) {}
    try {
        const std::string ar = ::cgetString(config,"rangeOfAddress","InfoSme range of address was not defined");
        rv.setAddressRange(ar);
    } catch (ConfigException&) {}
    try {
        cfg.ussdPushOp = config.getInt("ussdPushTag");
    } catch (ConfigException) {
        cfg.ussdPushOp = -1;
    }
    try {
        cfg.ussdPushVlrOp = config.getInt("ussdPushVlrTag");
    } catch (ConfigException) {
        cfg.ussdPushVlrOp = -1;
    }
}


InfosmeCoreV1::InfosmeCoreV1() :
log_(smsc::logger::Logger::getInstance("core")),
stopping_(false),
started_(false),
dlvMgr_(0),
adminServer_(0),
logStateTime_(0)
{
}


InfosmeCoreV1::~InfosmeCoreV1()
{
    smsc_log_info(log_,"dtor started, FIXME: cleanup");

    stop();

    delete adminServer_;

    char* smscId;
    SmscSender* sender;
    for ( Hash< SmscSender* >::Iterator i(&smscs_); i.Next(smscId,sender); ) {
        smsc_log_debug(log_,"destroying smsc '%s'",smscId);
        delete sender;
    }
    int regId;
    RegionSender* regsend;
    for ( IntHash< RegionSender* >::Iterator i(regSends_); i.Next(regId,regsend); ) {
        smsc_log_debug(log_,"destroying regsend %u", regionid_type(regId));
        delete regsend;
    }

    delete dlvMgr_;

    regions_.Empty();
    users_.Empty();
    smsc_log_info(log_,"dtor finished");
}


void InfosmeCoreV1::init( const ConfigView& cfg )
{
    smsc_log_info(log_,"initing InfosmeCore");

    cs_.init("store/");
    if (!dlvMgr_) { dlvMgr_ = new DeliveryMgr(*this,cs_); }

    // create admin server
    if (!adminServer_) {
        adminServer_ = new admin::AdminServer();
        adminServer_->assignCore(this);
        adminServer_->Init(smsc::util::config::ConfString(cfg.getString("Admin.host")).c_str(),
                           cfg.getInt("Admin.port"),
                           cfg.getInt("Admin.handlers") );
    }

    // FIXME: load users
    {
        UserInfo* ui = new UserInfo("bukind","pwd");
        users_.Insert(ui->getUserId(),UserInfoPtr(ui));
    }

    // create smscs
    {
        smsc_log_debug(log_,"--- loading smsc ---");
        const char* fname = "smsc.xml";
        smsc_log_info(log_,"reading smscs config '%s'",fname);
        std::auto_ptr< Config > centerConfig( Config::createFromFile(fname));
        if (!centerConfig.get()) {
            throw InfosmeException(EXC_CONFIG,"cannot load config '%s'",fname);
        }
        std::auto_ptr< ConfigView > ccv(new ConfigView(*centerConfig.get(),"SMSCConnectors"));
        const ConfString defConn(ccv->getString("default","default SMSC id not found"));
        std::auto_ptr< CStrSet > connNames(ccv->getShortSectionNames());
        if ( connNames->find(defConn.str()) == connNames->end() ) {
            throw ConfigException("default SMSC '%s' does not match any section",defConn.c_str());
        }
        for ( CStrSet::iterator i = connNames->begin(); i != connNames->end(); ++i ) {
            smsc_log_info(log_,"processing smsc S='%s'",i->c_str());
            std::auto_ptr< ConfigView > sect(ccv->getSubConfig(i->c_str()));
            SmscConfig smscConfig;
            readSmscConfig(smscConfig, *sect.get());
            updateSmsc( i->c_str(), &smscConfig );
        }

        // create regions
        smsc_log_debug(log_,"--- loading regions ---");
        reloadRegions( defConn.str() );
    }

    dlvMgr_->init();
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
            smsc_log_info(log_,"stop() received");
            stopping_ = true;
            bindQueue_.notify();  // wake up bind queue
            itp_.stopNotify();
            rtp_.stopNotify();
            startMon_.notifyAll();
        }
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
    smsc_log_debug(log_,"leaving stop()");
}


void InfosmeCoreV1::addUser( const char* user )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: addUser");
}


void InfosmeCoreV1::deleteUser( const char* user )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: deleteUser");
}


UserInfoPtr InfosmeCoreV1::getUserInfo( const char* login )
{
    MutexGuard mg(startMon_);
    UserInfoPtr* ptr = users_.GetPtr(login);
    if (!ptr) return UserInfoPtr();
    return *ptr;
}


void InfosmeCoreV1::updateUserInfo( const char* user )
{
    throw InfosmeException(EXC_NOTIMPL,"FIXME: updateUserInfo");
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
    smsc_log_debug(log_,"selfTest started");
    dlvid_type dlvId = 22;
    DeliveryImplPtr dlv;
    if (dlvMgr_->getDelivery(dlvId,dlv)) {
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
        setDeliveryState(dlvId,DLVSTATE_ACTIVE,0);
    }
    smsc_log_debug(log_,"selfTest finished");
}


void InfosmeCoreV1::reloadRegions( const std::string& defaultSmscId )
{
    // reading region file
    RegionLoader rl("regions.xml",defaultSmscId.c_str());

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

        RegionSender** rs = regSends_.GetPtr(regionId);
        if (!rs) {
            rs = &regSends_.Insert(regionId,new RegionSender(**smsc,*ptr));
        } else {
            (*rs)->assignSender(**smsc,*ptr);
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
    bindQueue_.Push(bs);
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


/*
void InfosmeCoreV1::updateDelivery( dlvid_type dlvId,
                                    const DeliveryInfoData& info )
{
    dlvMgr_->updateDelivery(dlvId,info);
}
 */


void InfosmeCoreV1::deleteDelivery( const UserInfo& userInfo,
                                    dlvid_type      dlvId )
{
    BindSignal bs;
    bs.bind = false;
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
    if ( &(ptr->getDlvInfo().getUserInfo()) != &userInfo &&
         !userInfo.hasRole(USERROLE_ADMIN)) {
        throw InfosmeException(EXC_ACCESSDENIED,"access denied to delivery %u",dlvId);
    }
    return ptr;
}


void InfosmeCoreV1::setDeliveryState( dlvid_type   dlvId,
                                      DlvState     newState,
                                      msgtime_type planTime )
{
    BindSignal bs;
    bs.dlvId = dlvId;
    bs.bind = (newState == DLVSTATE_ACTIVE ? true : false);
    dlvMgr_->setDeliveryState(dlvId,newState,planTime,bs.regIds);
    bindDeliveryRegions(bs);
}


void InfosmeCoreV1::logStateChange( ulonglong   ymd,
                                    dlvid_type  dlvId,
                                    const char* userId,
                                    DlvState    newState,
                                    msgtime_type planTime )
{
    // prepare the buffer
    char buf[100];
    const int buflen = sprintf(buf,"%04u,%c,%u,%s,%u\n",
                               unsigned(ymd % 10000), dlvStateToString(newState)[0],
                               dlvId, userId, planTime );
    if ( buflen < 0 ) {
        throw InfosmeException(EXC_SYSTEM,"cannot write dlv state change, dlvId=%u",dlvId);
    }

    const ulonglong fileTime = ymd / 10000 * 10000;
    char fnbuf[50];
    if ( logStateTime_ < fileTime ) {
        const unsigned day( unsigned(fileTime / 1000000));
        sprintf(fnbuf,"status_log/%04u.%02u.%02u/%02u.log",
                day / 10000, (day / 100) % 100, day % 100,
                unsigned((ymd/10000) % 100) );
    }

    MutexGuard mg(logStateLock_);
    if ( logStateTime_ < fileTime ) {
        // need to replace cur file
        FileGuard fg;
        fg.create( (cs_.getStorePath() + fnbuf).c_str(), true );
        fg.seek(0,SEEK_END);
        if (fg.getPos() == 0) {
            const char* header = "# MINSEC,STATE,DLVID,USER,PLAN\n";
            fg.write( header, strlen(header));
        }
        logStateTime_ = fileTime;
        logStateFile_.swap(fg);
    } else if ( logStateTime_ > fileTime ) {
        // fix delayed record
        memcpy(buf,"0000",4);
    }
    logStateFile_.write(buf,size_t(buflen));
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
    typedef std::vector<regionid_type> regIdVector;
    smsc_log_debug(log_,"%sbinding D=%u with [%s]",
                   bs.bind ? "" : "un", unsigned(bs.dlvId),
                   formatRegionList(bs.regIds.begin(),bs.regIds.end()).c_str());

    if (! bs.bind) {
        // unbind from senders
        MutexGuard mg(startMon_);
        for (regIdVector::const_iterator i = bs.regIds.begin();
             i != bs.regIds.end(); ++i) {
            RegionSender** rs = regSends_.GetPtr(*i);
            if (!rs || !*rs) {
                smsc_log_warn(log_,"RS=%u is not found",unsigned(*i));
                continue;
            }
            (*rs)->removeDelivery(bs.dlvId);
        }
        return;
    }

    DeliveryImplPtr dlv;
    if (!dlvMgr_->getDelivery(bs.dlvId,dlv)) {
        throw InfosmeException(EXC_NOTFOUND,"D=%u is not found",bs.dlvId);
    }

    for ( regIdVector::const_iterator i = bs.regIds.begin();
          i != bs.regIds.end(); ++i ) {
        MutexGuard mg(startMon_);
        RegionPtr* ptr = regions_.GetPtr(*i);
        if (!ptr || !ptr->get()) {
            // no such region
            smsc_log_warn(log_,"R=%u is not found",unsigned(*i));
            continue;
        }
        RegionSender** rs = regSends_.GetPtr(*i);
        if (!rs || !*rs) {
            // no such region sender
            smsc_log_warn(log_,"RS=%u is not found",unsigned(*i));
            continue;
        }
        RegionalStoragePtr rptr = dlv->getRegionalStorage(*i,true);
        if (!rptr.get()) {
            smsc_log_warn(log_,"D=%u cannot create R=%u",unsigned(bs.dlvId),unsigned(*i));
            continue;
        }
        (*rs)->addDelivery(*rptr.get());
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
        sprintf(buf,"statistics/%04u.%02u.%02u/dlv%02u.log",
                now.tm_year + 1900, now.tm_mon+1, now.tm_mday, now.tm_hour );
        fg.create((cs_.getStorePath()+buf).c_str(),true);
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

}
}
