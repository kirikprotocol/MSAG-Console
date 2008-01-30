#include "AdvertisingTh.h"
#include "AdvertisingImpl.h"

using namespace std;

namespace scag {
namespace advert {
//------------------------------------------------------------------------------
//
void AdvertisingTh::Init(const std::string& host, int port, int timeout)
{
    sock.Close();

    srv_timeout = timeout;
  srv_host = host;
  srv_port = port;
}

//------------------------------------------------------------------------------
//
// ������ WaitBanner
int AdvertisingTh::ServBanner(advertising_item* advItem, SerializationBuffer* req, uint32_t req_len)
{

    int rc;
    {

        MutexGuard mg(advItem->eventMon);              // ����� ����� - �� ������ � �����

        // ������� � ��� - �� ������ � �����
        {
            MutexGuard guard(hash_mtx);             // ����� ������ ���� (insert/delete)
        advPtrs.Insert(advItem->TransactID, advItem);
        }
        /*
          ��� ������ ��� localhost �� ����������� �������� �������� ������ �������� ��������,
         ����� ������ �������� �������, ��� ������� ����� �������� �������� � ��� ���������� �
         ������ �������, ��� ��������� �� �������� �������� ���������� �� ������� ������� �
         ������� ��������� ��������.
        */
        rc = Write(req, req_len);
        if (rc == 0)
        {
            if (advItem->isAsync)
            {
                ((advertAsync_item*)advItem)->setLastTime(srv_timeout); // ��������� ����-����
                AsyncKeys.Push(advItem->TransactID);        // ������� ����� � ���� ����������� ��������
            }
            else
            {
                if (advItem->eventMon.wait(srv_timeout))    // �������� ������ �������
                {
                    rc = ERR_ADV_TIMEOUT;
                    advItem->pakLen = -rc;
                }
            }
        }
        else  rc = ERR_ADV_SOCK_WRITE;        //������ ������ � �����
    }
    if (rc || advItem->isAsync == false)
        RemoveAdvert(advItem->TransactID);
    return rc;
}
//------------------------------------------------------------------------------
// �������� �� ����
//
void AdvertisingTh::RemoveAdvert(int advId)
{
    MutexGuard guard(hash_mtx);            // ����� ������ ����  (insert/delete)
    advPtrs.Delete(advId);
}
//------------------------------------------------------------------------------
//
int AdvertisingTh::Write(SerializationBuffer* req, uint32_t req_len)
{
    if (wait_timeout != MIN_WAIT_TIME) // �� �������� ����������� �������� �����
        return ERR_ADV_SOCKET;

    MutexGuard sock_guard(sock_mtx);

    if (sock.canWrite(1) <= 0)       // ����� �� ������ � �����?
        return  ERR_ADV_SOCKET;

    int rc = sock.Write((char *)req->getBuffer(), req_len); // ����� �����
    if (rc < 0)
        return  ERR_ADV_SOCK_WRITE;

    return 0;
}

//------------------------------------------------------------------------------
//
int AdvertisingTh::Execute()
{
    TmpBuf<char, MAX_PACKET_LEN> buf(0);// ����� ������ ������ �� ������

    smsc_log_debug(logger, "AdvertisingTh::Execute() started");
    wait_timeout = MIN_WAIT_TIME;

    advertising_item* curAdvItem;
    int rc;

    while(stop_th == false)
    {
        try
        {
            //[����]������������� ������
            rc = sock.Init(srv_host.c_str(), srv_port, srv_timeout);
            //sock.setNonBlocking(1);

            if (rc == 0)
                rc = -sock.Connect();

            if (rc != 0)
            {
                smsc_log_error(logger, "Advertising::Execute(), can't %s server  host = %s, port = %d",
                                             rc >0 ? "connect to":"init", srv_host.c_str(), srv_port);
                throw std::exception();
            }

            wait_timeout = MIN_WAIT_TIME;
            while(stop_th == false)
            {
                buf.SetPos(0);
                uint32_t *s = (uint32_t*)buf.GetCurPtr();

                int len = CMD_HEADER + sizeof(uint32_t)*3;
                ReadAll(buf.GetCurPtr(), len);                  // ������ � ����� ���������
          buf.SetPos(len);

                // �������� ������������ ������
                uint32_t word = ntohl(s[0]);
                if (word != CMD_BANNER_RSP)                     // ��� ������ ���� ������� �� ������ �������
                {
                    smsc_log_error(logger, "AdvertisingTh::Execute(), incorrect packet type %d", word);
                    throw std::exception();
                }

                uint32_t pak_len = ntohl(s[1]) + CMD_HEADER;    // ����� ����� ������
                if (pak_len > MAX_PACKET_LEN)                   // �������� �� �����
                {
                    smsc_log_warn(logger, "AdvertisingTh::Execute(), bad packet length");
                    throw std::exception();
                }

        buf.setSize(pak_len);                   // ����� ��� ���� �����
                ReadAll(buf.GetCurPtr(), pak_len-len);  // ���������� �� ������ ������e��� ����� ������

        // ����������� ���������
                word = ntohl(s[3]);                     //  �������� TransactID(��� �� = hash key)
                /*{
                    // ������������ �������� ����������� ����������� �������� � ������ AdvertisAsync
                    // ��� ����� �������� ��� ������� � ����� �������
                    MutexGuard mg(AsyncItemDeleteMtx);      // ����� ��������
                */
                    curAdvItem = GetAdvertItem(word);       // �������� �������
                    if (curAdvItem)                         // ��� � ����
                    {
                        MutexGuard mg(curAdvItem->eventMon);
                        if (curAdvItem->pakLen == 0)   // ����� ��� ���������
                        {
                            //memcpy(curAdvItem->buf, (char*)s, pak_len);
                            char* dest;
                            if (curAdvItem->isAsync)
                            {
                                curAdvItem->asyncBuf = new char[pak_len];
                                dest = curAdvItem->asyncBuf;
                            }
                            else
                            {
                                curAdvItem->buf->setSize(pak_len);
                                curAdvItem->buf->SetPos(0);         // ������-�� ������
                                dest = curAdvItem->buf->GetCurPtr();
                            }
                            memcpy(dest, (char*)s, pak_len);
                            curAdvItem->pakLen = pak_len;
                            if (curAdvItem->isAsync)
                                  AsyncAdverts.Push((advertAsync_item*)curAdvItem); // ������� ���������� �� ������� ����������
                            else  curAdvItem->eventMon.notify();                    // ServBanner ������������ � ���������� ��������
                        }//MutexGuard(curAdvItem->eventMon);
                   }//if (curAdvItem)
               // }//MutexGuard mg(AsyncItemDeleteMtx
            }//while(stop_th == false)
        }//try
        catch(...)
        {
            //��� ������ ����������, ����� ����� ��������� ����������� ������������� � 2 ����
            //���� �� ��������� MAX_WAIT_TIME ��� �� ������������� ����������
            sock_wait.Wait(wait_timeout);
            if (wait_timeout < MAX_WAIT_TIME)
                wait_timeout*=2;
            smsc_log_warn(logger, "AdvertisingTh::Execute(), wait_timeout = %d", wait_timeout);
        }
    }//while(stop_th == false)

    ClearAsyncs();

    smsc_log_debug(logger, "AdvertisingTh::Execute(), leave");
    return 0;
};
//------------------------------------------------------------------------------
//
advertising_item* AdvertisingTh::GetAdvertItem(int key)
{
    advertising_item*  curAdvItem = NULL;
    {
        MutexGuard guard(hash_mtx);         // ����� ������ ����  (insert/delete)
        if (advPtrs.Exist(key))
            curAdvItem = advPtrs.Get(key);
    }
    return curAdvItem;
}

//------------------------------------------------------------------------------
// Socket::ReadAll(), ��  �� ����������� � ����  ������� CanRead
// � ���������� ������ (������ � ���, ��������� exeptiona)
int AdvertisingTh::ReadAll(char* buf,int size)
{
  int rd = 0, res;
  while(rd < size)
  {
    res = sock.canRead();               // ����� �� ������ �� ������?
    if (res > 0)
        res = sock.Read(buf+rd, size-rd);
    if (res <= 0)
    {
        smsc_log_warn(logger, "AdvertisingTh::ReadAll(), socket closed");
        throw std::exception();
    }
    rd += res;
  }
  return size;
}
//------------------------------------------------------------------------------
// ������� ����������� �������
void AdvertisingTh::ClearAsyncs()
{
    advertising_item* asyncItem;
    int key;
    while(AsyncKeys.Pop(key)) // ���� ������� �� �����
    {
        asyncItem = (advertAsync_item*) GetAdvertItem(key);
        if( asyncItem)
        {
            RemoveAdvert(asyncItem->TransactID);    // �������� �� ������ ������
            delete asyncItem;
        }
    }
}

//------------------------------------------------------------------------------
//
void AdvertisingTh::Stop()
{
    srv_timeout = 0;
    wait_timeout = 0;
    stop_th = true;
    sock_wait.Signal();
    sock.Abort();
    pthread_cancel(thread);
    //WaitFor();
}

} // adverTh
} // scag
