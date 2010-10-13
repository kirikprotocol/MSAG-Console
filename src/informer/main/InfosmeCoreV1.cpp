#include <memory>
#include "InfosmeCoreV1.h"
#include "RegionLoader.h"
#include "informer/data/InputMessageSource.h"
#include "informer/io/InfosmeException.h"
#include "informer/newstore/InputStorage.h"
#include "informer/opstore/StoreJournal.h"
#include "informer/newstore/InputJournal.h"
#include "informer/sender/RegionSender.h"
#include "informer/sender/SmscSender.h"
#include "util/config/ConfString.h"
#include "util/config/ConfigView.h"
#include "informer/io/DirListing.h"
#include "informer/io/RelockMutexGuard.h"

using namespace smsc::util::config;

namespace {

std::string cgetString( const ConfigView& cv, const char* tag, const char* what )
{
    std::auto_ptr<char> str(cv.getString(tag,what));
    return std::string(str.get());
}


struct NumericNameFilter {
    inline bool operator()( const char* fn ) const {
        char* endptr;
        strtoul(fn,&endptr,10);
        return (*endptr=='\0');
    }
};

}

namespace eyeline {
namespace informer {


class InfosmeCoreV1::InputJournalReader : public InputJournal::Reader
{
public:
    InputJournalReader( InfosmeCoreV1& core ) : core_(core) {}
    virtual bool isStopping() const { return core_.isStopping(); }
    virtual void setRecordAtInit( dlvid_type               dlvId,
                                  regionid_type            regionId,
                                  const InputRegionRecord& rec,
                                  uint64_t                 maxMsgId )
    {
        smsc_log_debug(core_.log_,"setting input record D=%u/R=%u",dlvId,regionId);
        DeliveryPtr* ptr = core_.deliveries_.GetPtr(dlvId);
        if (!ptr) {
            smsc_log_info(core_.log_,"delivery D=%u is not found, ok",dlvId);
            return;
        }
        (*ptr)->setRecordAtInit( regionId, rec, maxMsgId );
    }


    virtual void postInit()
    {
        BindSignal bs;
        bs.bind = true;
        int dlvId;
        DeliveryPtr* ptr;
        smsc_log_debug(core_.log_,"invoking postInit to bind filled regions");
        for ( smsc::core::buffers::IntHash< DeliveryPtr >::Iterator i(core_.deliveries_);
              i.Next(dlvId,ptr); ) {
            if (core_.isStopping()) break;
            bs.regIds.clear();
            (*ptr)->postInitInput(bs.regIds);
            if (!bs.regIds.empty()) {
                bs.dlvId = (*ptr)->getDlvId();
                core_.bindDeliveryRegions(bs);
            }
        }
    }

private:
    InfosmeCoreV1& core_;
};


class InfosmeCoreV1::StoreJournalReader : public StoreJournal::Reader
{
public:
    StoreJournalReader( InfosmeCoreV1& core ) : core_(core) {}
    virtual bool isStopping() const { return core_.isStopping(); }
    virtual void setRecordAtInit( dlvid_type    dlvId,
                                  regionid_type regionId,
                                  Message&      msg,
                                  regionid_type serial )
    {
        smsc_log_error(core_.log_,"load store record D=%u/R=%u/M=%llu state=%s serial=%u",
                       dlvId, regionId, ulonglong(msg.msgId),
                       msgStateToString(MsgState(msg.state)), serial);
        DeliveryPtr* ptr = core_.deliveries_.GetPtr(dlvId);
        if (!ptr) {
            smsc_log_info(core_.log_,"delivery D=%u is not found, ok",dlvId);
            return;
        }
        (*ptr)->setRecordAtInit(regionId,msg,serial);
    }

    virtual void postInit()
    {
        BindSignal bsEmpty, bsFilled;
        bsEmpty.bind = false;
        bsFilled.bind = true;
        int dlvId;
        DeliveryPtr* ptr;
        smsc_log_debug(core_.log_,"invoking postInit to bind/unbind regions");
        for ( smsc::core::buffers::IntHash< DeliveryPtr >::Iterator i(core_.deliveries_);
              i.Next(dlvId,ptr); ) {
            if (core_.isStopping()) break;
            bsEmpty.regIds.clear();
            bsFilled.regIds.clear();
            (*ptr)->postInitOperative(bsFilled.regIds,bsEmpty.regIds);
            if (!bsEmpty.regIds.empty()) {
                bsEmpty.dlvId = (*ptr)->getDlvId();
                core_.bindDeliveryRegions(bsEmpty);
            }
            if (!bsFilled.regIds.empty()) {
                bsFilled.dlvId = (*ptr)->getDlvId();
                core_.bindDeliveryRegions(bsFilled);
            }
        }
    }

private:
    InfosmeCoreV1& core_;
};


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
storeJournal_(0),
inputJournal_(0)
{
}


InfosmeCoreV1::~InfosmeCoreV1()
{
    smsc_log_info(log_,"dtor started, FIXME: cleanup");
    stop();

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
    regions_.Empty();
    deliveries_.Empty();

    delete storeJournal_;
    delete inputJournal_;
    smsc_log_info(log_,"dtor finished");
}


void InfosmeCoreV1::init( const ConfigView& cfg )
{
    smsc_log_info(log_,"initing InfosmeCore");

    cs_.init("store/");

    // create journals
    if (!inputJournal_) inputJournal_ = new InputJournal(cs_);
    if (!storeJournal_) storeJournal_ = new StoreJournal(cs_);

    // create smscs
    {
        const char* fname = "smsc.xml";
        std::auto_ptr< Config > centerConfig( Config::createFromFile(fname));
        if (!centerConfig.get()) {
            throw InfosmeException("cannot create config from '%s'",fname);
        }
        std::auto_ptr< ConfigView > ccv(new ConfigView(*centerConfig.get(),"SMSCConnectors"));
        const ConfString defConn(ccv->getString("default","default SMSC id not found"));
        std::auto_ptr< CStrSet > connNames(ccv->getShortSectionNames());
        if ( connNames->find(defConn.str()) == connNames->end() ) {
            throw ConfigException("default SMSC '%s' does not match any section",defConn.c_str());
        }
        for ( CStrSet::iterator i = connNames->begin(); i != connNames->end(); ++i ) {
            smsc_log_debug(log_,"processing S='%s'",i->c_str());
            std::auto_ptr< ConfigView > sect(ccv->getSubConfig(i->c_str()));
            SmscConfig smscConfig;
            readSmscConfig(smscConfig, *sect.get());
            updateSmsc( i->c_str(), &smscConfig );
        }

        // create regions
        reloadRegions( defConn.str() );
    }

    {
        // loading deliveries
        smsc::core::buffers::TmpBuf<char,200> buf;
        const std::string& path = cs_.getStorePath();
        buf.setSize(path.size()+50);
        strcpy(buf.get(),path.c_str());
        strcat(buf.get(),"deliveries/");
        buf.SetPos(strlen(buf.get()));
        std::vector<std::string> chunks;
        std::vector<std::string> dlvs;
        smsc_log_debug(log_,"listing deliveries storage '%s'",buf.get());
        makeDirListing(NumericNameFilter(),S_IFDIR).list(buf.get(), chunks);
        std::sort( chunks.begin(), chunks.end() );
        for ( std::vector<std::string>::iterator ichunk = chunks.begin();
              ichunk != chunks.end(); ++ichunk ) {
            strcpy(buf.GetCurPtr(),ichunk->c_str());
            strcat(buf.GetCurPtr(),"/");
            dlvs.clear();
            smsc_log_debug(log_,"listing delivery chunk '%s'",buf.get());
            makeDirListing(NumericNameFilter(),S_IFDIR).list(buf.get(), dlvs);
            for ( std::vector<std::string>::iterator idlv = dlvs.begin();
                  idlv != dlvs.end();
                  ++idlv ) {
                // get dlvid
                const dlvid_type dlvId = strtoul(idlv->c_str(),0,10);
                std::auto_ptr<DeliveryInfo> info(new DeliveryInfo(cs_,dlvId));
                try {
                    info->read();
                    updateDelivery(dlvId,info);
                } catch (std::exception& e) {
                    smsc_log_warn(log_,"cannot read dlvInfo D=%u: %s",dlvId,e.what());
                    continue;
                }
            }
        }
    }

    // reading journals and binding deliveries and regions
    StoreJournalReader sjr(*this);
    storeJournal_->init(sjr);
    InputJournalReader ijr(*this);
    inputJournal_->init(ijr);
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

        // stop all smscs

        char* smscId;
        SmscSender* sender;
        for (Hash< SmscSender* >::Iterator i(&smscs_); i.Next(smscId,sender);) {
            sender->stop();
        }

        while (started_) {
            startMon_.wait(100);
        }
    }
    smsc_log_debug(log_,"leaving stop()");
}


void InfosmeCoreV1::start()
{
    if (started_) return;
    MutexGuard mg(startMon_);
    if (started_) return;
    Start();
}


void InfosmeCoreV1::selfTest()
{
    smsc_log_debug(log_,"selfTest started");
    dlvid_type dlvId = 22;
    MutexGuard mg(startMon_);
    DeliveryPtr* dlv = deliveries_.GetPtr(dlvId);
    if (!dlv) return;
    MessageList msgList;
    MessageLocker mlk;
    mlk.msg.subscriber = addressToSubscriber(1,1,79137654079ULL);
    mlk.msg.text.reset(new MessageText(0,1));
    mlk.msg.userData = "myfirstmsg";
    msgList.push_back(mlk);
    mlk.msg.subscriber = addressToSubscriber(1,1,79537699490ULL);
    mlk.msg.text.reset(new MessageText("the unbound message",0));
    mlk.msg.userData = "thesecondone";
    msgList.push_back(mlk);
    (*dlv)->addNewMessages(msgList.begin(), msgList.end());
    smsc_log_debug(log_,"selfTest finished");
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
        if (ptr && *ptr) {
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
            throw InfosmeException("S='%s' is not found for R=%u",smscId.c_str(),regionId);
        }

        RegionPtr* ptr = regions_.GetPtr(regionId);

        // update masks
        rf_.updateMasks(ptr ? ptr->get() : 0, *r.get());

        if (!ptr) {
            smsc_log_debug(log_,"creating R=%u for S='%s'",regionId,smscId.c_str());
            ptr = &regions_.Insert(regionId,RegionPtr(r.release()));
        } else {
            smsc_log_debug(log_,"updating R=%u for S='%s'",regionId,smscId.c_str());
            (*ptr)->replaceBy( *r.get() );
        }

        RegionSender** rs = regSends_.GetPtr(regionId);
        if (!rs) {
            rs = &regSends_.Insert(regionId,new RegionSender(**smsc,*ptr));
        } else {
            (*rs)->assignSender(**smsc,*ptr);
        }

    } while (true);
}


void InfosmeCoreV1::updateDelivery( dlvid_type dlvId,
                                    std::auto_ptr<DeliveryInfo>& dlvInfo )
{
    RelockMutexGuard mg(startMon_);
    DeliveryPtr* ptr = deliveries_.GetPtr(dlvId);
    smsc_log_debug(log_,"%s delivery D=%u",
                   (ptr ? (dlvInfo.get() ? "update" : "delete") : "create"),
                   unsigned(dlvId));
    if (ptr) {
        assert(ptr->get());
        if (!dlvInfo.get()) {
            // delete
            DeliveryPtr d;
            if (deliveries_.Pop(dlvId,d)) {
                mg.Unlock();
                BindSignal bs;
                bs.bind = false;
                bs.dlvId = dlvId;
                d->getRegionList(bs.regIds);
                bindDeliveryRegions(bs);
            };
            return;
        } else {
            // update
            (*ptr)->updateDlvInfo(*dlvInfo.get());
        }
    } else {
        // create
        if (!dlvInfo.get()) {
            throw InfosmeException("delivery info not passed");
        }
        InputMessageSource* ims = new InputStorage(*this,dlvInfo->getDlvId(),*inputJournal_);
        deliveries_.Insert(dlvId, DeliveryPtr(new Delivery(dlvInfo,*storeJournal_,ims)));
    }
    startMon_.notify();
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
    smsc::core::synchronization::MutexGuard mg(bindQueueLock_);
    bindQueue_.Push(bs);
}


void InfosmeCoreV1::startTransfer( TransferTask* task )
{
    ttp_.startTask(task);
}


int InfosmeCoreV1::Execute()
{
    {
        MutexGuard mg(startMon_);
        started_ = true;
    }
    smsc_log_info(log_,"starting main loop");
    while ( !stopping_ ) {

        // processing signals
        BindSignal bs;
        while (true) {
            {
                smsc::core::synchronization::MutexGuard bmg(bindQueueLock_);
                if (!bindQueue_.Pop(bs)) break;
            }
            bindDeliveryRegions(bs);
        }
        MutexGuard mg(startMon_);
        if (stopping_) break;
        startMon_.wait(1000);
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
    MutexGuard mg(startMon_);
    if (!bs.bind) {
        // unbind from senders
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

    // getting delivery
    DeliveryPtr* dlv = deliveries_.GetPtr(bs.dlvId);
    if (!dlv || !dlv->get()) {
        smsc_log_warn(log_,"D=%u is not found",unsigned(bs.dlvId));
        return;
    }
    for ( regIdVector::const_iterator i = bs.regIds.begin();
          i != bs.regIds.end(); ++i ) {
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
        RegionalStoragePtr rptr = (*dlv)->getRegionalStorage(*i,true);
        if (!rptr.get()) {
            smsc_log_warn(log_,"D=%u cannot create R=%u",unsigned(bs.dlvId),unsigned(*i));
            continue;
        }
        (*rs)->addDelivery(*rptr.get());
    }
}



}
}
