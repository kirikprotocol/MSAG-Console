#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "Statistics2.h"
#include "util/BufferSerialization.hpp"
//#include "scag/transport/smpp/base/SmppCommandIds.h"
#include "scag/re/base/EventHandlerType.h"
#include <iosfwd>
#include <sstream>


using namespace scag2::stat;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< Statistics, OuterCreation > Single;
bool inited = false;
Mutex mtx;

/// NOTE: statistics is required for everything, except Config.
inline unsigned GetLongevity( Statistics* ) { return 250; }

}


namespace{

//from scag/transport/smpp/base/SmppCommand2.cpp
using namespace scag2::re;
const char* commandIdName( int id )
{
#define CMDIDNAME(x) case x: return #x
    switch (id) {
    CMDIDNAME(EH_UNKNOWN);
    CMDIDNAME(EH_SUBMIT_SM);
    CMDIDNAME(EH_SUBMIT_SM_RESP);
    CMDIDNAME(EH_DELIVER_SM);
    CMDIDNAME(EH_DELIVER_SM_RESP);
    // CMDIDNAME(EH_RECEIPT);

    CMDIDNAME(EH_HTTP_REQUEST);
    CMDIDNAME(EH_HTTP_RESPONSE);
    CMDIDNAME(EH_HTTP_DELIVERY);

    CMDIDNAME(EH_DATA_SM);
    CMDIDNAME(EH_DATA_SM_RESP);

    CMDIDNAME(EH_SESSION_INIT);
    CMDIDNAME(EH_SESSION_DESTROY);
    default: return "???";
    }
#undef CMDIDNAME
}
}

namespace scag2 {
namespace stat {

// ============================================================
SaccSerialBuffer::SaccSerialBuffer(char* data, size_t size):buff_() {
  buff_.setExtBuf(data, size);
}

void SaccSerialBuffer::readStr(std::string& s)
{
//  uint16_t len = (s.length() > maxLen) ? maxLen : uint16_t(s.length());
//  buff_ << len;
//  buff_.Append(s.data(), len);
  uint16_t len;
  buff_ >> len;
  s.assign(buff_.GetCurPtr(),len);
  buff_.SetPos(buff_.GetPos()+len);
}

void SaccSerialBuffer::readInt16(uint16_t& i)
{
  uint16_t len;
  buff_ >> len;
  buff_ >> i;
}

void SaccSerialBuffer::ReadNetInt16(uint16_t& i) {
  //buff_ << i;
  buff_ >> i;
}

void SaccSerialBuffer::readInt32(uint32_t& i)
{
//  uint16_t len = static_cast<uint16_t>(sizeof(uint32_t));
//  buff_ << len;
//  buff_ << i;
  uint16_t len;
  buff_ >> len;
  buff_ >> i;
}

void SaccSerialBuffer::ReadNetInt32(uint32_t& i) {
  //buff_ << i;
  buff_ << i;
}

void SaccSerialBuffer::readInt64(uint64_t& i)
{
//  uint16_t len = static_cast<uint16_t>(sizeof(uint64_t));
//  buff_ << len;
//  buff_ << i;
  uint16_t len;
  buff_ >> len;
  buff_ >> i;
}
void SaccSerialBuffer::readByte(uint8_t& i)
{
//  uint16_t len = static_cast<uint16_t>(sizeof(uint8_t));
//  buff_ << len;
//  buff_ << i;
  uint16_t len;
  buff_ >> len;
  buff_ >> i;
}

void SaccSerialBuffer::readFloat(float& i) {
//  uint16_t len = static_cast<uint16_t>(sizeof(float));
//  buff_ << len;
//  buff_ << i;
  uint16_t len;
  buff_ >> len;
  buff_ >> i;
}

void SaccSerialBuffer::writeStr(std::string& s, uint16_t maxLen)
{
    uint16_t len = (s.length() > maxLen) ? maxLen : uint16_t(s.length());
    //WriteNetInt16(len);
    //Write(s.c_str(), len);
    buff_ << len;
    buff_.Append(s.data(), len);
}
void SaccSerialBuffer::writeInt16(uint16_t i)
{
    //WriteNetInt16(sizeof(uint16_t));
    //WriteNetInt16(i);
  uint16_t len = static_cast<uint16_t>(sizeof(uint16_t));
  buff_ << len;
  buff_ << i;
}

void SaccSerialBuffer::WriteNetInt16(uint16_t i) {
  buff_ << i;
}

void SaccSerialBuffer::writeInt32(uint32_t i)
{
    //WriteNetInt16(sizeof(uint32_t));
    //WriteNetInt32(i);
  uint16_t len = static_cast<uint16_t>(sizeof(uint32_t));
  buff_ << len;
  buff_ << i;
}

void SaccSerialBuffer::WriteNetInt32(uint32_t i) {
  buff_ << i;
}

void SaccSerialBuffer::writeInt64(uint64_t i)
{
    //WriteNetInt16(sizeof(uint64_t));
    //WriteNetInt64(i);
  uint16_t len = static_cast<uint16_t>(sizeof(uint64_t));
  buff_ << len;
  buff_ << i;
}
void SaccSerialBuffer::writeByte(uint8_t i)
{
    //WriteNetInt16(sizeof(uint8_t));
    //WriteByte(i);
  uint16_t len = static_cast<uint16_t>(sizeof(uint8_t));
  buff_ << len;
  buff_ << i;
}

void SaccSerialBuffer::writeFloat(float i) {
  uint16_t len = static_cast<uint16_t>(sizeof(float));
  buff_ << len;
  buff_ << i;
}

bool SaccSerialBuffer::writeToSocket(Socket& sock)
{
    //uint32_t bsize = getPos();
    //setPos(0);
    //WriteNetInt32(bsize);
    //setPos(0);
    //return sock.WriteAll(getBuffer() ,bsize) > 0;
  uint32_t bsize = uint32_t(buff_.GetPos());
  buff_.SetPos(0);
  buff_ << bsize;
  buff_.SetPos(0);
  return sock.WriteAll(buff_.GetCurPtr() ,bsize) > 0;
}

void * SaccSerialBuffer::getBuffer() {
  return buff_.GetCurPtr();
}
uint32_t SaccSerialBuffer::getPos() {
  return uint32_t(buff_.GetPos());
}
void SaccSerialBuffer::setPos(uint32_t newPos) {
  buff_.SetPos(newPos);
}
uint32_t SaccSerialBuffer::getSize() {
  return uint32_t(buff_.getSize());
}

std::string SaccEventHeader::toString(SaccEvent::tostring mode){
  std::ostringstream res;

  switch(mode){
  case SaccEvent::JSON:
    res<<"   pAbonentNumber: "<<pAbonentNumber<<std::endl;
    res<<"   cCommandId: "<<(int)cCommandId<<"("<<commandIdName(cCommandId)<<")"<<std::endl;
    res<<"   cProtocolId: "<<(int)cProtocolId<<std::endl;
    res<<"   sCommandStatus: "<<sCommandStatus<<std::endl;
    res<<"   lDateTime: "<<lDateTime<<std::endl;
    res<<"   iServiceProviderId: "<<iServiceProviderId<<std::endl;
    res<<"   iServiceId: "<<iServiceId<<std::endl;
    res<<"   iOperatorId: "<<iOperatorId<<std::endl;
    res<<"   pSessionKey: "<<pSessionKey<<std::endl;
    break;
  case SaccEvent::CSV:
    res<<pAbonentNumber<<", ";
    res<<(int)cCommandId<<"("<<commandIdName(cCommandId)<<")"<<", ";
    res<<(int)cProtocolId<<", ";
    res<<sCommandStatus<<", ";
    res<<lDateTime<<", ";
    res<<iServiceProviderId<<", ";
    res<<iServiceId<<", ";
    res<<iOperatorId<<", ";
    res<<pSessionKey;
    break;
  }

  return res.str();
}


SaccEvent* SaccEvent::deserialize(char* buf, size_t buflen){
  SaccSerialBuffer buffer(buf,buflen);
  SaccEvent* event;
  uint16_t eventType;
  buffer.ReadNetInt16(eventType);
  switch(eventType){
  case sec_transport: event = new SaccTrafficInfoEvent(); break;
  case sec_bill: event = new SaccBillingInfoEvent(); break;
  default: return NULL;
  }
  event->read(buffer);
  return event;
}


void SaccEventHeader::write(SaccSerialBuffer& buf)
{
    buf.writeStr(pAbonentNumber, MAX_ABONENT_NUMBER_LENGTH);
    buf.writeInt64(lDateTime);
    buf.writeInt32(iOperatorId);
    buf.writeInt32(iServiceProviderId);
    buf.writeInt32(iServiceId);

    buf.writeStr(pSessionKey, MAX_SESSION_KEY_LENGTH);    

    buf.writeByte(cProtocolId);
    buf.writeByte(cCommandId);
    buf.writeInt16(sCommandStatus);
}

void SaccTrafficInfoEvent::write(SaccSerialBuffer& buf)
{
    Header.write(buf);

    buf.writeStr(pMessageText, MAX_TEXT_MESSAGE_LENGTH * 2);
    buf.writeByte(cDirection);
    if (!keywords.empty()) {
      buf.writeStr(keywords, MAX_KEYWORDS_TEXT_LENGTH * 2);
    }
}

void SaccEventHeader::read(SaccSerialBuffer& buf)
{
    buf.readStr(pAbonentNumber);
    buf.readInt64(lDateTime);
    buf.readInt32(iOperatorId);
    buf.readInt32(iServiceProviderId);
    buf.readInt32(iServiceId);

    buf.readStr(pSessionKey);

    buf.readByte(cProtocolId);
    buf.readByte(cCommandId);
    buf.readInt16(sCommandStatus);
}

void SaccTrafficInfoEvent::read(SaccSerialBuffer& buf)
{
    Header.read(buf);

    buf.readStr(pMessageText);
    buf.readByte(cDirection);
    if (buf.getPos()<buf.getSize()) {//has more data
      buf.readStr(keywords);
    }
}

std::string SaccTrafficInfoEvent::toString(SaccEvent::tostring mode){
  std::ostringstream res;
  switch(mode){
  case SaccEvent::JSON:
    res << getName() << ": {" <<std::endl;
    res << Header.toString(mode);
    res << "  cDirection: " << cDirection << std::endl;
    res << "  keywords: " << keywords << std::endl;
    res << "  pMessageText: " << pMessageText << std::endl;
    res <<"}"<<std::endl;
    break;
  case SaccEvent::CSV:
    res << Header.toString(mode);
    res << cDirection << ", ";
    res << keywords << ", ";
    res << pMessageText;
    break;
  default: res <<"Invalid format " << mode;
  }
  return res.str();
}


void SaccBillingInfoEvent::write(SaccSerialBuffer& buf)
{
    Header.write(buf);

    buf.writeInt32(iMediaResourceType);
    buf.writeInt32(iPriceCatId);

    //buf.Write(&fBillingSumm, sizeof(float));
    //buf.Append(static_cast<char*>(&fBillingSumm), sizeof(float));
    //buf.writeFloat(fBillingSumm);
    std::string billingSumm = fBillingSumm.empty() ? "0.0" : fBillingSumm;
    buf.writeStr(billingSumm, MAX_BILLING_SUMM_LENGTH);

    buf.writeStr(pBillingCurrency, MAX_BILLING_CURRENCY_LENGTH);
    // db 20111128: the transition period is over, so write keywords indefinitely
    buf.writeStr(keywords, MAX_KEYWORDS_TEXT_LENGTH * 2);
}

void SaccBillingInfoEvent::read(SaccSerialBuffer& buf)
{
    Header.read(buf);

    buf.readInt32(iMediaResourceType);
    buf.readInt32(iPriceCatId);

    std::string billingSumm;
    buf.readStr(billingSumm);
    if(billingSumm!="0.0") fBillingSumm=billingSumm;

    buf.readStr(pBillingCurrency);
    buf.readStr(keywords);
}

std::string SaccBillingInfoEvent::toString(SaccEvent::tostring mode){
  std::ostringstream res;

  switch(mode){
  case SaccEvent::JSON:
  res << getName() << ": {" <<std::endl;
  res << Header.toString(mode);
  res << "  iMediaResourceType: " << iMediaResourceType << std::endl;
  res << "  iPriceCatId: " << iPriceCatId << std::endl;
  res << "  fBillingSumm: " << (fBillingSumm.empty() ? fBillingSumm : std::string("0.0"))
    << " "<< pBillingCurrency << std::endl;
  res << "  keywords: " << keywords << std::endl;

  res<<"}"<<std::endl;
  break;
  case SaccEvent::CSV:
    res << getName() << ": ";
    res << Header.toString(mode) << ", ";
    res << iMediaResourceType << ", ";
    res << iPriceCatId << ", ";
    res << (fBillingSumm.empty() ? fBillingSumm : std::string("0.0"))
      << " "<< pBillingCurrency << ", ";
    res << keywords;
    break;
  }
  return res.str();
}

void SaccAlarmMessageEvent::write(SaccSerialBuffer& buf)
{
    buf.writeStr(pAbonentsNumbers, MAX_NUMBERS_TEXT_LENGTH);
    buf.writeStr(pAddressEmail, MAX_EMAIL_ADDRESS_LENGTH);
    buf.writeStr(pMessageText, MAX_TEXT_MESSAGE_LENGTH * 2);
    buf.writeStr(pDeliveryTime, DELEVIRY_TIME_LENGTH);

    buf.writeByte(cCriticalityLevel);
 
    if(sUsr)
        buf.writeInt16(sUsr);
    else
        buf.WriteNetInt16(0); 

    if(cEsmClass)
        buf.writeByte(cEsmClass);
    else
        buf.WriteNetInt16(0); 

    if(sSrcPort)
       buf.writeInt16(sSrcPort);    
    else
       buf.WriteNetInt16(0); 
     
    if(sDestPort)
       buf.writeInt16(sDestPort);
    else
       buf.WriteNetInt16(0); 

    buf.writeStr(pPacketType, PACKET_TYPE_LENGTH);
}

void SaccAlarmEvent::write(SaccSerialBuffer& buf)
{
    Header.write(buf);

    buf.writeStr(pMessageText, MAX_TEXT_MESSAGE_LENGTH * 2);

    buf.writeByte(cDirection);
    buf.writeInt32(iAlarmEventId);
}

// ============================================================

Statistics& Statistics::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("Statistics not inited");
    }
    return Single::Instance();
}

Statistics::Statistics()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

Statistics::~Statistics()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace stat
} // namespace scag2
