static char const ident[] = "$Id$";

#include "inman/common/util.hpp"
#include "inman/common/cvtutil.hpp"
#include <assert.h>
#include "ussdsm.hpp"


namespace smsc {
namespace inman {
namespace uss {

using smsc::inman::common::fillAddress;
using smsc::inman::common::dump;
using smsc::cvtutil::packTextAs7BitSafe;

using smsc::inman::usscomp::ProcessUSSRequestRes;
using smsc::inman::usscomp::ProcessUSSRequestArg;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::Invoke;

using smsc::inman::usscomp::MAPUSS_OpCode;

USHORT_T USSDSM::handleEndDialog()
{
  smsc_log_debug(logger," USSDSM{id=%d} receives END ",this->getId());
  return MSG_OK;
}

static APP_CONTEXT_T map_networkUnstructuredSs_v2 = {7,{0x04,0x00,0x00,0x01,0x00,0x13,0x02,}};

USSDSM::USSDSM(Session* ses):Dialog(ses, 0, id_ac_map_networkUnstructuredSs_v2)
{
  assert( session );
  logger = Logger::getInstance("smsc.inman.ussdsm");
  addListener( this );
}

USSDSM::~USSDSM()
{
  //this->removeListener( this );
  session->closeDialog(this);
}

void USSDSM::onDialogInvoke( Invoke* op )
{
  assert( op );
  smsc_log_debug(logger," USSDSM{id=%d} onDialogInvoke called",this->getId());
}


template <int N>
void push(std::vector<unsigned char>& v,const char (&arr)[N])
{
  v.push_back(arr,arr+N);
}

void push(std::vector<unsigned char>& v,unsigned char c)
{
  v.push_back(c);
  //v.insert(v.begin(),arr,arr+len);
}

void USSDSM::makeRequest(const char * msisdn,const char* vlraddr)
{
  unsigned char ussdString[32]={0,};
  char req[] = "*100#";
  ProcessUSSRequestArg arg;

  //make user info
  UCHAR_T map_open[] = {06,0x07,0x04,0x00,0x00,0x01,0x01,0x01,0x01,0xA0};
  ADDRESS_BUF_T msisdnbuf;
  fillAddress(&msisdnbuf,msisdn);
  ADDRESS_BUF_T vlrbuf;
  fillAddress(&vlrbuf,vlraddr);
  int clen;
  vector<unsigned char> ui4;
  ui4.insert(ui4.begin(),msisdnbuf.addr,msisdnbuf.addr+msisdnbuf.addrLen);
  ui4.insert(ui4.begin(),msisdnbuf.addrLen);
  ui4.insert(ui4.begin(),0x82);

  ui4.insert(ui4.begin(),vlrbuf.addr,vlrbuf.addr+vlrbuf.addrLen);
  ui4.insert(ui4.begin(),vlrbuf.addrLen);
  ui4.insert(ui4.begin(),0x81);

  clen = ui4.size();
  ui4.insert(ui4.begin(),clen);
  ui4.insert(ui4.begin(),0xA0);
  clen = ui4.size();
  ui4.insert(ui4.begin(),clen);
  ui4.insert(ui4.begin(),map_open,map_open+sizeof(map_open));
  clen = ui4.size();
  ui4.insert(ui4.begin(),clen);
  ui4.insert(ui4.begin(),0x28);

  //fillAddress
  arg._dCS = 0x0F;
  unsigned bytes = 0;
  unsigned elen = 0;
  bytes = packTextAs7BitSafe((const char*)req,strlen(req),(unsigned char *)ussdString,sizeof(ussdString),&elen);
  if( bytes*8-elen*7 == 7 ) ussdString[bytes-1] |= (0x0D<<1);

  arg.setUSSData(ussdString,bytes);
  ProcessUnstructuredSSRequestReq(&arg);
  beginDialog(remoteAddr,&ui4[0],ui4.size());
}

void USSDSM::ProcessUnstructuredSSRequestReq(ProcessUSSRequestArg* arg)
{
  Invoke* op = invoke( MAPUSS_OpCode::processUSS_Request );
  assert( op );
  assert( arg );
  op->setParam( arg );
  op->send( this );
}



}//namespace uss
}//namespace inman
}//namespace smsc
