#include <cassert>
#include "scag/util/io/Print.h"
#include "SmppCommand2.h"
#include "SmppEntity2.h"
#include "scag/util/encodings/Encodings.h"
#include "scag/util/io/HexDump.h"

namespace scag2 {
namespace transport {
namespace smpp {

smsc::core::synchronization::Mutex SmppCommand::loggerMutex_;
smsc::logger::Logger* SmppCommand::log_ = 0;

smsc::core::synchronization::Mutex SmppCommand::cntMutex;
uint32_t SmppCommand::commandCounter = 0;
uint32_t SmppCommand::stuid = 0;

/*
uint32_t _SmppCommand::getCommandStatus() const
{
    return (cmdid == DELIVERY_RESP || cmdid == SUBMIT_RESP || cmdid == DATASM_RESP)
      ? ((SmsResp*)dta)->get_status() : status;
}
 */

const char* commandIdName( int id )
{
#define CMDIDNAME(x) case x: return #x
    switch (id) {
        CMDIDNAME(DELIVERY);
        CMDIDNAME(SUBMIT);
        CMDIDNAME(DATASM);
        CMDIDNAME(DELIVERY_RESP);
        CMDIDNAME(SUBMIT_RESP);
        CMDIDNAME(DATASM_RESP);
        CMDIDNAME(UNKNOWN);
        CMDIDNAME(FORWARD);
        CMDIDNAME(GENERIC_NACK);
        CMDIDNAME(QUERY);
        CMDIDNAME(QUERY_RESP);
        CMDIDNAME(UNBIND);
        CMDIDNAME(UNBIND_RESP);
        CMDIDNAME(REPLACE);
        CMDIDNAME(REPLACE_RESP);
        CMDIDNAME(CANCEL);
        CMDIDNAME(CANCEL_RESP);
        CMDIDNAME(ENQUIRELINK);
        CMDIDNAME(ENQUIRELINK_RESP);
        CMDIDNAME(SUBMIT_MULTI_SM);
        CMDIDNAME(SUBMIT_MULTI_SM_RESP);
        CMDIDNAME(BIND_TRANSCEIVER);
        CMDIDNAME(BIND_RECIEVER_RESP);
        CMDIDNAME(BIND_TRANSMITTER_RESP);
        CMDIDNAME(BIND_TRANCIEVER_RESP);
        CMDIDNAME(PROCESSEXPIREDRESP);
        CMDIDNAME(ALERT_NOTIFICATION);
    default: return "???";
    }
#undef CMDIDNAME
}


const char* ussdOpName( int id )
{
    using namespace smsc::smpp::UssdServiceOpValue;
#define USSDOPNAME(x) case x: return #x
    switch (id) {
        USSDOPNAME(USSR_REQUEST);
        USSDOPNAME(PSSR_RESPONSE);
        USSDOPNAME(USSN_REQUEST);
        USSDOPNAME(PSSD_INDICATION);
        USSDOPNAME(PSSR_INDICATION);
        USSDOPNAME(PSSD_RESPONSE);
        USSDOPNAME(USSR_CONFIRM);
        USSDOPNAME(USSN_CONFIRM);
    default : return "???";
    }
#undef USSDOPNAME
}


// specialized constructors (meta constructors)
std::auto_ptr<SmppCommand> SmppCommand::makeCommandSm( CommandId command, const SMS& sms, uint32_t dialogId )
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = command;
    cmd->dta_ = new SmsCommand(sms);
    cmd->shared_ = &cmd->get_smsCommand();
    cmd->set_dialogId(dialogId);
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeSubmitSm( const SMS& sms, uint32_t dialogId )
{
    return makeCommandSm( SUBMIT, sms, dialogId );
}


std::auto_ptr<SmppCommand> SmppCommand::makeDeliverySm(const SMS& sms,uint32_t dialogId)
{
    return makeCommandSm( DELIVERY, sms, dialogId );
}


std::auto_ptr<SmppCommand> SmppCommand::makeDataSm(const SMS& sms,uint32_t dialogId)
{
    return makeCommandSm(DATASM, sms, dialogId);
}


std::auto_ptr<SmppCommand> SmppCommand::makeSubmitSmResp(const char* messageId, uint32_t dialogId, uint32_t status,bool dataSm)
{
    return makeCommandSmResp(SUBMIT_RESP, messageId, dialogId, status, dataSm );
}


std::auto_ptr<SmppCommand> SmppCommand::makeCommandSmResp(CommandId cmdid, const char* messageId, uint32_t dialogId, uint32_t status, bool dataSm)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = cmdid;
    SmsResp* resp;
    cmd->dta_ = resp = new SmsResp( status );
    cmd->set_status(status);
    cmd->set_dialogId( dialogId );
    resp->set_messageId(messageId);
    if (dataSm) resp->set_dataSm();
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeDataSmResp(const char* messageId, uint32_t dialogId, uint32_t status)
{
    return makeCommandSmResp( DATASM_RESP, messageId, dialogId, status, true );
}


std::auto_ptr<SmppCommand> SmppCommand::makeSubmitMultiResp(const char* messageId, uint32_t dialogId, uint32_t status)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = SUBMIT_MULTI_SM_RESP;
    SubmitMultiResp* resp;
    cmd->dta_ = resp = new SubmitMultiResp;
    resp->set_messageId(messageId);
    cmd->set_status(status);
    cmd->set_dialogId(dialogId);
    resp->set_unsuccessCount(0);
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeDeliverySmResp(const char* messageId, uint32_t dialogId, uint32_t status)
{
    return makeCommandSmResp( DELIVERY_RESP, messageId, dialogId, status, false );
}


std::auto_ptr<SmppCommand> SmppCommand::makeGenericNack(uint32_t dialogId,uint32_t status)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = GENERIC_NACK;
    cmd->set_status( status );
    cmd->set_dialogId( dialogId );
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeUnbind(int dialogId,int mode)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = UNBIND;
    cmd->dta_ = (void*)mode;
    cmd->set_status(0);
    cmd->set_dialogId(dialogId);
    cmd->postfix();
    return cmd;
}

std::auto_ptr<SmppCommand> SmppCommand::makeUnbindResp(uint32_t dialogId,uint32_t status,void* data)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = UNBIND_RESP;
    cmd->dta_ = data;
    cmd->set_status( status );
    cmd->set_dialogId( dialogId );
    cmd->postfix();
    return cmd;
}

std::auto_ptr<SmppCommand> SmppCommand::makeReplaceSmResp(uint32_t dialogId,uint32_t status)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = REPLACE_RESP;
    cmd->set_status( status );
    cmd->set_dialogId( dialogId );
    cmd->postfix();
    return cmd;
}

std::auto_ptr<SmppCommand> SmppCommand::makeQuerySmResp( uint32_t dialogId,uint32_t status,
                                                         const char* id,
                                                         time_t findate,uint8_t state,uint8_t netcode)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = QUERY_RESP;
    cmd->dta_ = new QuerySmResp(id,findate,state,netcode);
    cmd->set_dialogId(dialogId);
    cmd->set_status(status);
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeCancelSmResp(uint32_t dialogId,uint32_t status)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = CANCEL_RESP;
    cmd->set_status( status );
    cmd->set_dialogId(dialogId);
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeCancel(SMSId id,const Address& oa,const Address& da)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = CANCEL;
    cmd->dta_ = new CancelSm(id,oa,da);
    cmd->set_dialogId(0);
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeCancel(SMSId id)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = CANCEL;
    cmd->dta_ = new CancelSm(id);
    cmd->set_dialogId(0);
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeBindCommand(const char* sysId,const char* pwd,const char* addrRange,const char* sysType)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = BIND_TRANSCEIVER;
    cmd->dta_ = new BindCommand(sysId,pwd,addrRange,sysType);
    cmd->set_dialogId(1);
    cmd->postfix();
    return cmd;
}


std::auto_ptr<SmppCommand> SmppCommand::makeCommand(CommandId cmdId,uint32_t dialogId,uint32_t status,void* data)
{
    std::auto_ptr<SmppCommand> cmd(new SmppCommand);
    cmd->cmdid_ = cmdId;
    cmd->set_status(status);
    cmd->dta_ = data;
    cmd->set_dialogId(dialogId);
    cmd->postfix();
    return cmd;
}


/*
template <class Pdu> inline void dumpPdu( smsc::logger::Logger* logg,
                                          const char* fmt,
                                          const Pdu*  pdu )
{
    if (logg->isDebugEnabled()) {
        util::PrintStdString dump;
        const_cast<Pdu*>(pdu)->dump(&dump);
        smsc_log_debug(logg,fmt,dump.buf());
    }
}


void dumpSms( smsc::logger::Logger* logg,
              const char* where,
              SMS* sms )
{
    if (!logg->isDebugEnabled()) return;
    util::HexDump hd;
    util::HexDump::string_type dump;
    Body& body = sms->getMessageBody();
    hd.hexdump(dump,body.getBuffer(),body.getBufferLength());
    util::PrintStdString pss;
    body.Print(&pss);
    smsc_log_debug(logg,"sms %s: sz=%u %s %s",
                   where, unsigned(body.getBufferLength()),
                   hd.c_str(dump), pss.buf());
}
 */


void SmppCommand::makeSMSBody(SMS* sms,const SmppHeader* pdu,uint32_t smeFlags)
{
    const PduXSm* xsm = reinterpret_cast<const PduXSm*>(pdu);
    // dumpPdu(log_,"pduXsm at input: %s",xsm);
    fetchSmsFromSmppPdu((PduXSm*)xsm,sms,smeFlags);
    // dumpSms(log_,"after conversion",sms);
    SMS &s=*sms;
    if(s.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && s.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
    {
        unsigned len;
        const unsigned char* data;
        if(s.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
        {
            data=(const unsigned char*)s.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
        }else
        {
            data=(const unsigned char*)s.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        }
        if(len==0 || *data>len)
        {
            throw smsc::util::Exception("SmppCommand: Invalid pdu (udhi length > message length)");
        }
    }
}


std::string SmppCommand::getMessageBody( SMS& data )
{
    unsigned len = 0;
    const char * buff = 0;
    std::string str;

    if (data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE)) 
        buff = data.getBinProperty(Tag::SMPP_SHORT_MESSAGE, &len);
    if (!len && data.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD)) 
        buff = data.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);

    if (!buff || !len) return str;

    if (data.hasIntProperty(Tag::SMPP_ESM_CLASS)) {
        const int esm = data.getIntProperty(Tag::SMPP_ESM_CLASS);
        if ( (esm & 0x40) ) {
            // udh is present, skip it
            const unsigned udhl = (unsigned(buff[0]) & 0xff)+1;
            if (udhl>=len) throw smsc::util::Exception("udh message is broken: len=%u udhl=%u", len, udhl);
            buff += udhl;
            len -= udhl;
        }
    }

    int code = smsc::smpp::DataCoding::SMSC7BIT;

    if (data.hasIntProperty(Tag::SMPP_DATA_CODING)) 
        code = data.getIntProperty(Tag::SMPP_DATA_CODING);

    switch (code) 
    {
    case smsc::smpp::DataCoding::SMSC7BIT:
        util::encodings::Convertor::GSM7BitToUTF8(buff,len,str);
        break;
    case smsc::smpp::DataCoding::LATIN1:
        util::encodings::Convertor::KOI8RToUTF8(buff, len, str);
        break;
    case smsc::smpp::DataCoding::UCS2:
        util::encodings::Convertor::UCS2BEToUTF8((unsigned short *)buff, len / 2, str);
        break;
    default:
        util::encodings::Convertor::GSM7BitToUTF8(buff,len,str);
    }
    /*
    if (log_->isDebugEnabled()) {
        util::HexDump hd;
        util::HexDump::string_type dump;
        dump.reserve(len*7+20);
        hd.hexdump(dump,buff,len);
        hd.addstr(dump,"-> ");
        hd.hexdump(dump,str.c_str(),str.size());
        smsc_log_debug(log_,"msg body (%u)->(%u): %s", len, str.size(), hd.c_str(dump));
    }
     */
    return str;
}


const char* SmppCommand::getUDH( SMS& data )
{
    unsigned len = 0;
    const char* buff = 0;

    do {
        if (data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE)) 
            buff = data.getBinProperty(Tag::SMPP_SHORT_MESSAGE, &len);
        if (!len && data.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD)) 
            buff = data.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);

        if (!len) {buff = 0;}
        if (!buff) {break;}

        if ( data.hasIntProperty(Tag::SMPP_ESM_CLASS) ) {
            int esm = data.getIntProperty(Tag::SMPP_ESM_CLASS);
            if ( (esm & 0x40) ) {
                break;
            }
        }
        buff = 0;

    } while (false);
    return buff;
}


void SmppCommand::getSlicingParameters( SMS& sms, int& sarmr, int& currentIndex, int& lastIndex )
{
    sarmr = 0;
    currentIndex = 0;
    lastIndex = 0;
    uint8_t slicingType = router::SlicingType::NONE;

    do {

        if ( sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) &&
             sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS) &&
             sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM) ) {
            currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
            lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
            sarmr = (sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) & 0xffff);
            slicingType = router::SlicingType::SAR;
            break;
        }

        const unsigned char* udh = 
            reinterpret_cast<const unsigned char*>(getUDH(sms));
        if (!udh) {break;}
        const unsigned udhl = (unsigned(udh[0]) & 0xff)+1;
        if ( log_->isDebugEnabled() ) {
            util::HexDump hd;
            util::HexDump::string_type dump;
            hd.hexdump(dump,udh,udhl);
            smsc_log_debug(log_,"msg w/ udh: %s",hd.c_str(dump));
        }
        const unsigned char* udhend = udh+udhl;
        for ( const unsigned char* p = udh+1; p < udhend; ) {
            if ( 0x00 == p[0] ) {
                // udh8
                if ( p[1] != 3 ) {
                    smsc_log_warn(log_,"msg w/ udh8: wrong len: %u", p[1]);
                    break;
                }
                sarmr = p[2];
                lastIndex = p[3];
                currentIndex = p[4];
                slicingType = router::SlicingType::UDH8;
                break;
            } else if ( 0x08 == p[0] ) {
                // udh16
                if ( p[1] != 4 ) {
                    smsc_log_warn(log_,"msg w/ udh16: wrong len: %u", p[1]);
                    break;
                }
                sarmr = (unsigned(p[2]) << 8) | p[3] ;
                lastIndex = p[4];
                currentIndex = p[5];
                slicingType = router::SlicingType::UDH16;
                break;
            }

            p += p[1] + 2;
            // util::HexDump hd;
            // util::HexDump::string_type dump;
            // hd.hexdump(dump,udh,udhl);
            // smsc_log_warn(log_,"udh bit is found but header has is weird: %s", hd.c_str(dump) );
        } // loop over udh fields
    } while ( false );

    if ( slicingType != router::SlicingType::NONE ) {
        if ( sms.getConcatMsgRef() != 0 ) {
            smsc_log_debug(log_,"original sar taken from sms: %u -> %u", sarmr, sms.getConcatMsgRef() );
            sarmr = sms.getConcatMsgRef();
        }
        smsc_log_debug(log_,"sar/udh fields found: type/ref/idx/tot=%u/%x/%u/%u",
                       slicingType, sarmr, currentIndex, lastIndex );
        sarmr += slicingType * 0x10000;
    }
}


void SmppCommand::changeSliceRefNum( SMS& sms, uint32_t sarmr )
{
    const uint16_t slicingType(sarmr/0x10000);
    switch ( uint8_t(slicingType) ) {
    case router::SlicingType::SAR : {
        sms.setConcatMsgRef(sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM));
        sms.setIntProperty(Tag::SMPP_SAR_MSG_REF_NUM,sarmr & 0xffff);
        break;
    }
    case router::SlicingType::UDH8:
    case router::SlicingType::UDH16: {
        // get original message
        const char* buff;
        unsigned len;
        uint16_t field;
        if (sms.hasBinProperty(Tag::SMPP_SHORT_MESSAGE)) {
            field = Tag::SMPP_SHORT_MESSAGE;
            buff = sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE, &len);
        }
        if (!len && sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD)) {
            field = Tag::SMPP_MESSAGE_PAYLOAD;
            buff = sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
        }
        std::auto_ptr<unsigned char> newbuf(new unsigned char[len]);
        memcpy(newbuf.get(),buff,len);
        unsigned char* udh = newbuf.get();
        const size_t udhl = unsigned(newbuf.get()[0]) + 1;
        if ( udhl < 6 ) {
            smsc_log_warn(log_,"cannot change too small udh: sz=%u", udhl);
            break;
        }
        if ( log_->isDebugEnabled() ) {
            util::HexDump hd;
            util::HexDump::string_type dump;
            hd.hexdump(dump,udh,udhl);
            smsc_log_debug(log_,"changing udh: %s",hd.c_str(dump));
        }
        // drop properties
        sms.dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
        sms.dropProperty(Tag::SMSC_RAW_PAYLOAD);
        sms.dropProperty(Tag::SMPP_SHORT_MESSAGE);
        sms.dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
        // FIXME: find position in udh
        const unsigned char* udhend = udh + udhl;
        uint16_t oldRefNum;
        bool changed = false;
        for ( unsigned char* p = udh+1; p < udhend; ) {
            if ( *p == 0x00 && slicingType == router::SlicingType::UDH8 ) {
                if ( p[1] != 3 ) {
                    smsc_log_warn(log_,"wrong udh8 size in message: udh[2]=%u", p[1] );
                    break;
                }
                oldRefNum = p[2];
                p[2] = sarmr & 0xff;
                changed = true;
                break;
            } else if ( *p == 0x08 && slicingType == router::SlicingType::UDH16 ) {
                if ( p[1] != 4 ) {
                    smsc_log_warn(log_,"wrong udh16 size in message: udh[2]=%u", p[1] );
                    break;
                }
                oldRefNum = (uint16_t(p[2]) << 8) + p[3];
                p[2] = (sarmr >> 8) & 0xff;
                p[3] = sarmr & 0xff;
                changed = true;
                break;
            }
            p += p[1] + 2;
        }
        if ( !changed ) {
            smsc_log_warn(log_,"udh change slicing: field not found");
            break;
        }
        if ( log_->isDebugEnabled() ) {
            util::HexDump hd;
            util::HexDump::string_type dump;
            hd.hexdump(dump,udh,udhl);
            smsc_log_debug(log_,"udh changed: %s",hd.c_str(dump));
        }
        sms.setConcatMsgRef(oldRefNum);
        sms.setBinProperty(field,reinterpret_cast<const char*>(newbuf.get()),len);
        break;
    } // udh case
    default :
        smsc_log_warn(log_,"change slicing: cannot be here");
    }
}

// --- non-static


void SmppCommand::print( util::Print& p ) const
{
    if ( ! p.enabled() ) return;
    SmppCommand* that = const_cast< SmppCommand* >( this );
    switch ( cmdid_ ) {
    case SUBMIT:
    case DELIVERY:
    case DATASM: {
        SmsCommand& sc = that->get_smsCommand();
        p.print( "smppcmd=%p session=%p type=%d(%s) serial=%d svc=%d opid=%d seq/org=%d/%d %s(%s)->%s",
                 this, session_, cmdid_, commandIdName(cmdid_), shared_->uid,
                 serviceId_, opId_,
                 shared_->dialogId,
                 sc.get_orgDialogId(),
                 sc.orgSrc.toString().c_str(),
                 src_ent_->getSystemId(),
                 sc.orgDst.toString().c_str() );
        break;
    }
    case SUBMIT_RESP:
    case DELIVERY_RESP:
    case DATASM_RESP: {
        SmsResp* r = that->get_resp();
        assert( r );
        SmppCommand* orgcmd = r->getOrgCmd();
        if ( orgcmd ) {
            SmsCommand& sc = orgcmd->get_smsCommand();
            p.print( "smppcmd=%p session=%p type=%d(%s) serial=%d svc=%d opid=%d orgcmd=%p seq/org=%d/%d %s(%s)->%s",
                     this, session_, cmdid_, commandIdName(cmdid_),
                     shared_->uid,
                     serviceId_, opId_, orgcmd,
                     shared_->dialogId, sc.get_orgDialogId(),
                     sc.orgSrc.toString().c_str(),
                     src_ent_->getSystemId(),
                     sc.orgDst.toString().c_str() );
        } else {
            p.print( "smppcmd=%p session=%p type=%d(%s) serial=%d svc=%d opid=%d orgcmd=%p seq=%d ?(%s)->?",
                     this, session_, cmdid_, commandIdName(cmdid_),
                     shared_->uid,
                     serviceId_, opId_, orgcmd,
                     shared_->dialogId,
                     src_ent_->getSystemId() );
        }
        break;
    }
    default:
        p.print( "smppcmd=%p session=%p type=%d(%s) serial=%d svc=%d opid=%d seq=%d ?(%s)->?",
                 this, session_, cmdid_, commandIdName(cmdid_), shared_->uid,
                 serviceId_, opId_, shared_->dialogId,
                 src_ent_->getSystemId() );
    } // switch
}


SmppCommand::SmppCommand( SmppHeader* pdu, uint32_t smeFlags ) :
SCAGCommand(), _SmppCommand()
{
    getlogger();
    __require__ ( pdu != NULL );
    try {

        switch ( pdu->commandId )
        {
        case SmppCommandSet::QUERY_SM:
            {
                cmdid_ = QUERY;
                dta_ = new QuerySm(reinterpret_cast<PduQuerySm*>(pdu));
                goto end_construct;
            }
        case SmppCommandSet::QUERY_SM_RESP:
            {
                cmdid_ = QUERY_RESP;
                PduQuerySmResp* resp = reinterpret_cast<PduQuerySmResp*>(pdu);
                // uint64_t id = 0;
                // if (resp->messageId.size()) sscanf(resp->messageId.cstr(),"%lld",&id);
                time_t findate=0;
                if (resp->finalDate.size()) findate=smppTime2CTime(resp->finalDate);
                set_status(resp->get_header().get_commandStatus());
                dta_ = new QuerySmResp(resp->messageId.cstr(),findate,resp->get_messageState(),resp->get_errorCode());
                goto end_construct;
            }
        case SmppCommandSet::DATA_SM:
            {
                cmdid_ = DATASM;
                PduDataSm* dsm = reinterpret_cast<PduDataSm*>(pdu);
                dta_ = new SmsCommand;
                if ( !fetchSmsFromDataSmPdu( dsm, get_sms(), smeFlags) )
                    throw smsc::util::Exception("Invalid data coding");
                get_sms()->setIntProperty(Tag::SMPP_DATA_SM,1);
                shared_ = & get_smsCommand();
                goto end_construct;
            }
        case SmppCommandSet::SUBMIT_SM:
            {
                cmdid_ = SUBMIT;
                goto sms_pdu;
            }
        case SmppCommandSet::DELIVERY_SM:
            {
                cmdid_ = DELIVERY;
                goto sms_pdu;
            }
        case SmppCommandSet::SUBMIT_SM_RESP:
            {
                cmdid_ = SUBMIT_RESP;
                goto sms_resp;
            }
        case SmppCommandSet::DATA_SM_RESP:
            {
                cmdid_ = DATASM_RESP;
                PduDataSmResp* xsm = reinterpret_cast<PduDataSmResp*>(pdu);
                SmsResp* resp = new SmsResp( xsm->header.get_commandStatus() );
                dta_ = resp;
                resp->set_messageId(xsm->get_messageId());
                resp->set_dataSm();
                set_status( xsm->header.get_commandStatus() );

                if (xsm->optional.has_additionalStatusInfoText()) {
                    resp->setAdditionalStatusInfoText(xsm->optional.get_additionalStatusInfoText());
                }
                if (xsm->optional.has_deliveryFailureReason()) {
                    resp->setDeliveryFailureReason(xsm->optional.get_deliveryFailureReason());
                }
                if (xsm->optional.has_dpfResult()) {
                    resp->setDpfResult(xsm->optional.get_dpfResult());
                }
                if (xsm->optional.has_networkErrorCode()) {
                    uint32_t nec = 0;
                    memcpy((uint8_t*)&nec + 1, xsm->optional.get_networkErrorCode(), 3);
                    resp->setNetworkErrorCode(ntohl(nec));
                }
                goto end_construct;
            }
        case SmppCommandSet::DELIVERY_SM_RESP:
            {
                cmdid_ = DELIVERY_RESP;
                goto sms_resp;
            }
        case SmppCommandSet::REPLACE_SM:
            {
                cmdid_ = REPLACE;
                dta_ = new ReplaceSm(reinterpret_cast<PduReplaceSm*>(pdu));
                goto end_construct;
            }
        case SmppCommandSet::REPLACE_SM_RESP:
            {
                cmdid_ = REPLACE_RESP;
                set_status( pdu->get_commandStatus() );
                // set_dialogId( pdu->get_sequenceNumber() );
                goto end_construct;
            }
        case SmppCommandSet::CANCEL_SM:
            {
                cmdid_ = CANCEL;
                dta_ = new CancelSm(reinterpret_cast<PduCancelSm*>(pdu));
                goto end_construct;
            }
        case SmppCommandSet::CANCEL_SM_RESP:
            {
                cmdid_ = CANCEL_RESP;
                set_status( pdu->get_commandStatus() );
                // _cmd->dialogId=pdu->get_sequenceNumber();
                goto end_construct;
            }
        case SmppCommandSet::ENQUIRE_LINK:
            {
                cmdid_ = ENQUIRELINK;
                goto end_construct;
            }
        case SmppCommandSet::ALERT_NOTIFICATION:
            {
                cmdid_ = ALERT_NOTIFICATION;
                dta_ = new AlertNotification((PduAlertNotification*)pdu);
                goto end_construct;
            }
        case SmppCommandSet::ENQUIRE_LINK_RESP:
            {
                cmdid_ = ENQUIRELINK_RESP;
                dta_ = (void*)pdu->get_commandStatus();
                goto end_construct;
            }
        case SmppCommandSet::SUBMIT_MULTI:
            {
                PduMultiSm* pduX = reinterpret_cast<PduMultiSm*>(pdu);
                cmdid_ = SUBMIT_MULTI_SM;
                SubmitMultiSm* sm = new SubmitMultiSm;
                dta_ = sm;
                makeSMSBody( &(sm->msg), pdu, smeFlags );
                unsigned u = 0;
                unsigned uu = pduX->message.numberOfDests;
                for ( ; u < uu; ++u )
                {
                    sm->dests[u].dest_flag = (pduX->message.dests[u].flag == 0x02);
                    __trace2__(":SUBMIT_MULTI_COMMAND: dest_flag = %d",pduX->message.dests[u].flag);
                    if ( pduX->message.dests[u].flag == 0x01 ) // SME address
                    {
                        sm->dests[u].value = pduX->message.dests[u].get_value();
                        sm->dests[u].ton = pduX->message.dests[u].get_typeOfNumber();
                        sm->dests[u].npi = pduX->message.dests[u].get_numberingPlan();
                    }
                    else if (pduX->message.dests[u].flag == 0x02)// Distribution list
                    {
                        sm->dests[u].value = pduX->message.dests[u].get_value();
                    }
                    else
                    {
                        __warning2__("submitmulti has invalid address flag 0x%x",pduX->message.dests[u].flag);
                        throw runtime_error("submitmulti has invalid address flag");
                    }
                }
                sm->number_of_dests = uu;
                goto end_construct;
            }
        default: throw smsc::util::Exception("Unsupported command id %08X",pdu->commandId);
        }
        //__unreachable__("command id is not processed");

        sms_pdu:
        {
            dta_ = new SmsCommand;
            shared_ = &get_smsCommand();
            makeSMSBody( get_sms(), pdu, smeFlags );
            goto end_construct;
        }
        sms_resp:
        {
            PduXSmResp* xsm = reinterpret_cast<PduXSmResp*>(pdu);
            SmsResp* resp = new SmsResp( xsm->header.get_commandStatus() );
            dta_ = resp;
            resp->set_messageId(xsm->get_messageId());
            set_status(xsm->header.get_commandStatus());

      /*
      if(pdu->commandId==SmppCommandSet::DELIVERY_SM_RESP || pdu->commandId==SmppCommandSet::DATA_SM_RESP)
      {
        int status=((SmsResp*)_cmd->dta)->get_status()&0xffff;
        if(status==SmppStatusSet::ESME_ROK)
        {
          ((SmsResp*)_cmd->dta)->set_status(
            MAKE_COMMAND_STATUS(CMD_OK,status)
            );
        }else
        if(smsc::system::Status::isErrorPermanent(status))
        {
          ((SmsResp*)_cmd->dta)->set_status(
            MAKE_COMMAND_STATUS(CMD_ERR_PERM,status)
            );
        }else
        {
          ((SmsResp*)_cmd->dta)->set_status(
            MAKE_COMMAND_STATUS(CMD_ERR_TEMP,status)
            );
        }
      }
      */
      /*
      if(pdu->commandId == SmppCommandSet::DATA_SM_RESP)
      {
        ((SmsResp*)_cmd->dta)->set_dataSm();
      }
      */
            goto end_construct;
        }
        end_construct:
        set_dialogId( pdu->get_sequenceNumber() );
        postfix();

    } catch (...) {

        dispose();

    }
    return;
}


SmppHeader* SmppCommand::makePdu(uint32_t smeFlags)
{
    switch ( getCommandId() )
    {
    case SUBMIT:
        {
            auto_ptr<PduXSm> xsm(new PduXSm);
            xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM);
            xsm->header.set_sequenceNumber(get_dialogId());
            // dumpSms(log_,"before conversion",get_sms());
            fillSmppPduFromSms(xsm.get(), get_sms(),smeFlags);
            // dumpPdu(log_,"makePdu (submit): %s",xsm.get());
            return reinterpret_cast<SmppHeader*>(xsm.release());
        }
    case DELIVERY:
        {
            if ( get_sms()->getIntProperty(Tag::SMPP_DATA_SM) ) {
                auto_ptr<PduDataSm> xsm(new PduDataSm);
                xsm->header.set_commandId(SmppCommandSet::DATA_SM);
                xsm->header.set_sequenceNumber(get_dialogId());
                fillDataSmFromSms(xsm.get(),get_sms(),smeFlags);
                return reinterpret_cast<SmppHeader*>(xsm.release());
            } else {
                auto_ptr<PduXSm> xsm(new PduXSm);
                xsm->header.set_commandId(SmppCommandSet::DELIVERY_SM);
                xsm->header.set_sequenceNumber(get_dialogId());
                // dumpSms(log_,"before conversion",get_sms());
                fillSmppPduFromSms(xsm.get(),get_sms(),smeFlags);
                xsm->message.set_scheduleDeliveryTime("");
                xsm->message.set_validityPeriod("");
                xsm->message.set_replaceIfPresentFlag(0);
                // dumpPdu(log_,"makePdu (delivery): %s",xsm.get());
                return reinterpret_cast<SmppHeader*>(xsm.release());
            }
        }
    case SUBMIT_RESP:
        {
            if ( get_resp()->get_dataSm() ) {
                auto_ptr<PduDataSmResp> xsm(new PduDataSmResp);
                xsm->header.set_commandId(SmppCommandSet::DATA_SM_RESP);
                xsm->header.set_sequenceNumber(get_dialogId());
                xsm->header.set_commandStatus(makeSmppStatus(get_status()));
                xsm->set_messageId(get_resp()->get_messageId());
                return reinterpret_cast<SmppHeader*>(xsm.release());
            } else {
                auto_ptr<PduXSmResp> xsm(new PduXSmResp);
                xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM_RESP);
                xsm->header.set_sequenceNumber(get_dialogId());
                xsm->header.set_commandStatus(makeSmppStatus(get_status()));
                xsm->set_messageId(get_resp()->get_messageId());
                return reinterpret_cast<SmppHeader*>(xsm.release());
            }
        }
    case SUBMIT_MULTI_SM_RESP:
        {
            auto_ptr<PduMultiSmResp> xsmR(new PduMultiSmResp);
            xsmR->header.set_commandId(SmppCommandSet::SUBMIT_MULTI_RESP);
            xsmR->header.set_sequenceNumber(get_dialogId());
            SubmitMultiResp* mr = get_MultiResp();
            xsmR->header.set_commandStatus(makeSmppStatus(get_status()));
            xsmR->set_messageId(mr->get_messageId());
            if ( mr->no_unsuccess ) {
                std::vector<UnsuccessDeliveries> ud;
                ud.resize(mr->no_unsuccess);
                for ( unsigned u=0; u<mr->no_unsuccess; ++u )
                {
                    ud[u].addr.set_numberingPlan(mr->unsuccess[u].addr.getNumberingPlan());
                    ud[u].addr.set_typeOfNumber(mr->unsuccess[u].addr.getTypeOfNumber());
                    ud[u].addr.set_value(mr->unsuccess[u].addr.value);
                    ud[u].errorStatusCode = mr->unsuccess[u].errcode;
                }
                xsmR->set_sme(&ud[0],mr->no_unsuccess);
                xsmR->set_noUnsuccess(mr->no_unsuccess);
            }
            return reinterpret_cast<SmppHeader*>(xsmR.release());
        }
    case DELIVERY_RESP:
        {
            auto_ptr<PduXSmResp> xsm(new PduXSmResp);
            xsm->header.set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
            xsm->header.set_sequenceNumber(get_dialogId());
            xsm->header.set_commandStatus(makeSmppStatus(get_status()));
            xsm->set_messageId(get_resp()->get_messageId());
            return reinterpret_cast<SmppHeader*>(xsm.release());
        }
    case DATASM:
        {
            auto_ptr<PduDataSm> xsm(new PduDataSm);
            xsm->header.set_commandId(SmppCommandSet::DATA_SM);
            xsm->header.set_sequenceNumber(get_dialogId());
            fillDataSmFromSms(xsm.get(),get_sms(),smeFlags);
            return reinterpret_cast<SmppHeader*>(xsm.release());
        }
    case DATASM_RESP:
        {
            auto_ptr<PduDataSmResp> xsm(new PduDataSmResp);
            xsm->header.set_commandId(SmppCommandSet::DATA_SM_RESP);
            xsm->header.set_sequenceNumber(get_dialogId());
            xsm->header.set_commandStatus(makeSmppStatus(get_status()));
            xsm->set_messageId(get_resp()->get_messageId());
            SmsResp* resp = get_resp();
            if (resp->hasAdditionalStatusInfoText()) {
                xsm->optional.set_additionalStatusInfoText(resp->getAdditionalStatusInfoText());
            }
            if (resp->hasDeliveryFailureReason()) {
                xsm->optional.set_deliveryFailureReason(resp->getDeliveryFailureReason());
            }
            if (resp->hasDpfResult()) {
                xsm->optional.set_dpfResult(resp->getDpfResult());
            }
            if (resp->hasNetworkErrorCode()) {
                uint32_t nec = htonl(resp->getNetworkErrorCode());
                xsm->optional.set_networkErrorCode((uint8_t*)&nec + 1);
            }
            return reinterpret_cast<SmppHeader*>(xsm.release());
        }
    case GENERIC_NACK:
        {
            auto_ptr<PduGenericNack> gnack(new PduGenericNack);
            gnack->header.set_commandId(SmppCommandSet::GENERIC_NACK);
            gnack->header.set_sequenceNumber(get_dialogId());
            gnack->header.set_commandStatus(makeSmppStatus(get_status()));
            return reinterpret_cast<SmppHeader*>(gnack.release());
        }
    case UNBIND:
        {
            auto_ptr<PduUnbind> unb(new PduUnbind);
            unb->header.set_commandId(SmppCommandSet::UNBIND);
            unb->header.set_sequenceNumber(get_dialogId());
            unb->header.set_commandStatus(0);
            return reinterpret_cast<SmppHeader*>(unb.release());
        }
    case UNBIND_RESP:
        {
            auto_ptr<PduUnbindResp> unb(new PduUnbindResp);
            unb->header.set_commandId(SmppCommandSet::UNBIND_RESP);
            unb->header.set_sequenceNumber(get_dialogId());
            unb->header.set_commandStatus(makeSmppStatus(get_status()));
            return reinterpret_cast<SmppHeader*>(unb.release());
        }
    case REPLACE:
        {
            auto_ptr<PduReplaceSm> rpl(new PduReplaceSm);
            rpl->header.set_commandId(SmppCommandSet::REPLACE_SM);
            rpl->header.set_sequenceNumber(get_dialogId());
            rpl->header.set_commandStatus(0);
            const ReplaceSm *r=&get_replaceSm();
            rpl->set_messageId(r->messageId.get());
            rpl->set_source(Address2PduAddress(r->sourceAddr.get()));
            char buf[64];
            cTime2SmppTime(r->scheduleDeliveryTime,buf);
            rpl->set_scheduleDeliveryTime(buf);
            cTime2SmppTime(r->validityPeriod,buf);
            rpl->set_validityPeriod(buf);
            rpl->set_registredDelivery(r->registeredDelivery);
            rpl->set_smDefaultMsgId(r->smDefaultMsgId);
            rpl->shortMessage.copy(r->shortMessage.get(),r->smLength);
            return reinterpret_cast<SmppHeader*>(rpl.release());
        }
    case REPLACE_RESP:
        {
            auto_ptr<PduReplaceSmResp> repl(new PduReplaceSmResp);
            repl->header.set_commandId(SmppCommandSet::REPLACE_SM_RESP);
            repl->header.set_sequenceNumber(get_dialogId());
            repl->header.set_commandStatus(makeSmppStatus(get_status()));
            return reinterpret_cast<SmppHeader*>(repl.release());
        }
    case QUERY:
        {
            auto_ptr<PduQuerySm> query(new PduQuerySm);
            query->header.set_commandId(SmppCommandSet::QUERY_SM);
            query->header.set_sequenceNumber(get_dialogId());
            query->header.set_commandStatus(makeSmppStatus(get_status()));
            query->set_messageId(get_querySm().messageId.get());
            query->set_source(Address2PduAddress(get_querySm().sourceAddr.get()));
            return reinterpret_cast<SmppHeader*>(query.release());
        }
    case QUERY_RESP:
        {
            auto_ptr<PduQuerySmResp> qresp(new PduQuerySmResp);
            qresp->header.set_commandId(SmppCommandSet::QUERY_SM_RESP);
            qresp->header.set_sequenceNumber(get_dialogId());
            ((QuerySmResp*)dta_)->fillPdu(qresp.get());
            qresp->get_header().set_commandStatus
                ( makeSmppStatus(get_status()) );
            return reinterpret_cast<SmppHeader*>(qresp.release());
        }
    case CANCEL:
        {
            auto_ptr<PduCancelSm> cnc(new PduCancelSm);
            cnc->header.set_commandId(SmppCommandSet::CANCEL_SM);
            cnc->header.set_sequenceNumber(get_dialogId());
            cnc->header.set_commandStatus(makeSmppStatus(get_status()));

            if(get_cancelSm().serviceType.get())
                cnc->set_serviceType(get_cancelSm().serviceType.get());
            if(get_cancelSm().messageId.get())
                cnc->set_messageId(get_cancelSm().messageId.get());

            if(get_cancelSm().sourceAddr.get())
                cnc->set_source(Address2PduAddress(get_cancelSm().sourceAddr.get()));
            if(get_cancelSm().destAddr.get())
                cnc->set_dest(Address2PduAddress(get_cancelSm().destAddr.get()));

            return reinterpret_cast<SmppHeader*>(cnc.release());
        }
    case CANCEL_RESP:
        {
            auto_ptr<PduCancelSmResp> cresp(new PduCancelSmResp);
            cresp->header.set_commandId(SmppCommandSet::CANCEL_SM_RESP);
            cresp->header.set_sequenceNumber(get_dialogId());
            cresp->header.set_commandStatus(makeSmppStatus(get_status()));
            return reinterpret_cast<SmppHeader*>(cresp.release());
        }
    case ENQUIRELINK:
        {
            auto_ptr<PduEnquireLink> pdu(new PduEnquireLink);
            pdu->header.set_commandId(SmppCommandSet::ENQUIRE_LINK);
            pdu->header.set_sequenceNumber(get_dialogId());
            pdu->header.set_commandStatus(0);
            return reinterpret_cast<SmppHeader*>(pdu.release());
        }
    case ENQUIRELINK_RESP:
        {
            auto_ptr<PduEnquireLink> pdu(new PduEnquireLinkResp);
            pdu->header.set_commandId(SmppCommandSet::ENQUIRE_LINK_RESP);
            pdu->header.set_sequenceNumber(get_dialogId());
            pdu->header.set_commandStatus(makeSmppStatus(get_status()));
            return reinterpret_cast<SmppHeader*>(pdu.release());
        }
    case BIND_TRANSCEIVER:
        {
            auto_ptr<PduBindTRX> pdu(new PduBindTRX);
            pdu->header.set_commandId(SmppCommandSet::BIND_TRANCIEVER);
            pdu->header.set_sequenceNumber(get_dialogId());
            pdu->header.set_commandStatus(makeSmppStatus(get_status()));
            BindCommand& bnd = get_bindCommand();
            pdu->set_systemId(bnd.sysId.c_str());
            pdu->set_password(bnd.pass.c_str());
            pdu->set_interfaceVersion(0x34);
            pdu->set_systemType(bnd.systemType.c_str());
            int ton=0, npi=0;
            char addr[64]={0,};
            sscanf(bnd.addrRange.c_str(),".%d.%d.%64s",&ton,&npi,addr);
            pdu->get_addressRange().set_typeOfNumber(ton);
            pdu->get_addressRange().set_numberingPlan(npi);
            pdu->get_addressRange().set_value(addr);
            return reinterpret_cast<SmppHeader*>(pdu.release());
        }
    case BIND_RECIEVER_RESP:
        {
            auto_ptr<PduBindTRXResp> pdu(new PduBindTRXResp);
            pdu->header.set_commandId(SmppCommandSet::BIND_RECIEVER_RESP);
            pdu->header.set_sequenceNumber(get_dialogId());
            pdu->header.set_commandStatus(get_status());
            return reinterpret_cast<SmppHeader*>(pdu.release());
        }
    case BIND_TRANSMITTER_RESP:
        {
            auto_ptr<PduBindTRXResp> pdu(new PduBindTRXResp);
            pdu->header.set_commandId(SmppCommandSet::BIND_TRANSMITTER_RESP);
            pdu->header.set_sequenceNumber(get_dialogId());
            pdu->header.set_commandStatus(get_status());
            return reinterpret_cast<SmppHeader*>(pdu.release());
        }
    case BIND_TRANCIEVER_RESP:
        {
            auto_ptr<PduBindTRXResp> pdu(new PduBindTRXResp);
            pdu->header.set_commandId(SmppCommandSet::BIND_TRANCIEVER_RESP);
            pdu->header.set_sequenceNumber(get_dialogId());
            pdu->header.set_commandStatus(get_status());
            return reinterpret_cast<SmppHeader*>(pdu.release());
        }
    case ALERT_NOTIFICATION:
        {
            auto_ptr<PduAlertNotification> pdu(new PduAlertNotification);
            AlertNotification& an = get_alertNotification();
            pdu->header.set_commandId(SmppCommandSet::ALERT_NOTIFICATION);
            pdu->header.set_sequenceNumber(get_dialogId());
            pdu->header.set_commandStatus(0);
            pdu->source.set_typeOfNumber(an.src.type);
            pdu->source.set_numberingPlan(an.src.plan);
            pdu->source.set_value(an.src.value);
            pdu->esme.set_typeOfNumber(an.dst.type);
            pdu->esme.set_numberingPlan(an.dst.plan);
            pdu->esme.set_value(an.dst.value);
            pdu->optional.set_msAvailableStatus(an.status);
            return reinterpret_cast<SmppHeader*>(pdu.release());
        }

    default:
        throw runtime_error("unknown commandid");
    }
    return 0; // for compiler
}


std::auto_ptr< SmppCommand > SmppCommand::clone()
{
    if ( cmdid_ != SUBMIT && cmdid_ != DELIVERY && cmdid_ != DATASM ) {
        smsc_log_error(log_, "cloning is allowed for submit/deliver/datasm only" );
        ::abort();
    }
    return std::auto_ptr< SmppCommand >(new SmppCommand(*this));
}


SmppCommand::SmppCommand() :
SCAGCommand(), _SmppCommand()
{
    getlogger();
}


SmppCommand::SmppCommand( const SmppCommand& c ) :
SCAGCommand(c), _SmppCommand(c)
{
    if ( cmdid_ != SUBMIT && cmdid_ != DELIVERY && cmdid_ != DATASM ) {
        smsc_log_error(log_, "cloning is allowed for submit/deliver/datasm only" );
        ::abort();
    }
    const unsigned clones = get_smsCommand().ref();
    // no need to assign uid
    smsc_log_debug( log_, "Command create (clone): cmd=%p, type=%d(%s), serial=%u, seq=%d, clones=%u",
                    this, cmdid_, commandIdName(cmdid_),
                    shared_->uid,
                    shared_->dialogId,
                    clones );
    session_ = 0;
}


void SmppCommand::dispose()
{
    if ( cmdid_ == DELIVERY || cmdid_ == SUBMIT || cmdid_ == DATASM ) {
        if ( !dta_ ) return;
        const uint32_t uid = shared_->uid;
        const uint32_t dialogId = shared_->dialogId;
        const unsigned clones = get_smsCommand().unref();
        if ( clones ) {
            smsc_log_debug(log_, "Command destroy: cmd=%p, type=%d(%s), serial=%u, seq=%u, %u clones still exist",
                           this, cmdid_, commandIdName(cmdid_), uid, dialogId, clones );
            return;
        }
    }

    if ( shared_ ) {
        if ( shared_->uid == 0 ) {
            smsc_log_warn(log_, "destroying command %p with serial=0, exception?", this );
        } else if ( log_->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG ) ) {
            uint32_t sc = 0;
            {
                MutexGuard mtxx(cntMutex);
                sc = --commandCounter;
            }
            if ( cmdid_ != PROCESSEXPIREDRESP ) {
                smsc_log_debug(log_, "Command destroy: cmd=%p, type=%d(%s), count=%u, serial=%u, seq=%u",
                               this, cmdid_, commandIdName(cmdid_), sc, shared_->uid, shared_->dialogId );
            }
        }
    }

    switch ( cmdid_ )
    {
    case DELIVERY:
    case SUBMIT:
    case DATASM:
        delete reinterpret_cast<SmsCommand*>(dta_);
        break;

    case SUBMIT_MULTI_SM:
        delete reinterpret_cast<SubmitMultiSm*>(dta_);
        break;

    case SUBMIT_MULTI_SM_RESP:
        delete reinterpret_cast<SubmitMultiResp*>(dta_);
        break;

    case DELIVERY_RESP:
    case SUBMIT_RESP:
    case DATASM_RESP:
        delete reinterpret_cast<SmsResp*>(dta_);
        break;

    case REPLACE:
        delete reinterpret_cast<ReplaceSm*>(dta_);
        break;

    case QUERY:
        delete reinterpret_cast<QuerySm*>(dta_);
        break;

    case QUERY_RESP:
        delete reinterpret_cast<QuerySmResp*>(dta_);
        break;

    case CANCEL:
        delete reinterpret_cast<CancelSm*>(dta_);
        break;

    case BIND_TRANSCEIVER:
        delete reinterpret_cast<BindCommand*>(dta_);
        break;

    case ALERT_NOTIFICATION:
        delete reinterpret_cast<AlertNotification*>(dta_);
        break;

    case UNKNOWN:
    case GENERIC_NACK:
    case UNBIND_RESP:
    case REPLACE_RESP:
    case CANCEL_RESP:
    case ENQUIRELINK:
    case ENQUIRELINK_RESP:
    case UNBIND:
    case BIND_RECIEVER_RESP:
    case BIND_TRANSMITTER_RESP:
    case BIND_TRANCIEVER_RESP:
    case PROCESSEXPIREDRESP:
        // nothing to delete
        break;
    default:
        __warning2__("~SmppCommand:unprocessed cmdid %d",cmdid_);
    }
}


void SmppCommand::postfix()
{
    if ( ! shared_ ) return;
    {
        MutexGuard mg(cntMutex);
        ++commandCounter;
    }
    // if ( ++stuid == uint32_t(-1) ) ++stuid;
    shared_->uid = makeSerial();
    if ( cmdid_ != PROCESSEXPIREDRESP ) {
        smsc_log_debug( log_, "Command create: cmd=%p, type=%d(%s), count=%u, serial=%u, seq=%u",
                        this, cmdid_, commandIdName(cmdid_), commandCounter, shared_->uid, shared_->dialogId );
    }
}


SmsResp::SmsResp( uint32_t origstatus ) :
messageId(0),
dataSm(false),
origStatus_(origstatus),
dir(dsdUnknown),
orgCmd(0),
deliveryFailureReason(0),
dpfResult(0),
networkErrorCode(0)
{
    expiredUid = 0;
    expiredResp = false;
    bHasDeliveryFailureReason = false;
    bHasAdditionalStatusInfoText = false;
    bHasDpfResult = false;
    bHasNetworkErrorCode = false;
    logger = Logger::getInstance("smppResp");
};


SmsResp::~SmsResp()
{
    if ( messageId ) delete [] messageId;
    // if ( sms ) delete sms;
    if ( orgCmd ) delete orgCmd;
}


void SmsResp::setOrgCmd(SmppCommand* o)
{
    if ( o ) {
        if ( orgCmd && orgCmd != o ) delete orgCmd;
        orgCmd = o;
    }
}

} //smpp
} //transport
} //scag
