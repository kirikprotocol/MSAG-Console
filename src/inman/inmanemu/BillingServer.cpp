#include "BillingServer.h"
#include <util/Exception.hpp>

#include <string>

#include <logger/Logger.h>
#include "inman/common/util.hpp"

namespace inmanemu { namespace server {

using smsc::inman::common::dump;
using inmanemu::MatrixKey;
using namespace smsc::util;


BillingServer::BillingServer() : buff(0), clnt(0)
{
    needToStop = false;
}

BillingServer::~BillingServer()
{
    socket.Abort();
    if (clnt) clnt->Abort();

}

void BillingServer::Init(const char * host, int port)
{
    logger = smsc::logger::Logger::getInstance("BillingServer");

    processor.init();

    //delete new char;

    //"localhost",10021,0
    if (socket.InitServer(host, port ,0)==-1)
        throw Exception("Failed to initialize socket on host '%s', port '%d'", host, port);

    if (socket.StartServer()==-1)
        throw Exception("Cannot start socket server on host '%s', port '%d'", host, port);

/*
    clnt = s.Accept();

    if (!clnt) m_ClientConnected = false;
    else m_ClientConnected = true;

*/

    //ObjectBuffer buf(0);
    smsc_log_debug(logger, "Server initialized of host '%s', port '%d'", host, port);

}


bool BillingServer::ClientConnected()
{
    clnt = socket.Accept();

    if (!clnt) m_ClientConnected = false;
    else 
    {
        m_ClientConnected = true;
        printf("Client accepted\n");
    }

    
    return m_ClientConnected;
}

void BillingServer::Stop()
{
    needToStop = false;
}


SerializableObject * BillingServer::ReadCommand()
{
    int len;

    if(clnt->ReadAll((char*)&len,4)!=4)
    {
        printf("read failed\n");
        return 0;
    }

    len = ntohl(len);
    printf("received:%d bytes\n",len);

    buff.reset(len);

    if (clnt->ReadAll((char*)buff.get(),len)!=len)
    {
      printf("read failed\n");
      return 0;
    }
    buff.setDataSize(len);

    std::string dstr;
    //dump(dstr, len, (unsigned char*)(buff), false);

    //smsc_log_debug(logger,"received:%s",dstr.c_str());

    SerializableObject* obj=SerializerInap::getInstance()->deserialize(buff);
    obj->load(buff);

    printf("received obj dialogId=%d\n",obj->getDialogId());
    return obj;
}


ChargeSmsResult * BillingServer::CreateRespOnCharge(SerializableObject * obj)
{
    smsc::inman::interaction::ChargeSms * op = dynamic_cast<smsc::inman::interaction::ChargeSms *>(obj);

    if (!op) 
    {
        printf("command invalid\n");
        return 0;
    }

    CDRRecord cdr;
    op->export2CDR(cdr);


    printf("Charge SMS:");
    printf("ServiceId = %d\n",cdr._serviceId);
    printf("userMsgRef=%d\n",cdr._userMsgRef);
    printf("service number=%s\n",cdr._dstAdr.c_str());
    printf("abonent %s\n", cdr._srcAdr.c_str());
    printf("--------------\n\n");

    int charge=1;
  /*if(argc>1)
  {
    printf("Allow charge[0,1]?:");fflush(stdout);
    scanf("%d",&charge);
  } */

  //ChargeSmsResult* res=charge?new ChargeSmsResult():new ChargeSmsResult(16);

    MatrixKey key;
    key.serviceNumber = atoi(cdr._dstAdr.c_str());

    Address addr(cdr._srcAdr.c_str());
    ChargeSmsResult* res;

  if (processor.charge(key, addr, obj->getDialogId()))
      res = new ChargeSmsResult(CHARGING_POSSIBLE);
  else
      res = new ChargeSmsResult(CHARGING_NOT_POSSIBLE);

  res->setDialogId(obj->getDialogId());
  return res;
}

void BillingServer::ProcessResultCommand(SerializableObject * obj)
{
    smsc::inman::interaction::DeliverySmsResult * op = dynamic_cast<smsc::inman::interaction::DeliverySmsResult *>(obj);

    int result = op->GetValue();

    printf("DeliverySmsResult\n");
    printf("BillId = %d\n", op->getDialogId());

    if (result) 
    {
        printf("rollback processing...\n");
        processor.rollback(op->getDialogId());
    }
    else
    {
        processor.commit(op->getDialogId());
        printf("commit processing...\n");
    }

    printf("--------------\n\n");


}

void BillingServer::SendResp(SerializableObject * resp)
{
    buff.setPos(0);
    buff.setDataSize(0);
    SerializerInap::getInstance()->serialize(resp,buff);

    printf("sending %d bytes\n",buff.getDataSize());
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
         
            SerializableObject * obj = 0;
            SerializableObject * resp = 0;       
            obj = ReadCommand();

            if (obj)
            {
                switch (obj->getObjectId()) 
                {
                case CHARGE_SMS_TAG:

                    resp = 0;
                    resp = CreateRespOnCharge(obj);

                    if (resp) 
                    {
                        SendResp(resp);
                        delete resp;
                    }

                    break;
                case DELIVERY_SMS_RESULT_TAG:
                    ProcessResultCommand(obj);
                    break;
                }

                delete obj;
            } else 
            {
                //sleep(10);
                ClientConnected();
            }
        }
        else 
        {
            //sleep(10);
            ClientConnected();
        }
    }

}

}}
