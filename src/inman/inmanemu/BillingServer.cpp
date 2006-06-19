#include "BillingServer.h"
#include <util/Exception.hpp>

#include <string>

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
/*    socket.Abort();
    if (clnt) clnt->Abort();*/
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


SerializableObject * BillingServer::ReadCommand()
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

    std::string dstr;
    //dump(dstr, len, (unsigned char*)(buff), false);

    //smsc_log_debug(logger,"received:%s",dstr.c_str());

    SerializableObject* obj=SerializerInap::getInstance()->deserialize(buff);
    obj->load(buff);

    smsc_log_debug(logger, "received obj dialogId=%d\n",obj->getDialogId());
    return obj;
}


ChargeSmsResult * BillingServer::CreateRespOnCharge(SerializableObject * obj)
{
    smsc::inman::interaction::ChargeSms * op = dynamic_cast<smsc::inman::interaction::ChargeSms *>(obj);

    if (!op) 
    {
        smsc_log_debug(logger, "command invalid\n");
        return 0;
    }

    CDRRecord cdr;
    op->export2CDR(cdr);
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

  //ChargeSmsResult* res=charge?new ChargeSmsResult():new ChargeSmsResult(16);

    MatrixKey key;
    key.serviceNumber = atoi(cdr._dstAdr.c_str());

    Address addr(cdr._srcAdr.c_str());
    ChargeSmsResult* res;

  if (processor.charge(key, addr, obj->getDialogId()))
      res = new ChargeSmsResult(0, CHARGING_POSSIBLE);
  else
      res = new ChargeSmsResult(0, CHARGING_NOT_POSSIBLE);

  res->setDialogId(obj->getDialogId());
  return res;
}

void BillingServer::ProcessResultCommand(SerializableObject * obj)
{
    smsc::inman::interaction::DeliverySmsResult * op = dynamic_cast<smsc::inman::interaction::DeliverySmsResult *>(obj);

    int result = op->GetValue();

    smsc_log_debug(logger, "DeliverySmsResult\n");
    smsc_log_debug(logger, "BillId = %d\n", op->getDialogId());

    CDRRecord cdr;
    op->export2CDR(cdr);
    //fileStorage->bill(cdr);

    if (result) 
    {
        smsc_log_debug(logger, "rollback processing...\n");
        processor.rollback(op->getDialogId());
    }
    else
    {
        processor.commit(op->getDialogId());
        smsc_log_debug(logger, "commit processing...\n");
    }

    smsc_log_debug(logger, "--------------\n\n");


}

void BillingServer::SendResp(SerializableObject * resp)
{
    buff.setPos(0);
    buff.setDataSize(0);
    SerializerInap::getInstance()->serialize(resp,buff);

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
