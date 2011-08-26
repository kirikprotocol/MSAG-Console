#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "Statistics2.h"
#include "util/BufferSerialization.hpp"

using namespace scag2::stat;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< Statistics, OuterCreation > Single;
bool inited = false;
Mutex mtx;

/// NOTE: statistics is required for everything, except Config.
inline unsigned GetLongevity( Statistics* ) { return 250; }

}

namespace scag2 {
namespace stat {

// ============================================================

void SaccSerialBuffer::writeStr(std::string& s, uint16_t maxLen)
{
    uint16_t len = (s.length() > maxLen) ? maxLen : s.length() ;
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
  uint32_t bsize = buff_.GetPos();
  buff_.SetPos(0);
  buff_ << bsize;
  buff_.SetPos(0);
  return sock.WriteAll(buff_.GetCurPtr() ,bsize) > 0;;
}

void * SaccSerialBuffer::getBuffer() {
  return buff_.GetCurPtr();
}
uint32_t SaccSerialBuffer::getPos() {
  return buff_.GetPos();
}
void SaccSerialBuffer::setPos(uint32_t newPos) {
  buff_.SetPos(newPos);
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
    if (!keywords.empty()) {
      buf.writeStr(keywords, MAX_KEYWORDS_TEXT_LENGTH * 2);
    }
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
