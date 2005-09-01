#include "TCO.hpp"
#include "util.hpp"
#include "logger/Logger.h"

namespace smsc{namespace mtsmsme{namespace processor{

using namespace smsc::mtsmsme::processor::util;

static void SCCPsend(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp);

static int reqnum = 0;
static unsigned long sm_mt_relay_v1_buf[] = {0,4,0,0,1,0,25,1};
static unsigned long sm_mt_relay_v2_buf[] = {0,4,0,0,1,0,25,2};
static unsigned long sm_mt_relay_v3_buf[] = {0,4,0,0,1,0,25,3};
static unsigned long           null_buf[] = {0,0,0,0,0,0,0,0};
AC null_ac        = AC(null_buf,sizeof(null_buf)/sizeof(unsigned long));
AC sm_mt_relay_v1 = AC(sm_mt_relay_v1_buf,sizeof(sm_mt_relay_v1_buf)/sizeof(unsigned long));
AC sm_mt_relay_v2 = AC(sm_mt_relay_v2_buf,sizeof(sm_mt_relay_v2_buf)/sizeof(unsigned long));
AC sm_mt_relay_v3 = AC(sm_mt_relay_v3_buf,sizeof(sm_mt_relay_v3_buf)/sizeof(unsigned long));

using smsc::logger::Logger;
static Logger *logger = 0;

TCO::TCO(int TrLimit)
{
  logger = Logger::getInstance("mt.sme.tco");
  TrId id; id.size=4; id.buf[0] = 0xBA; id.buf[1] = 0xBE;
  for (unsigned n=1;n<=TrLimit;++n){
    id.buf[2] = (n >> 8) & 0xFF;
    id.buf[3] = n & 0xFF;
    tridpool.push_back(id);
  }
}
TCO::~TCO()
{
  smsc_log_warn(logger,"please clean TSM in the TCO::~TCO()");
}

void TCO::NUNITDATA(uint8_t cdlen,
                    uint8_t *cd, /* called party address */
                    uint8_t cllen,
                    uint8_t *cl, /* calling party address */
                    uint16_t ulen,
                    uint8_t *udp /* user data */
                   )
{
    //using namespace smsc::mtsmsme::processor::encode;
    //using namespace smsc::mtsmsme::processor::decode;
    Message msg;
    decoder.decode(udp,ulen,msg);
    if (msg.isBegin())
    {
      TrId rtrid;
      AC appcntx;
      rtrid = msg.getOTID();
      msg.getAppContext(appcntx);


      if (!msg.isDialoguePortionExist() && appcntx != sm_mt_relay_v1)
      {
        smsc_log_debug(logger,"tco receive BEGIN with no dialogue portion, send ABORT(badlyFormattedTransactionPortion)");
        std::vector<unsigned char> rsp;
        encoder.encodeBadTrPortion(rtrid,rsp);
        SCCPsend(cllen,cl,cdlen,cd,rsp.size(),&rsp[0]);
        return;
      }

      if (msg.isDialoguePortionExist() && appcntx != sm_mt_relay_v2 && appcntx != sm_mt_relay_v3)
      {
        smsc_log_debug(logger,"tco receive BEGIN with unsupported app context, send ABORT(application-context-name-not-supported)");
        std::vector<unsigned char> rsp;
        encoder.encodeACNotSupported(rtrid,appcntx,rsp);
        SCCPsend(cllen,cl,cdlen,cd,rsp.size(),&rsp[0]);
        return;
      }

      if ( tridpool.empty() ) {
        smsc_log_debug(logger,"tco BEGIN and transaction pool is empty, send ABORT(resourceLimitation)");
        std::vector<unsigned char> rsp;
        encoder.encodeResourceLimitation(rtrid,rsp);
        SCCPsend(cllen,cl,cdlen,cd,rsp.size(),&rsp[0]);
        return;
      }

      // выделить id, создать TSM, подать на вход BEGIN
      TrId ltrid = tridpool.front();
      TSM* tsm = new TSM(ltrid,appcntx,this);
      tridpool.pop_front();
      tsms.Insert(ltrid,tsm);
      tsm->BEGIN_received(cdlen,cd,cllen,cl,rtrid,msg);
    } /* end of BEGIN handling section */
    if (msg.isContinue())
    {
      TrId ltrid;
      ltrid = msg.getDTID();
      TSM** ptr = tsms.GetPtr(ltrid);
      if (ptr)
      {
        TSM* tsm = *ptr;
        tsm->CONTINUE_received(msg);
      }
      else
      {
        smsc_log_debug(logger,"tco receive CONTINUE and TSM not found, DISCARD, but need to send ABORT");
      }
    }/* end of CONTINUE handling section */
}
void TCO::TSMStopped(TrId ltrid)
{
  tridpool.push_back(ltrid);
  TSM** ptr = tsms.GetPtr(ltrid);
  if (ptr)
  {
    tsms.Delete(ltrid);
    delete(*ptr);
  }
}

void TCO::setRequestSender(RequestSender* _sender)
{
  sender = _sender;
}

UCHAR_T resp[] = {
0x64, 0x39,
0x49, 0x04, 0x32, 0x07, 0x00, 0x00,  //orig transaction
0x6b, 0x2a, 0x28, 0x28, 0x06, 0x07, 0x00, 0x11, 0x86, 0x05, 0x01, 0x01, 0x01, 0xa0, 0x1d, //dialog portion
0x61, 0x1b, 0x80, 0x02, 0x07, 0x80, 0xa1, 0x09, 0x06, 0x07, 0x04, 0x00, 0x00, 0x01, 0x00, //dialog resp
0x19, 0x02, 0xa2, 0x03, 0x02, 0x01, 0x00, 0xa3, 0x05, 0xa1, 0x03, 0x02, 0x01, 0x00,
0x6c, 0x05, 0xa2, 0x03, 0x02, 0x01, 0x01
};
/*
  64   39   49   04   E0   03   00   00
  6B   2A   28   28   06   07   00   11
  86   05   01   01   01   A0   1D   61
  1B   80   02   07   80   A1   09   06
  07   04   00   00   01   00   19   02
  A2   03   02   01   00   A3   05   A1
  03   02   01   00   6C   05   A2   03
  02   01   01
*/
//void TCO::sendResponse(int result)
//{
  /* fix ssn from 191 to 8 */
/*
  UCHAR_T  tcdlen = cdlen;
  UCHAR_T* tcd = (UCHAR_T*)malloc(tcdlen);
  memcpy(tcd,cd,tcdlen);
  tcd[1] = 0x08;
*/
  /* fix transaction id */

//  memcpy(resp+4,udp+4,4);
//  USHORT_T res;
//  res =
//  EINSS7_I96SccpUnitdataReq(
//                              SSN,
//                              sls++,
//                              EINSS7_I96SCCP_SEQ_CTRL_OFF,
//                              EINSS7_I96SCCP_RET_OPT_OFF,
//                              0,                          /* UCHAR_T messPriImportance    */
//                              0,                          /* BOOLEAN_T hopCounterUsed     */
//                              cllen,                      /* UCHAR_T calledAddressLength  */
//                              cl,                         /* UCHAR_T *calledAddress_p     */
//                              tcdlen,                     /* UCHAR_T callingAddressLength */
//                              tcd,                        /* UCHAR_T *callingAddress_p    */
//                              sizeof(resp),               /* USHORT_T userDataLength      */
//                              resp                        /* UCHAR_T *userData_p          */
//                           );
//  if (res != 0)
//  {
//    smsc_log_error(logger,
//                   "EINSS7_I96SccpUnitdataReq failed with code %d(%s)",
//                   res,getReturnCodeDescription(res));
//  }
//  free(tcd);
//}

#define SSN 191
static UCHAR_T sls=0;
void TCO::SCCPsend(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp)
{
  USHORT_T res;
  res =
  EINSS7_I96SccpUnitdataReq(
                           SSN,
                           sls++,
                           EINSS7_I96SCCP_SEQ_CTRL_OFF,
                           EINSS7_I96SCCP_RET_OPT_OFF,
                           0,                          /* UCHAR_T messPriImportance    */
                           0,                          /* BOOLEAN_T hopCounterUsed     */
                           cdlen,                      /* UCHAR_T calledAddressLength  */
                           cd,                         /* UCHAR_T *calledAddress_p     */
                           cllen,                      /* UCHAR_T callingAddressLength */
                           cl,                         /* UCHAR_T *callingAddress_p    */
                           ulen,                       /* USHORT_T userDataLength      */
                           udp                       /* UCHAR_T *userData_p          */
                          );
  if (res != 0)
  {
    smsc_log_error(logger,
                   "EINSS7_I96SccpUnitdataReq failed with code %d(%s)",
                   res,getReturnCodeDescription(res));
  }
}

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
