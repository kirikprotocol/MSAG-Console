#include <poll.h>
#include "MTPersClient.h"
#include "scag/pers/util/Types.h"

namespace scag { namespace mtpers {

MTPersClient::MTPersClient(const string& _host, int _port, int _timeout, int _pingTimeout):connected(false),
                           host(_host), port(_port), timeout(_timeout), pingTimeout(_pingTimeout), asynch(false), sequenceNumber(1)
{
  log = Logger::getInstance("client");
}

MTPersClient::~MTPersClient() { 
  if (connected) {
    sock.Close(); 
  }
}

void MTPersClient::init()
{
  char resp[3];

  if (connected) {
    return;
  }
  asynch = false;

  smsc_log_info(log, "Connecting to persserver host=%s:%d timeout=%d", host.c_str(), port, timeout);

  if (sock.Init(host.c_str(), port, timeout) == -1 || sock.Connect() == -1) {
    throw PersClientException(CANT_CONNECT);
  }

  if (sock.Read(resp, 2) != 2) {
    sock.Close();
    throw PersClientException(CANT_CONNECT);
  }
  resp[2] = 0;
  if (!strcmp(resp, "SB")) {
    sock.Close();
    throw PersClientException(SERVER_BUSY);
  } else if(strcmp(resp, "OK")) {
    sock.Close();
    throw PersClientException(UNKNOWN_RESPONSE);
  }
  connected = true;
  //if (!asynch) {
   // bind();
  //}
}

void MTPersClient::ReadAllTO(char* buf, uint32_t sz)
{
  int cnt;
  uint32_t rd = 0;
  struct pollfd pfd;

  while(sz) {
    pfd.fd = sock.getSocket();
    pfd.events = POLLIN;
    if (poll(&pfd, 1, timeout) <= 0 || !(pfd.revents & POLLIN)) {
      //if (poll(&pfd, 1, timeout * 1000) <= 0 || !(pfd.revents & POLLIN)) {
      throw PersClientException(TIMEOUT);
    }

    cnt = sock.Read(buf + rd, sz);
    if (cnt <= 0) {
      throw PersClientException(READ_FAILED);
    }

    rd += cnt;
    sz -= cnt;
  }
}

void MTPersClient::WriteAllTO(const char* buf, uint32_t sz)
{
  int cnt;
  uint32_t wr = 0;
  struct pollfd pfd;    

  while(sz) {
    pfd.fd = sock.getSocket();
    pfd.events = POLLOUT | POLLIN;
    if (poll(&pfd, 1, timeout) <= 0) {
      //if (poll(&pfd, 1, timeout * 1000) <= 0) {
      throw PersClientException(TIMEOUT);
    } else if (!(pfd.revents & POLLOUT) || (pfd.revents & POLLIN)) {
      throw PersClientException(SEND_FAILED);		
    }
    cnt = sock.Write(buf + wr, sz);
    if (cnt <= 0) {
      throw PersClientException(SEND_FAILED);
    }
    wr += cnt;
    sz -= cnt;
  }
}

void MTPersClient::SendPacket(SerialBuffer& bsb)
{
  uint32_t  t = 0;

  for (;;) {
    try {
      init();
      setPacketSize(bsb);
      WriteAllTO(bsb.c_ptr(), bsb.length());
      smsc_log_debug(log, "write to socket: len=%d, data=%s", bsb.length(), bsb.toString().c_str());			
      return;
    } catch(PersClientException &e) {
      smsc_log_debug(log, "PersClientException: %s", e.what());		
      connected = false;
      sock.Close();
      if(++t >= 2) throw;
    }
  }
}

void MTPersClient::ReadPacket(SerialBuffer& bsb)
{
    char tmp_buf[1024];
    uint32_t sz;

    if (!connected) {
      throw PersClientException(NOT_CONNECTED);
    }

    bsb.Empty();
    ReadAllTO(tmp_buf, static_cast<uint32_t>(sizeof(uint32_t)));
    bsb.Append(tmp_buf, static_cast<uint32_t>(sizeof(uint32_t)));
    bsb.SetPos(0);
    sz = bsb.ReadInt32() - static_cast<uint32_t>(sizeof(uint32_t));
    smsc_log_debug(log, "%d bytes will be read from socket", sz);

    while (sz > 1024) {
      ReadAllTO(tmp_buf, 1024);
      bsb.Append(tmp_buf, 1024);
      sz -= 1024;
    } 
    if(sz) {
      ReadAllTO(tmp_buf, sz);
      bsb.Append(tmp_buf, sz);
    }
    smsc_log_debug(log, "read from socket: len=%d, data=%s", bsb.length(), bsb.toString().c_str());
	bsb.SetPos(4);
}

void MTPersClient::setPacketSize(SerialBuffer& bsb)
{
  bsb.SetPos(0);
  bsb.WriteInt32(bsb.GetSize());
}

uint32_t MTPersClient::getPacketSize(SerialBuffer& bsb)
{
  bsb.SetPos(0);
  return bsb.ReadInt32();
}

PersServerResponseType MTPersClient::GetServerResponse(SerialBuffer& bsb)
{
  try {
    if (asynch) {
      uint32_t number = bsb.ReadInt32();
      smsc_log_debug(log, "read %d packet", number);
    }
    return (PersServerResponseType)bsb.ReadInt8();
  } catch(SerialBufferOutOfBounds &e) {
    throw PersClientException(BAD_RESPONSE);
  }
}

void MTPersClient::FillHead(PersCmd pc, ProfileType pt, const PersKey& key, SerialBuffer& bsb, bool batch)
{
  if (asynch) {
    bsb.WriteInt32(sequenceNumber);
    smsc_log_debug(log, "prepare %d packet", sequenceNumber);
    ++sequenceNumber;
  }
  bsb.WriteInt8((uint8_t)pc);
  if (batch) {
    return;
  }
  bsb.WriteInt8((uint8_t)pt);
  if(pt == PT_ABONENT) {
    bsb.WriteString(key.skey);
  } else {
    bsb.WriteInt32(key.ikey);	
  }
}

void MTPersClient::setBatchCount(uint32_t cnt, SerialBuffer& bsb)
{
	uint32_t i = bsb.getPos();
	bsb.SetPos(5);
	bsb.WriteInt16(cnt);
	bsb.SetPos(i);
}

void MTPersClient::CheckServerResponse(SerialBuffer& bsb)
{
  switch (GetServerResponse(bsb)) {
  case RESPONSE_OK: smsc_log_debug(log, "Response OK"); break;
  case RESPONSE_PROPERTY_NOT_FOUND: throw PersClientException(PROPERTY_NOT_FOUND);
  case RESPONSE_ERROR: throw PersClientException(SERVER_ERROR);
  case RESPONSE_BAD_REQUEST: throw PersClientException(BAD_REQUEST);
  case RESPONSE_TYPE_INCONSISTENCE: throw PersClientException(TYPE_INCONSISTENCE);
  case RESPONSE_PROFILE_LOCKED: throw PersClientException(PROFILE_LOCKED);
  case RESPONSE_NOTSUPPORT: throw PersClientException(COMMAND_NOTSUPPORT);
  default: throw PersClientException(UNKNOWN_RESPONSE);
  }
}
void MTPersClient::reinit(const char *_host, int _port, int _timeout, int _pingTimeout) //throw(PersClientException)
{
  smsc_log_info(log, "PersClient reinit host=%s:%d timeout=%d, pingtimeout=%d", _host, _port, _timeout, _pingTimeout);

  if (connected) {
    sock.Close();
  }
  connected = false;
  host = _host;
  port = _port;
  timeout = _timeout;
  pingTimeout = _pingTimeout;
  actTS = time(NULL);
  try {
    init();
  } catch(const PersClientException& e) {
    smsc_log_error(log, "Error during reinitialization. %s", e.what());
  }
}

void MTPersClient::ping()
{
  try {
    sb.Empty();
    sb.SetPos(4);
    sb.WriteInt8(PC_PING);
    SendPacket(sb);
    ReadPacket(sb);
    if (GetServerResponse(sb) != RESPONSE_OK) {
      throw PersClientException(SERVER_ERROR);
    }
    smsc_log_debug(log, "Ping sent");
  } catch(const PersClientException& e) {
    smsc_log_error(log, "Ping failed: %s", e.what());
    if(connected) {
      sock.Close();
      connected = false;
    }
    try {
      init();
    } catch(...) {
    }
  } catch(...) {
    smsc_log_error(log, "Unknown exception");
  }
}

void MTPersClient::SetPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb, bool batch) {
  FillHead(PC_SET, pt, key, bsb, batch);
  prop.Serialize(bsb);	
}

void MTPersClient::SetPropertyResult(SerialBuffer& bsb) {
  CheckServerResponse(bsb);
}

void MTPersClient::SetProperty(ProfileType pt, const PersKey& key, Property& prop, bool batch) {
  try {
    emptyPacket(sb);
    SetPropertyPrepare(pt, key, prop, sb, batch);
    SendPacket(sb);
    if (asynch) {
      emptyPacket(sb);
      SetPropertyPrepare(pt, key, prop, sb, batch);
      SendPacket(sb);

      emptyPacket(sb);
      SetPropertyPrepare(pt, key, prop, sb, batch);
      SendPacket(sb);

      emptyPacket(sb);
      SetPropertyPrepare(pt, key, prop, sb, batch);
      SendPacket(sb);
    }
    ReadPacket(sb);
    SetPropertyResult(sb);
    if (asynch) {
      ReadPacket(sb);
      SetPropertyResult(sb);

      ReadPacket(sb);
      SetPropertyResult(sb);

      ReadPacket(sb);
      SetPropertyResult(sb);
    }
  } catch(const PersClientException& e) {
    smsc_log_warn(log, "Set Failed. %s", e.what());
  } 
}

void MTPersClient::GetPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb, bool batch)
{
    FillHead(PC_GET, pt, key, bsb, batch);
    bsb.WriteString(property_name);
}

void MTPersClient::GetPropertyResult(Property& prop, SerialBuffer& bsb)
{
  CheckServerResponse(bsb);
  prop.Deserialize(bsb);
}

void MTPersClient::GetProperty(ProfileType pt, const PersKey& key, const char *property_name, Property& prop, bool batch)
{
  try {
    emptyPacket(sb);
    GetPropertyPrepare(pt, key, property_name, sb, batch);
    SendPacket(sb);
    ReadPacket(sb);
    GetPropertyResult(prop, sb);
    actTS = time(NULL);
  } catch(const PersClientException& e) {
    smsc_log_warn(log, "Get Failed. %s", e.what());
  } 
}

void MTPersClient::DelPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb, bool batch)
{
    FillHead(PC_DEL, pt, key, bsb, batch);
    bsb.WriteString(property_name);
}

bool MTPersClient::DelPropertyResult(SerialBuffer& bsb)
{
  CheckServerResponse(bsb);
  return  true;
}

bool MTPersClient::DelProperty(ProfileType pt, const PersKey& key, const char *property_name, bool batch)
{
  try {
    emptyPacket(sb);
    DelPropertyPrepare(pt, key, property_name, sb, batch);
    SendPacket(sb);
    ReadPacket(sb);
    bool b = DelPropertyResult(sb);
    actTS = time(NULL);
    return b;
  } catch(const PersClientException& e) {
    smsc_log_warn(log, "Del Failed. %s", e.what());
    return false;
  }
}

bool MTPersClient::bind() {
  try {
    SerialBuffer bindSb;
    bindSb.SetPos(4);
    bindSb.WriteInt8(PC_BIND_ASYNCH);
    SendPacket(bindSb);
    bindSb.Empty();
    ReadPacket(bindSb);
    CheckServerResponse(bindSb);
    actTS = time(NULL);
    asynch = true;
    return true;
  } catch(const PersClientException& e) {
    smsc_log_warn(log, "Bind Failed. %s", e.what());
    return false;
  }
}

void MTPersClient::IncPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb, bool batch)
{
    if(prop.getType() != INT && prop.getType() != DATE)
        throw PersClientException(INVALID_PROPERTY_TYPE);

    FillHead(PC_INC_RESULT, pt, key, bsb, batch);
    prop.Serialize(bsb);
}

int MTPersClient::IncPropertyResult(SerialBuffer& bsb)
{
  return IncModPropertyResult(bsb);
}

int MTPersClient::IncProperty(ProfileType pt, const PersKey& key, Property& prop, bool batch)
{
  try {
    emptyPacket(sb);
    IncPropertyPrepare(pt, key, prop, sb, batch);
    SendPacket(sb);
    ReadPacket(sb);
    int i = IncPropertyResult(sb);

    actTS = time(NULL);
    return i;
  } catch(const PersClientException& e) {
    smsc_log_warn(log, "Inc Failed. %s", e.what());
    return -1;
  }
}

void MTPersClient::IncModPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod, SerialBuffer& bsb, bool batch)
{
    if(prop.getType() != INT && prop.getType() != DATE)
        throw PersClientException(INVALID_PROPERTY_TYPE);

    FillHead(PC_INC_MOD, pt, key, bsb, batch);
    bsb.WriteInt32(mod);
    prop.Serialize(bsb);
}

int MTPersClient::IncModPropertyResult(SerialBuffer& bsb)
{
  try {
    CheckServerResponse(bsb);
    return bsb.ReadInt32();
  } catch (const SerialBufferOutOfBounds &e)  {
    throw PersClientException(BAD_RESPONSE);
  }
}

int MTPersClient::IncModProperty(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod, bool batch) {
  try {
    emptyPacket(sb);
    IncModPropertyPrepare(pt, key, prop, mod, sb, batch);
    SendPacket(sb);
    ReadPacket(sb);
    int i =  IncModPropertyResult(sb);
    actTS = time(NULL);
    return i;
  } catch(const PersClientException& e) {
    smsc_log_warn(log, "IncMod Failed. %s", e.what());
    return -1;
  }
}

void MTPersClient::PrepareBatch(SerialBuffer& bsb, ProfileType pt, const PersKey& key, uint16_t cnt, bool transactMode) {
  emptyPacket(bsb);
  FillHead(PC_MTBATCH, pt, key, bsb);
  bsb.WriteInt16(cnt);
  bsb.WriteInt8((uint8_t)transactMode);
}

void MTPersClient::RunBatch(SerialBuffer& bsb) {
  SendPacket(bsb);
  ReadPacket(bsb);
}

void MTPersClient::FinishPrepareBatch(uint32_t cnt, SerialBuffer& bsb) {
  //setBatchCount(cnt, bsb);
}

void MTPersClient::disconnect() {
  if (connected) {
    sock.Close();
    connected = false;
    smsc_log_debug(log, "disconnect");
  }
}

void MTPersClient::asynchTest(const string& address, int key) {
  Property prop;
  char tvs[30];
  sprintf(tvs, "test_prop%s", address.c_str());						
  prop.setString(tvs, "test value", INFINIT, -1, 20);
  try {
    //if (!asynch) {
      //return;
    //}
    SetProperty(PT_ABONENT, address.c_str(), prop);
  } catch (const PersClientException& exc) {
    smsc_log_warn(log, "address '%s' PersClientException: %s", address.c_str(), exc.what());
  }
}

void MTPersClient::testCase(const string& address, int key) {
  Property prop;
  char tvs[30];
  sprintf(tvs, "test_prop%s", address.c_str());						
  prop.setString(tvs, "test value", INFINIT, -1, 20);
  try {
    SetProperty(PT_ABONENT, address.c_str(), prop);
    SetProperty(PT_PROVIDER, key, prop);
    SetProperty(PT_OPERATOR, key, prop);
    SetProperty(PT_SERVICE, key, prop);

    GetProperty(PT_ABONENT, address.c_str(), tvs, prop);
    DelProperty(PT_ABONENT, address.c_str(), tvs);
  } catch (const PersClientException& exc) {
    smsc_log_warn(log, "address '%s' PersClientException: %s", address.c_str(), exc.what());
  }

}

void MTPersClient::testCase_DelProfile(ProfileType pt, const char* address, int intKey) {
  char tv[50];
  PersKey key(address);
  char profKey[20];
  sprintf(profKey, "%s", address);						
  string storeName = "ABONENT";

  sprintf(tv, "test_valDelProfile%s", address);		
  Property prop;
  GetProperty(pt, key, tv, prop);
  smsc_log_debug(log,  ">>%s %s: get int befor set %s", storeName.c_str(), profKey, prop.toString().c_str());
  prop.setInt(tv, 999, FIXED, -1, 20);
  SetProperty(pt, key, prop);
  GetProperty(pt, key, tv, prop);
  smsc_log_debug(log,  ">>%s %s: get int after set %s", storeName.c_str(), profKey, prop.toString().c_str());
  DelProperty(pt, key, tv);
  GetProperty(pt, key, tv, prop);
  smsc_log_debug(log,  ">>%s %s: get int after del %s", storeName.c_str(), profKey, prop.toString().c_str());
}

void MTPersClient::testCase_CommandProcessing(ProfileType pt, const char* address, int intKey) {
  char tv[50], tvs[50], tvb[50], tvd[50];		
  PersKey key;
  char profKey[20];
  if (pt == PT_ABONENT) {
    key.skey = address;
    sprintf(tv, "test_val%s", address);			
    sprintf(tvs, "test_val_string%s", address);						
    sprintf(tvb, "test_val_bool%s", address);						
    sprintf(tvd, "test_val_date%s", address);						
    sprintf(profKey, "%s", address);						
  } else {
    key.ikey = intKey;
    sprintf(tv, "test_val%05d", intKey);			
    sprintf(tvs, "test_val_string%05d", intKey);						
    sprintf(tvb, "test_val_bool%05d", intKey);						
    sprintf(tvd, "test_val_date%05d", intKey);						
    sprintf(profKey, "%d", intKey);						
  }
  string storeName;
  switch (pt) {
  case scag::pers::util::PT_ABONENT: storeName = "ABONENT"; break;
  case scag::pers::util::PT_SERVICE: storeName = "SERVICE"; break;
  case scag::pers::util::PT_PROVIDER: storeName = "PROVIDER"; break;
  case scag::pers::util::PT_OPERATOR: storeName = "OPERATOR"; break;
  }
  Property prop;

  prop.setInt(tv, 234567, FIXED, -1, 20);
  SetProperty(pt, key, prop);

  prop.setBool(tvb, false, R_ACCESS, -1, 25);
  SetProperty(pt, key, prop);

  prop.setString(tvs, "test_string", W_ACCESS, -1, 25);
  SetProperty(pt, key, prop);

  prop.setDate(tvd, 111111, INFINIT, -1, 25);
  SetProperty(pt, key, prop);

  GetProperty(pt, key, tv, prop);
  smsc_log_debug(log,  ">>%s %s: get int %s", storeName.c_str(), profKey, prop.toString().c_str());

  GetProperty(pt, key, tvb, prop);
  smsc_log_debug(log,  ">>%s %s: get bool %s", storeName.c_str(), profKey, prop.toString().c_str());

  GetProperty(pt, key, tvs, prop);
  smsc_log_debug(log,  ">>%s %s: get string %s", storeName.c_str(), profKey, prop.toString().c_str());

  GetProperty(pt, key, tvd, prop);
  smsc_log_debug(log,  ">>%s %s: get string1 %s", storeName.c_str(), profKey, prop.toString().c_str());

  prop.setInt(tv, 10, FIXED, -1, 20);
  IncProperty(pt, key, prop);
  GetProperty(pt, key, tv, prop);
  smsc_log_debug(log,  ">>%s %s: get int(after inc) %s", storeName.c_str(), profKey, prop.toString().c_str());
  DelProperty(pt, key, tv);
  smsc_log_debug(log,  ">>%s %s: del int", storeName.c_str(), profKey);

  prop.setInt(tv, 123, FIXED, -1, 20);
  IncModProperty(pt, key, prop, 10);
  GetProperty(pt, key, tv, prop);
  smsc_log_debug(log,  ">>%s %s: get int(after inc mod) %s", storeName.c_str(), profKey, prop.toString().c_str());

  uint16_t cnt = 5;
  PrepareBatch(sb, pt, key, cnt, false);
  prop.setInt(tv, 234567, FIXED, -1, 20);
  SetPropertyPrepare(pt, key, prop, sb, true);
  GetPropertyPrepare(pt, key, prop.getName(), sb, true);
  prop.setInt(tv, 10, FIXED, -1, 20);
  IncPropertyPrepare(pt, key, prop, sb, true);
  GetPropertyPrepare(pt, key, prop.getName(), sb, true);
  //DelPropertyPrepare(pt, key, prop.getName(), sb, true);
  GetPropertyPrepare(pt, key, prop.getName(), sb, true);
  RunBatch(sb);
  try {
    SetPropertyResult(sb);
    GetPropertyResult(prop, sb);
    smsc_log_debug(log,  "BATCH>>%s %s: get int(after set) %s", storeName.c_str(), profKey, prop.toString().c_str());
    IncPropertyResult(sb);
    GetPropertyResult(prop, sb);
    smsc_log_debug(log,  "BATCH>>%s %s: get int(after inc) %s", storeName.c_str(), profKey, prop.toString().c_str());
    //DelPropertyResult(sb);
    GetPropertyResult(prop, sb);
    smsc_log_debug(log,  "BATCH>>%s %s: get int(after del) %s", storeName.c_str(), profKey, prop.toString().c_str());
  } catch (const PersClientException& exc) {
    smsc_log_warn(log, "Batch Failed: PersClientException: %s", exc.what());
  }
}


int ClientTask::Execute() {
  smsc_log_debug(logger, "client task %p started", this);
  char address[20];
  time_t t = time(NULL);
  //int iterCnt = 100;
  int iterCnt = 400;
  int reqCount = 14 * iterCnt * 2;
  client.init();
  for (int i = 1; i < iterCnt + 1; ++i) {
    if (isStopping) {
      break;
    }
    try {
      sprintf(address, "8913%04d%03d", i, taskIndex);			
      //client.asynchTest(address, i);
      //client.testCase_DelProfile(PT_ABONENT, address, i);
      client.testCase_CommandProcessing(PT_ABONENT, address, i);
      client.testCase_CommandProcessing(PT_PROVIDER, address, 20);
      //client.testCase_CommandProcessing(PT_SERVICE, address, i);
      //client.testCase_CommandProcessing(PT_OPERATOR, address, i);
      //client.disconnect();
    } catch (const PersClientException& exc) {
      smsc_log_error(logger, "%p: address '%s' PersClientException: %s", this, exc.what());
    }
  }
  t = time(NULL) - t;
  if (t) {
    int reqPerSec = static_cast<int>(reqCount / t);
    smsc_log_error(logger, "%p timings %d", this,  reqPerSec);
  }
  isStopping = true;
  smsc_log_error(logger, "client task %p stoped", this);
  return 0;
}




}//mtpers
}//scag

