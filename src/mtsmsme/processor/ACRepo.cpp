static char const ident[] = "$Id$";
#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/processor/MTFTSM.hpp"
#include "mtsmsme/processor/ULTSM.hpp"
#include "mtsmsme/processor/SRI4SMTSM.hpp"
#include "mtsmsme/processor/CLTSM.hpp"
#include "mtsmsme/processor/PRNTSM.hpp"
#include "mtsmsme/processor/ISD.hpp"
#include "mtsmsme/processor/MOFTSM.hpp"
namespace smsc{namespace mtsmsme{namespace processor{

bool TrId::operator==(const TrId& obj)const
{
  return size == obj.size && memcmp(&buf[0],&obj.buf[0],sizeof(uint8_t)*size) == 0;
}
string TrId::toString()
{
  char tmpstr[9];
  char* text = tmpstr;
  int k = 0;
  for ( int i=0; i<size; i++){
    k+=sprintf(text+k,"%02X",buf[i]);
  }
  text[k]=0;
  return string(text);
}
  AC::AC(){}
  void AC::init(unsigned long *_arcs, int _size)
  {
    arcs.clear();
    arcs.reserve(_size);
    arcs.insert(arcs.begin(),_arcs,_arcs+_size);
  }
  AC::AC(unsigned long *_arcs, int _size)
  {
    arcs.reserve(_size);
    arcs.insert(arcs.begin(),_arcs,_arcs+_size);
  }
  bool AC::operator==(const AC& obj)const
  {
    return arcs.size()==obj.arcs.size() &&
           memcmp(&arcs[0],&obj.arcs[0],sizeof(unsigned long)*arcs.size())==0;
  }
  bool AC::operator!=(const AC& obj)const
  {
    return arcs.size()!=obj.arcs.size() ||
           memcmp(&arcs[0],&obj.arcs[0],sizeof(unsigned long)*arcs.size())!=0;
  }
  string AC::toString()
  {
    int k = 0;
    int len = (int)arcs.size();
    char text[32] = {0,};
    std::vector<unsigned char> stream;

    for(int i = 0; i < len; )
    {
      k=sprintf(text,"%ld",arcs[i]);
      stream.insert(stream.end(),text,text+k);
      if(++i < len) stream.push_back('.');
    }
    string result((char*)&stream[0],(char*)&stream[0]+stream.size());
    return result;
  }
static unsigned long           null_buf[] = {0,0,0,0,0,0, 0,0};
AC null_ac        = AC(null_buf,(int)(sizeof(null_buf)/sizeof(unsigned long)));

static unsigned long sm_mt_relay_v1_buf[] = {0,4,0,0,1,0,25,1};
static unsigned long sm_mt_relay_v2_buf[] = {0,4,0,0,1,0,25,2};
static unsigned long sm_mt_relay_v3_buf[] = {0,4,0,0,1,0,25,3};
AC sm_mt_relay_v1 = AC(sm_mt_relay_v1_buf,(int)(sizeof(sm_mt_relay_v1_buf)/sizeof(unsigned long)));
AC sm_mt_relay_v2 = AC(sm_mt_relay_v2_buf,(int)(sizeof(sm_mt_relay_v2_buf)/sizeof(unsigned long)));
AC sm_mt_relay_v3 = AC(sm_mt_relay_v3_buf,(int)(sizeof(sm_mt_relay_v3_buf)/sizeof(unsigned long)));

static unsigned long net_loc_upd_v1_buf[] = {0,4,0,0,1,0, 1,1};
static unsigned long net_loc_upd_v2_buf[] = {0,4,0,0,1,0, 1,2};
static unsigned long net_loc_upd_v3_buf[] = {0,4,0,0,1,0, 1,3};
AC net_loc_upd_v1 = AC(net_loc_upd_v1_buf,(int)(sizeof(net_loc_upd_v1_buf)/sizeof(unsigned long)));
AC net_loc_upd_v2 = AC(net_loc_upd_v2_buf,(int)(sizeof(net_loc_upd_v2_buf)/sizeof(unsigned long)));
AC net_loc_upd_v3 = AC(net_loc_upd_v3_buf,(int)(sizeof(net_loc_upd_v3_buf)/sizeof(unsigned long)));

static unsigned long net_loc_cancel_v1_buf[] = {0,4,0,0,1,0,2,1};
static unsigned long net_loc_cancel_v2_buf[] = {0,4,0,0,1,0,2,2};
static unsigned long net_loc_cancel_v3_buf[] = {0,4,0,0,1,0,2,3};
AC net_loc_cancel_v1 = AC(net_loc_cancel_v1_buf,(int)(sizeof(net_loc_cancel_v1_buf)/sizeof(unsigned long)));
AC net_loc_cancel_v2 = AC(net_loc_cancel_v2_buf,(int)(sizeof(net_loc_cancel_v2_buf)/sizeof(unsigned long)));
AC net_loc_cancel_v3 = AC(net_loc_cancel_v3_buf,(int)(sizeof(net_loc_cancel_v3_buf)/sizeof(unsigned long)));

static unsigned long shortMsgGatewayContext_v1_buf[] = {0,4,0,0,1,0,20,1};
static unsigned long shortMsgGatewayContext_v2_buf[] = {0,4,0,0,1,0,20,2};
static unsigned long shortMsgGatewayContext_v3_buf[] = {0,4,0,0,1,0,20,3};
AC shortMsgGatewayContext_v1 = AC(shortMsgGatewayContext_v1_buf,(int)(sizeof(shortMsgGatewayContext_v1_buf)/sizeof(unsigned long)));
AC shortMsgGatewayContext_v2 = AC(shortMsgGatewayContext_v2_buf,(int)(sizeof(shortMsgGatewayContext_v2_buf)/sizeof(unsigned long)));
AC shortMsgGatewayContext_v3 = AC(shortMsgGatewayContext_v3_buf,(int)(sizeof(shortMsgGatewayContext_v3_buf)/sizeof(unsigned long)));

static unsigned long roamingNumberEnquiryContext_v1_buf[] = {0,4,0,0,1,0,3,1};
static unsigned long roamingNumberEnquiryContext_v2_buf[] = {0,4,0,0,1,0,3,2};
static unsigned long roamingNumberEnquiryContext_v3_buf[] = {0,4,0,0,1,0,3,3};
AC roamingNumberEnquiryContext_v1 = AC(roamingNumberEnquiryContext_v1_buf,(int)(sizeof(roamingNumberEnquiryContext_v1_buf)/sizeof(unsigned long)));
AC roamingNumberEnquiryContext_v2 = AC(roamingNumberEnquiryContext_v2_buf,(int)(sizeof(roamingNumberEnquiryContext_v2_buf)/sizeof(unsigned long)));
AC roamingNumberEnquiryContext_v3 = AC(roamingNumberEnquiryContext_v3_buf,(int)(sizeof(roamingNumberEnquiryContext_v3_buf)/sizeof(unsigned long)));

static unsigned long subscriberDataMngtContext_v1_buf[] = {0,4,0,0,1,0,16,1};
static unsigned long subscriberDataMngtContext_v2_buf[] = {0,4,0,0,1,0,16,2};
static unsigned long subscriberDataMngtContext_v3_buf[] = {0,4,0,0,1,0,16,3};
AC subscriberDataMngtContext_v1 = AC(subscriberDataMngtContext_v1_buf,(int)(sizeof(subscriberDataMngtContext_v1_buf)/sizeof(unsigned long)));
AC subscriberDataMngtContext_v2 = AC(subscriberDataMngtContext_v2_buf,(int)(sizeof(subscriberDataMngtContext_v2_buf)/sizeof(unsigned long)));
AC subscriberDataMngtContext_v3 = AC(subscriberDataMngtContext_v3_buf,(int)(sizeof(subscriberDataMngtContext_v3_buf)/sizeof(unsigned long)));

static unsigned long shortMsgMoRelayContext_v2_buf[] = {0,4,0,0,1,0,21,2};
AC shortMsgMoRelayContext_v2 = AC(shortMsgMoRelayContext_v2_buf,(int)(sizeof(shortMsgMoRelayContext_v2_buf)/sizeof(unsigned long)));

static unsigned long locationInfoRetrievalContext_v3_buf[] = {0,4,0,0,1,0,5,3};
AC locationInfoRetrievalContext_v3 = AC(locationInfoRetrievalContext_v3_buf,(int)(sizeof(locationInfoRetrievalContext_v3_buf)/sizeof(unsigned long)));

bool isMapV1ContextSupported(AC& appcntx)
{
  return (
		  appcntx == sm_mt_relay_v1 ||
		  appcntx == roamingNumberEnquiryContext_v1 ||
		  appcntx == shortMsgGatewayContext_v1
         );
}
bool isIncomingContextSupported(AC& appcntx)
{
  return (
          appcntx == sm_mt_relay_v2 ||
          appcntx == sm_mt_relay_v3  ||
          appcntx == net_loc_cancel_v2 ||
          appcntx == net_loc_cancel_v3 ||
          appcntx == roamingNumberEnquiryContext_v2 ||
          appcntx == roamingNumberEnquiryContext_v3 ||
          appcntx == subscriberDataMngtContext_v2 ||
          appcntx == subscriberDataMngtContext_v3 ||
          appcntx == shortMsgGatewayContext_v2 ||
          appcntx == shortMsgGatewayContext_v3 ||
          appcntx == shortMsgMoRelayContext_v2
         );
}
TSM* createIncomingTSM(TrId ltrid,AC& appcntx,TCO* tco)
{
  TSM* tsm = 0;
  if (appcntx == sm_mt_relay_v1 || appcntx == sm_mt_relay_v2 || appcntx == sm_mt_relay_v3 )
  {
    tsm = new MTFTSM(ltrid,appcntx,tco);
  }
  if (appcntx == net_loc_cancel_v1 || appcntx == net_loc_cancel_v2 || appcntx == net_loc_cancel_v3 )
  {
    tsm = new CLTSM(ltrid,appcntx,tco);
  }
  if (appcntx == shortMsgGatewayContext_v1 || appcntx == shortMsgGatewayContext_v2 || appcntx == shortMsgGatewayContext_v3)
  {
    tsm = new SRI4SMTSM(ltrid,appcntx,tco);
  }
  if (appcntx == roamingNumberEnquiryContext_v1 || appcntx == roamingNumberEnquiryContext_v2 || appcntx == roamingNumberEnquiryContext_v3)
  {
    tsm = new PRNTSM(ltrid,appcntx,tco);
  }
  if (appcntx == subscriberDataMngtContext_v2 || appcntx == subscriberDataMngtContext_v3)
  {
	  tsm = new ISD(ltrid,appcntx,tco);
  }
  if (appcntx == shortMsgMoRelayContext_v2)
  {
    tsm = new MOFTSM(ltrid,appcntx,tco);
  }
  return tsm;
}
TSM* createOutgoingTSM(TrId ltrid,AC& appcntx,TCO* tco)
{
  TSM* tsm = 0;
  if ( appcntx == shortMsgGatewayContext_v3 )
    tsm = new SRI4SMTSM(ltrid,appcntx,tco);
  if ( appcntx == shortMsgGatewayContext_v2 )
    tsm = new SRI4SMTSM(ltrid,appcntx,tco);
  if ( appcntx == net_loc_upd_v3 || appcntx == net_loc_upd_v2 )
    tsm = new ULTSM(ltrid,appcntx,tco);
  if ( appcntx == shortMsgMoRelayContext_v2)
    tsm = new MOFTSM(ltrid,appcntx,tco);
  return tsm;
}
static uint8_t magic0780[] = {0x80};
BIT_STRING_t tcapversion = {magic0780,sizeof(magic0780),0x07,};

static uint8_t magic773[] = {0x00,0x11,0x86,0x05,0x01,0x01,0x01};
ASN__PRIMITIVE_TYPE_t pduoid = {magic773,sizeof(magic773)};

/* namespace processor */ } /* namespace mtsmsme */ } /* namespace smsc */ }
