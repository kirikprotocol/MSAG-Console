#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "BillingServer.h"
#include <util/Exception.hpp>

#include <string>

//#include "util/BinDump.hpp"
//using smsc::util::DumpHex;

namespace inmanemu { namespace server {

using inmanemu::MatrixKey;
using namespace smsc::util;

BillingServer::BillingServer() : buff(0), clnt(0)
{
    needToStop = false;
    INPSerializer::getInstance()->registerCmdSet(INPCSBilling::getInstance());
}

BillingServer::~BillingServer()
{
    //socket.Abort();
    //if (clnt) clnt->Abort();
    //if (fileStorage) delete fileStorage;
}

void BillingServer::Init(const std::string& host, int port, const std::string& cdr_dir)
{
    logger = smsc::logger::Logger::getInstance("BillingServer");

    processor.init();

    //delete new char;

    //"localhost",10021,0
    if (socket.InitServer(host.c_str(), port ,0)==-1)
        throw Exception("Failed to initialize socket on host '%s', port '%d'", host.c_str(), port);

    if (socket.StartServer()==-1)
        throw Exception("Cannot start socket server on host '%s', port '%d'", host.c_str(), port);

/*
    clnt = s.Accept();

    if (!clnt) m_ClientConnected = false;
    else m_ClientConnected = true;

*/

    //ObjectBuffer buf(0);

    //fileStorage = new InBillingFileStorage(cdr_dir, 0, logger);

    smsc_log_debug(logger, "Server initialized of host '%s', port '%d'", host.c_str(), port);

}


bool BillingServer::ClientConnected()
{
    clnt = socket.Accept();

    if (!clnt) m_ClientConnected = false;
    else 
    {
        m_ClientConnected = true;
        smsc_log_debug(logger,"Client accepted\n");
    }

    
    return m_ClientConnected;
}

void BillingServer::Stop()
{
    smsc_log_debug(logger, "Stopping server");
    needToStop = true;
    socket.Abort();
    if (clnt) clnt->Abort();
}


INPPacketAC * BillingServer::ReadCommand()
{
    int len;

    if(clnt->ReadAll((char*)&len,4)!=4)
    {
        smsc_log_debug(logger, "read failed\n");
        return 0;
    }

    len = ntohl(len);
    smsc_log_debug(logger, "received:%d bytes\n",len);

    buff.reset(len);

    if (clnt->ReadAll((char*)buff.get(),len)!=len)
    {
      smsc_log_debug(logger, "read failed\n");
      return 0;
    }
    buff.setDataSize(len);

    //smsc_log_debug(logger,"received: 0x%s", DumpHex(len, (unsigned char*)(buff)).c_str());

    auto_ptr<INPPacketAC> pck(INPSerializer::getInstance()->deserialize(buff)); //throws

    if ((pck->pHdr())->Id() != INPCSBilling::HDR_DIALOG) {
        smsc_log_error(logger, "received cmd %u: unknown header: %u",
                       (pck->pCmd())->Id(), (pck->pHdr())->Id());
        return 0;
    }
    CsBillingHdr_dlg * srvHdr = static_cast<CsBillingHdr_dlg*>(pck->pHdr());
    smsc_log_debug(logger, "received cmd %u, dialogId = %u\n", (pck->pCmd())->Id(), srvHdr->dlgId);

    return pck.release();
}


SPckChargeSmsResult * BillingServer::CreateRespOnCharge(SPckChargeSms* pck)
{
    if (!pck)
    {
        smsc_log_error(logger, "CreateRespOnCharge(): command invalid\n");
        return 0;
    }

    CDRRecord cdr;
    pck->Cmd().export2CDR(cdr);
    //fileStorage->bill(cdr);
    

    smsc_log_debug(logger, "Charge SMS:");
    smsc_log_debug(logger, "ServiceId = %d\n",cdr._serviceId);
    smsc_log_debug(logger, "userMsgRef=%d\n",cdr._userMsgRef);
    smsc_log_debug(logger, "service number=%s\n",cdr._dstAdr.c_str());
    smsc_log_debug(logger, "abonent %s\n", cdr._srcAdr.c_str());
    smsc_log_debug(logger, "--------------\n\n");

    int charge=1;
  /*if(argc>1)
  {
    printf("Allow charge[0,1]?:");fflush(stdout);
    scanf("%d",&charge);
  } */

    MatrixKey key;
    if (cdr._dstAdr.length() > 0) 
        key.serviceNumber = atoi(cdr._dstAdr.c_str());
    else
        key.serviceNumber = 0;

    Address addr((cdr._srcAdr.length()>0) ? cdr._srcAdr.c_str(): "0");

    SPckChargeSmsResult * res = new SPckChargeSmsResult();

    //std::auto_ptr<SPckChargeSmsResult> res(new SPckChargeSmsResult()); //dflt: CHARGING_POSSIBLE
    res->Hdr().dlgId = pck->Hdr().dlgId;

    smsc_log_debug(logger, "Start charging \n\n");

    if (!processor.charge(key, addr, pck->Hdr().dlgId))
        res->Cmd().setValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);

    smsc_log_debug(logger, "Finish charging \n\n");
    return res;
}

void BillingServer::ProcessResultCommand(SPckDeliverySmsResult * pck)
{
    int result = pck->Cmd().GetValue();
    smsc_log_debug(logger, "DeliverySmsResult: %u\n", result);
    smsc_log_debug(logger, "BillId = %u\n", pck->Hdr().dlgId);

    CDRRecord cdr;
    pck->Cmd().export2CDR(cdr);
    //fileStorage->bill(cdr);

    if (result) 
    {
        smsc_log_debug(logger, "rollback processing...\n");
        processor.rollback(pck->Hdr().dlgId);
    }
    else
    {
        processor.commit(pck->Hdr().dlgId);
        smsc_log_debug(logger, "commit processing...\n");
    }
    smsc_log_debug(logger, "--------------\n\n");
}

void BillingServer::SendResp(SPckChargeSmsResult * pck)
{
    buff.setPos(0);
    buff.setDataSize(0);
    pck->serialize(buff);

    smsc_log_debug(logger, "\nSending responce: %d bytes\n", buff.getDataSize());
    smsc_log_debug(logger, "-------------------\n");

    /*
    std::string s;

    for(int i=0;i<buff.getDataSize();i++)
    {
      char b[32];
      sprintf(b,"%02X",buff.get()[i]);
      s+=b;
    }
    smsc_log_debug(logger,"out:%s",s.c_str());
    */
    long len = htonl(buff.getDataSize());

    clnt->WriteAll((char*)&len,4);
    clnt->WriteAll(buff.get(), buff.getDataSize());

}

void BillingServer::Run()
{
    ClientConnected();

    while(isStarted())
    {
        if (m_ClientConnected) 
        {
            std::auto_ptr<INPPacketAC> pck;
            try { pck.reset(ReadCommand());
            } catch (std::exception & exc) {
                smsc_log_error(logger, "Run(): %s", exc.what());
            }

            if (pck.get())
            {
                switch ((pck->pCmd())->Id())
                {
                case INPCSBilling::CHARGE_SMS_TAG: {
                    std::auto_ptr<SPckChargeSmsResult> resp(CreateRespOnCharge(static_cast<SPckChargeSms*>(pck.get())));
                    if (resp.get()) 
                        SendResp(resp.get());
                } break;

                case INPCSBilling::DELIVERY_SMS_RESULT_TAG:
                    ProcessResultCommand(static_cast<SPckDeliverySmsResult*>(pck.get()));
                    break;
                }
            } else
            {
                //sleep(10);
                if (isStarted()) ClientConnected();
            }
        }
        else 
        {
            //sleep(10);
            if (isStarted()) ClientConnected();
        }
    }

}

}}
