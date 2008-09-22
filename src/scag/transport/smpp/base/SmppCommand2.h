#ifndef _SCAG_TRANSPORT_SMPP_SMPPCOMMAND2_H
#define _SCAG_TRANSPORT_SMPP_SMPPCOMMAND2_H

#include <memory>
#include <string.h>
#include <string>
#include <vector>

#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"
#include "scag/transport/smpp/router/route_types.h"
#include "scag/transport/SCAGCommand2.h"
#include "smpp/smpp_sms.h"
#include "smpp/smpp_structures.h"
#include "smpp/smpp_time.h"
#include "sms/sms.h"
#include "system/status.h"
#include "util/64bitcompat.h"
#include "util/Exception.hpp"
#include "util/int.h"
#include "SmppCommandIds.h"

namespace scag2 {
namespace transport {
namespace smpp {

using std::auto_ptr;
using namespace smsc::smpp;
using smsc::sms::SMS;
using smsc::sms::SMSId;
using smsc::sms::Address;
using smsc::sms::Descriptor;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Exception;
using std::string;

const int ScagCommandDefaultPriority = 16;

/*
#define MAKE_COMMAND_STATUS(type,code) ((type<<16)|code)
#define GET_STATUS_TYPE(status) ((status>>16)&7)
#define GET_STATUS_CODE(status) (status&0x0ffff)
*/
static inline void fillField(auto_ptr<char>& field,const char* text,int length=-1)
{
  if(length==0 || text==NULL)return;
  if(length==-1)length=(int)strlen(text);
  field=auto_ptr<char>(new char[length+1]);
  memcpy(field.get(),text,length);
  field.get()[length]=0;
}

static inline void fillSmppAddr(auto_ptr<char>& field,PduAddress& addr)
{
  char buf[64];
  if(!addr.get_value() || !addr.get_value()[0])
  {
    fillField(field,NULL);
  }else
  {
    int len=sprintf(buf,".%d.%d.%.20s",
      (int)addr.get_typeOfNumber(),
      (int)addr.get_numberingPlan(),
      addr.get_value());
    fillField(field,buf,len);
  }
}


/// helpers for logging
const char* commandIdName( int );
const char* ussdOpName( int );


/// shared data for commands
struct SmppCommandData
{
    int        priority;
    uint32_t   uid;       // serial number
    uint32_t   dialogId;
    int        status;
    uint32_t   flags;

    SmppCommandData() :
    priority( ScagCommandDefaultPriority ),
    uid(uint32_t(-1)),
    dialogId(uint32_t(-1)),
    status(0),
    flags(0) {}
};


struct ReplaceSm
{
    auto_ptr<char> messageId;
    auto_ptr<char> sourceAddr;
    time_t scheduleDeliveryTime;
    time_t validityPeriod;
    int registeredDelivery;
    int smDefaultMsgId;
    int smLength;
    auto_ptr<char> shortMessage;

    SMSId getMessageId()const
    {
        SMSId id=0;
        if(messageId.get())sscanf(messageId.get(),"%lld",&id);
        return id;
    }

    ReplaceSm(PduReplaceSm* repl)
    {
        fillField(messageId,repl->get_messageId());
        if(!messageId.get() || !messageId.get()[0])
        {
            throw Exception("REPLACE: non empty messageId required");
        }
        fillSmppAddr(sourceAddr,repl->get_source());
        scheduleDeliveryTime=
            repl->scheduleDeliveryTime.size()?smppTime2CTime(repl->scheduleDeliveryTime):0;
        validityPeriod=
            repl->validityPeriod?smppTime2CTime(repl->validityPeriod):0;
        registeredDelivery=repl->get_registredDelivery();
        smDefaultMsgId=repl->get_smDefaultMsgId();
        smLength=repl->shortMessage.size();
        fillField(shortMessage,repl->shortMessage.cstr(),smLength);
    }

};


struct QuerySm
{
    auto_ptr<char> messageId;
    auto_ptr<char> sourceAddr;

    QuerySm(PduQuerySm* q)
    {
        fillField(messageId,q->get_messageId());
        if(!messageId.get())
        {
            messageId=auto_ptr<char>(new char[1]);
            messageId.get()[0]=0;
        }
        fillSmppAddr(sourceAddr,q->get_source());
    }

    SMSId getMessageId()const
    {
        SMSId id=0;
        if(messageId.get())sscanf(messageId.get(),"%lld",&id);
        return id;
    }
};


struct QuerySmResp
{
    auto_ptr<char> messageId;
    auto_ptr<char> finalDate;
    uint8_t        messageState;
    uint8_t        networkCode;
    // int            commandStatus; // use generic status
    QuerySmResp(SMSId id,time_t findate,int state,int netcode)
    {
        // commandStatus=cmdStatus;
        messageId=auto_ptr<char>(new char[22]);
        sprintf(messageId.get(),"%lld",id);
        if(findate==0)
        {
            finalDate=auto_ptr<char>(new char[1]);
            finalDate.get()[0]=0;
        }else
        {
            finalDate=auto_ptr<char>(new char[18]);
            cTime2SmppTime(findate,finalDate.get());
        }
        messageState=(uint8_t)state;
        networkCode=(uint8_t)netcode;
    }
    void fillPdu(PduQuerySmResp* pdu)
    {
        pdu->set_messageId(messageId.get());
        pdu->set_finalDate(finalDate.get());
        pdu->set_messageState(messageState);
        pdu->set_errorCode(networkCode);
    }
};


struct CancelSm
{
    auto_ptr<char> serviceType;
    auto_ptr<char> messageId;
    auto_ptr<char> sourceAddr;
    auto_ptr<char> destAddr;
    bool internall;
    bool force;

    CancelSm(PduCancelSm* pdu)
    {
        fillField(serviceType,pdu->get_serviceType());
        fillField(messageId,pdu->get_messageId());
        fillSmppAddr(sourceAddr,pdu->get_source());
        fillSmppAddr(destAddr,pdu->get_dest());
        internall=false;
        force=false;
    }

    CancelSm(SMSId id,const Address& oa,const Address& da)
    {
        char oabuf[32];
        char dabuf[32];
        int oalen=oa.toString(oabuf,sizeof(oabuf));
        int dalen=da.toString(dabuf,sizeof(dabuf));
        fillField(sourceAddr,oabuf,oalen);
        fillField(destAddr,dabuf,dalen);
        char idbuf[32];
        int idlen=sprintf(idbuf,"%lld",id);
        fillField(messageId,idbuf,idlen);
        internall=true;
        force=false;
    }

    CancelSm(SMSId id)
    {
        char idbuf[32];
        int idlen=sprintf(idbuf,"%lld",id);
        fillField(messageId,idbuf,idlen);
        internall=true;
        force=true;
    }

    SMSId getMessageId()const
    {
        SMSId id=0;
        if(messageId.get())sscanf(messageId.get(),"%lld",&id);
        return id;
    }
};


struct DlElement
{
    unsigned dest_flag : 1;
    unsigned __  : 7;
    unsigned ton : 8;
    unsigned npi : 8;
    string value;
};


struct UnsuccessElement
{
    Address addr;
    uint32_t errcode;
};


struct SubmitMultiSm
{
    SMS msg;
    uint8_t number_of_dests;
    DlElement dests[255];
    //SubmitMultiSm(const SMS& sms){}
    //~SubmitMultiSm(){}
};


struct SubmitMultiResp
{
    //SMS msg;
    char* messageId;
    uint32_t status;
    uint8_t no_unsuccess;
    UnsuccessElement unsuccess[255];
public:
    void set_messageId(const char* msgid)
    {
        if(!msgid)return;
        if ( messageId ) delete( messageId);
        messageId = new char[strlen(msgid)+1];
        strcpy(messageId,msgid);
    }
    void set_status(uint32_t st) { status = st; }
    const char* get_messageId() const {return messageId;}
    UnsuccessElement* get_unsuccess() { return unsuccess; }
    void set_unsuccessCount(unsigned c) { no_unsuccess = c; }
    uint32_t get_status() { return status; }
    SubmitMultiResp() : messageId(0), status(0) {};
    ~SubmitMultiResp() { if ( messageId ) delete messageId; }
};


struct BindCommand
{
    std::string sysId;
    std::string pass;
    std::string addrRange;
    std::string systemType;
    BindCommand(const char* id,const char* pwd,const char* rng,const char* st):sysId(id),pass(pwd),addrRange(rng),systemType(st){}
};


struct AlertNotification
{
    AlertNotification(PduAlertNotification* pdu)
    {
        src=PduAddress2Address(pdu->get_source());
        dst=PduAddress2Address(pdu->get_esme());
        status=pdu->optional.get_msAvailableStatus();
    }
    Address src;
    Address dst;
    int status;
};


struct SmsCommand : public SmppCommandData 
{
    SmsCommand() : dir(dsdUnknown), original_ussd_op(-1), orgDialogId_(-1), sliceCount_(1), slicedRespSent_(false), ref_(1) {}
    SmsCommand( const SMS& sms ): sms(sms), dir(dsdUnknown), original_ussd_op(-1), orgDialogId_(-1), sliceCount_(1), slicedRespSent_(false), ref_(1) {}

    void setSlicingParams( uint8_t srp, uint32_t cnt) {
        slicingRespPolicy_ = srp;
        sliceCount_ = cnt;
    }

    bool essentialSlicedResponse( bool failed )
    {
        MutexGuard mg(slicedMutex_);
        if ( slicedRespSent_ ) return true;
        if ( failed ||
             slicingRespPolicy_ == router::SlicingRespPolicy::ANY ||
             (! --sliceCount_) )
            return slicedRespSent_ = true;
        return false;
    }

    unsigned ref() {
        MutexGuard mg(mtx_);
        __require__(ref_ > 0);
        return ++ref_;
    }

    unsigned unref() {
        MutexGuard mg(mtx_);
        __require__(ref_ > 0);
        return --ref_;
    }

    uint32_t get_orgDialogId() const { return orgDialogId_; }
    void set_orgDialogId( uint32_t dlgId ) { orgDialogId_ = dlgId; }

public:
    SMS sms;
    Address orgSrc,orgDst;
    DataSmDirection dir;
    int  original_ussd_op;

protected:
    uint32_t orgDialogId_;
    uint32_t sliceCount_;
    uint8_t  slicingRespPolicy_;
    bool     slicedRespSent_;
    Mutex    slicedMutex_;

    Mutex    mtx_;
    unsigned ref_;

private:
    SmsCommand( const SmsCommand& cmd );
};


namespace SmppCommandFlags 
{
    const uint8_t NOTIFICATION_RECEIPT = 1;
    const uint8_t SERVICE_INITIATED_USSD_DIALOG = 2;
    const uint8_t EXPIRED_COMMAND = 4;
    const uint8_t FAILED_COMMAND_RESP = 8;
};


struct SmppEntity;
struct SmsResp;

/*
struct _SmppCommand
{
  ~_SmppCommand();

  _SmppCommand() : ref_count(0), dta(0), status(0), ent(0), dst_ent(0), priority(ScagCommandDefaultPriority), opId(-1), usr(0), flags(0), sliceCount(1), slicedRespSent(false)
  {
    if(!logger)
    {
        MutexGuard mt(loggerMutex);
        if(!logger) logger = Logger::getInstance("smppMan");
    }

    if(logger->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG))
    {
        uint32_t sc = 0;
        {
            MutexGuard mtxx(cntMutex);
            sc = ++commandCounter;
            uid = ++stuid;
        }
        // smsc_log_debug(logger, "Command create: count=%d, addr=%s, usr=%d, uid=%d", sc, session.Get() ? session->getSessionKey().abonentAddr.toString().c_str() : "", session.Get() ?  session->getSessionKey().USR : 0, uid);
    }
  }

    // uint32_t get_orgDialogId() const { return orgDialogId; }
    // void set_orgDialogId(uint32_t dlgId) { orgDialogId=dlgId; }

  SMS* get_sms()
  {
    __require__(cmdid==SUBMIT || cmdid==DELIVERY || cmdid==DATASM);
    return &((SmsCommand*)dta)->sms;
  }
  SmsCommand& get_smsCommand()
  {
    __require__(cmdid==SUBMIT || cmdid==DELIVERY || cmdid==DATASM);
    return *((SmsCommand*)dta);
  }
  const ReplaceSm& get_replaceSm(){return *(ReplaceSm*)dta;}
  const QuerySm& get_querySm(){return *(QuerySm*)dta;}
  const CancelSm& get_cancelSm(){return *(CancelSm*)dta;}
  SmsResp* get_resp() const
  {
    __require__(cmdid==DELIVERY_RESP || cmdid==SUBMIT_RESP || cmdid==DATASM_RESP);
    return (SmsResp*)dta;
  }
  uint32_t getCommandStatus() const;
  SubmitMultiResp* get_MultiResp() { return (SubmitMultiResp*)dta;}
  SubmitMultiSm* get_Multi() { return (SubmitMultiSm*)dta;}

  int get_priority(){return priority;};
  void set_priority(int newprio){priority=newprio;}

    // const Address& get_address() { return *(Address*)dta; }
    // void set_address(const Address& addr) { *(Address*)dta = addr; }

  //SmppHeader* get_smppPdu(){return (SmppHeader*)dta;}

  BindCommand& get_bindCommand(){return *((BindCommand*)dta);}

  AlertNotification& get_alertNotification()
  {
      return *((AlertNotification*)dta);
  }

    // int get_smeIndex(){return VoidPtr2Int(dta);}

  void set_status(int st){status=st;}
  int get_status(){return status;} // for enquirelink and unbind

 // int get_mode()
 // {
 // return VoidPtr2Int(dta);
 // }

};
 */

class _SmppCommand
{
protected:
    _SmppCommand() : cmdid_(UNKNOWN), serviceId_(-1),
    opId_( SCAGCommand::invalidOpId() ),
    session_(0),
    src_ent_(0), dst_ent_(0),
    shared_(&keep_), dta_(0) {}

private:
    _SmppCommand& operator = ( const _SmppCommand& );

protected:
    // -- generic stuff
    CommandId        cmdid_;
    int              serviceId_;
    opid_type        opId_;
    Session*         session_;

    // -- smpp-specific
    SmppEntity* src_ent_;
    SmppEntity* dst_ent_;

    // pointer to a shared data (flags, etc.) (not owned)
    SmppCommandData* shared_;

    // a keep for shared data (don't access it directly, use shared_ pointer instead).
    // this keep is not used for SUBMIT, DELIVERY, DATASM
    SmppCommandData keep_;

    // uint32_t orgDialogId_; only in resps

    // only in delivery, submit, datasm
    /*
    uint32_t sliceCount_;
    uint8_t slicingRespPolicy_;
    bool slicedRespSent_;
    Mutex    slicedMutex_;
     */

    // pointer to a command-specific data (owned)
    void* dta_;
};


class SmppCommand: public SCAGCommand, protected _SmppCommand
{
public:
    // specialized constructors (meta constructors)
    static std::auto_ptr<SmppCommand> makeCommandSm(CommandId command, const SMS& sms,uint32_t dialogId);
    static std::auto_ptr<SmppCommand> makeSubmitSm(const SMS& sms,uint32_t dialogId);
    static std::auto_ptr<SmppCommand> makeDeliverySm(const SMS& sms,uint32_t dialogId);
    static std::auto_ptr<SmppCommand> makeDataSm(const SMS& sms,uint32_t dialogId);
    static std::auto_ptr<SmppCommand> makeSubmitSmResp(const char* messageId, uint32_t dialogId, uint32_t status,bool dataSm=false);
    static std::auto_ptr<SmppCommand> makeDataSmResp(const char* messageId, uint32_t dialogId, uint32_t status);
    static std::auto_ptr<SmppCommand> makeSubmitMultiResp(const char* messageId, uint32_t dialogId, uint32_t status);
    static std::auto_ptr<SmppCommand> makeDeliverySmResp(const char* messageId, uint32_t dialogId, uint32_t status);
    static std::auto_ptr<SmppCommand> makeGenericNack(uint32_t dialogId,uint32_t status);
    static std::auto_ptr<SmppCommand> makeUnbind(int dialogId,int mode=0);
    static std::auto_ptr<SmppCommand> makeUnbindResp(uint32_t dialogId,uint32_t status,void* data);
    static std::auto_ptr<SmppCommand> makeReplaceSmResp(uint32_t dialogId,uint32_t status);
    static std::auto_ptr<SmppCommand> makeQuerySmResp(uint32_t dialogId,uint32_t status,
                                                      SMSId id,time_t findate,uint8_t state,uint8_t netcode);
    static std::auto_ptr<SmppCommand> makeCancelSmResp(uint32_t dialogId,uint32_t status);
    static std::auto_ptr<SmppCommand> makeCancel(SMSId id,const Address& oa,const Address& da);
    static std::auto_ptr<SmppCommand> makeCancel(SMSId id);
    static std::auto_ptr<SmppCommand> makeBindCommand(const char* sysId,const char* pwd,const char* addrRange,const char* sysType);
    static std::auto_ptr<SmppCommand> makeCommand(CommandId cmdId,uint32_t dialogId,uint32_t status,void* data);

    static void makeSMSBody( SMS* sms, const SmppHeader* pdu, bool forceDC );

protected:
    static std::auto_ptr<SmppCommand> makeCommandSmResp(CommandId cmdid, const char* messageId, uint32_t dialogId, uint32_t status, bool dataSm=false );

public:

    /// SCAGCommand interface
    virtual TransportType getType() const { return SMPP; }

    virtual int getServiceId() const {
        return serviceId_;
    }
    virtual void setServiceId(int serviceId) {
        serviceId_ = serviceId;
    }

    virtual opid_type getOperationId() const
    {
        return opId_;
    }
    virtual void setOperationId(opid_type op)
    {
        opId_ = op;
    }
    virtual uint8_t getCommandId() const
    {
        return cmdid_;
    }
    virtual uint32_t getSerial() const {
        return shared_->uid;
    }
    
    virtual Session* getSession() { return session_; };

    virtual void print( util::Print& p ) const;

    /// === SMPP specific

    SmppEntity* getEntity() const { return src_ent_; }
    void setEntity(SmppEntity* newent){ src_ent_ = newent; }

    SmppEntity* getDstEntity() const { return dst_ent_; }
    void setDstEntity(SmppEntity* newent) { dst_ent_ = newent; }

    void setFlag(uint32_t f) { shared_->flags |= f; }
    bool flagSet(uint32_t f) { return shared_->flags & f; }

    uint32_t get_dialogId() const { return shared_->dialogId; }
    void set_dialogId(uint32_t dlgId) { shared_->dialogId = dlgId; }

    int get_status() const { return shared_->status; } // for enquirelink and unbind
    void set_status(int st) { shared_->status = st; }

    int get_priority() const { return shared_->priority; }
    void set_priority( int prio ) { shared_->priority = prio; }

    inline bool isResp() const {
        return ( cmdid_ == SUBMIT_RESP ||
                 cmdid_ == DELIVERY_RESP ||
                 cmdid_ == DATASM_RESP );
    }

    // === access to different command types

    inline SMS* get_sms();

    SmsCommand& get_smsCommand() {
        __require__(cmdid_ == SUBMIT || cmdid_ == DELIVERY || cmdid_ == DATASM );
        return * reinterpret_cast<SmsCommand*>(dta_);
    }

    inline SmsResp* get_resp();

    SubmitMultiSm* get_Multi() {
        return reinterpret_cast<SubmitMultiSm*>(dta_);
    }

    SubmitMultiResp* get_MultiResp() {
        return reinterpret_cast<SubmitMultiResp*>(dta_);
    }

    const ReplaceSm& get_replaceSm() const {
        return * reinterpret_cast< const ReplaceSm* >(dta_);
    }

    const QuerySm& get_querySm() const {
        return * reinterpret_cast< const QuerySm* >(dta_);
    }

    const CancelSm& get_cancelSm() const {
        return * reinterpret_cast< const CancelSm* >(dta_);
    }

    BindCommand& get_bindCommand() {
        return * reinterpret_cast< BindCommand* >(dta_);
    }

    AlertNotification& get_alertNotification() {
        return * reinterpret_cast< AlertNotification* >(dta_);
    }

    // === ctors, dtors, copying

    virtual ~SmppCommand() {
        dispose();
    }
    SmppCommand( SmppHeader* pdu, bool forceDC = false );
    SmppHeader* makePdu( bool forceDC=false );

    /// cloning the command (not all command types are allowed to be cloned)
    virtual std::auto_ptr<SmppCommand> clone();

protected:
    uint32_t makeSmppStatus(uint32_t status)
    {
        return status;
    }
    
private:
    SmppCommand();
    SmppCommand( const SmppCommand& c );
    const SmppCommand& operator = ( const SmppCommand& );

    // destroying dta_ depending on command type
    void dispose();

    /// this method is invoked in virtual constructor to setup additional
    /// fields: uid, etc. (may be extended in the future).
    void postfix();

    inline void getlogger() {
        if ( ! log_ ) {
            MutexGuard mg(loggerMutex_);
            if ( !log_ ) log_ = smsc::logger::Logger::getInstance("smpp.cmd");
        }
    }

public:

protected:
    virtual void setSession( Session* s) { session_ = s; };

private:
    static Logger* log_;
    static Mutex   loggerMutex_;
    static uint32_t commandCounter; // for debugging
    static Mutex    cntMutex;
    static uint32_t stuid;
};



struct SmsResp
{
private:
    char* messageId;
    bool dataSm;
    // uint32_t status; taken from command data
    // SMS* sms; // points to orgCmd
    DataSmDirection dir;
    SmppCommand* orgCmd;

    bool bHasDeliveryFailureReason;
    uint8_t deliveryFailureReason;
    bool bHasAdditionalStatusInfoText;
    string additionalStatusInfoText;
    bool bHasDpfResult;
    uint8_t dpfResult;
    bool bHasNetworkErrorCode;
    uint32_t networkErrorCode;
    Logger* logger;

public:

    SmsResp();
    ~SmsResp();

    void set_dir(DataSmDirection direction)
    {
        dir = direction;
    }

    DataSmDirection get_dir()
    {
        return dir;
    }

    void set_messageId(const char* msgid)
    {
        if(!msgid)return;
        if ( messageId ) delete( messageId);
        messageId = new char[strlen(msgid)+1];
        strcpy(messageId,msgid);
    }

    void setDeliveryFailureReason(uint8_t reason) {
        if (reason < 4) {
            bHasDeliveryFailureReason  = true;
            deliveryFailureReason = reason;
        } else {
            smsc_log_warn(logger, "error value for 'delivery_failure_reason' tag: %d. should be >=0 & <=3", reason);
        }
    }

    uint8_t getDeliveryFailureReason() const {
        return deliveryFailureReason;
    }

    void setAdditionalStatusInfoText(const char* info) {
        if (strlen(info) <= 255) {
            bHasAdditionalStatusInfoText = true;
            additionalStatusInfoText = info;
        } else {
            smsc_log_warn(logger, "to long string (should be <= 255) for 'additional_status_info_text' tag: '%s'", info);
        }
    }

    const char* getAdditionalStatusInfoText() const {
        return additionalStatusInfoText.c_str();
    }

    void setDpfResult(uint8_t result) {
        if (result < 2) {
            bHasDpfResult = true;
            dpfResult = result;
        } else {
            smsc_log_warn(logger, "error value for 'dpf_result' tag: %d. should be 0 or 1", result);
        }
    }

    uint8_t getDpfResult() const {
        return dpfResult;
    }

    bool hasAdditionalStatusInfoText() const {
        return bHasAdditionalStatusInfoText;
    }

    bool hasDeliveryFailureReason() const {
        return bHasDeliveryFailureReason;
    }

    bool hasDpfResult() const {
        return bHasDpfResult;
    }

    bool hasNetworkErrorCode() const {
        return bHasNetworkErrorCode;
    }

    void setNetworkErrorCode(uint32_t code) {
        bHasNetworkErrorCode = true;
        networkErrorCode = code;
    }

    uint32_t getNetworkErrorCode() const {
        return networkErrorCode;
    }

    const char* get_messageId()const
    {
        return messageId?messageId:"";
    }

    void set_dataSm()
    {
        dataSm=true;
    }
    bool get_dataSm() const
    {
        return dataSm;
    }

    SMS* get_sms() {
        if ( ! hasOrgCmd() ) return 0;
        return getOrgCmd()->get_sms();
    }

    /*
    void set_sms(SMS* s)
    {
        sms = s;
    }
    SMS* get_sms() const
    {
        return sms;
    }
     */

    // taking ownership
    void setOrgCmd(SmppCommand* o);
    inline SmppCommand* getOrgCmd() { return orgCmd; }
    inline bool hasOrgCmd() { return orgCmd; };

  int expiredUid;
  bool expiredResp;

};



inline SMS* SmppCommand::get_sms()
{        
    SMS* ret = 0;
    switch (cmdid_) {
    case SUBMIT :
    case DELIVERY :
    case DATASM : {
        ret = & get_smsCommand().sms;
        break;
    }
    case SUBMIT_RESP :
    case DELIVERY_RESP :
    case DATASM_RESP : {
        SmsResp* r = get_resp();
        if (r) ret = r->get_sms();
        break;
    }
    default:
        ret = 0;
    }
    return ret;
}


inline SmsResp* SmppCommand::get_resp() 
{
    __require__(cmdid_ == SUBMIT_RESP || cmdid_ == DELIVERY_RESP || cmdid_ == DATASM_RESP );
    return reinterpret_cast<SmsResp*>(dta_);
}

} //smpp
} //transport
} //scag

#endif
