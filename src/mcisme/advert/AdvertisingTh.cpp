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
// бывшая WaitBanner
int AdvertisingTh::ServBanner(advertising_item* advItem, SerializationBuffer* req, uint32_t req_len)
{

    int rc;
    {

        MutexGuard mg(advItem->eventMon);              // лочим сразу - до работы с хэшем

        // вставка в хэш - до записи в сокет
        {
            MutexGuard guard(hash_mtx);             // общий мьютех хэша (insert/delete)
        advPtrs.Insert(advItem->TransactID, advItem);
        }
        /*
          При работе чрз localhost на загруженной рабочими потоками машине возможна ситуация,
         когда сервер отвечает быстрее, чем рабочий поток успевает вставить в хэш информацию о
         данном запросе, как следствие не проходит проверка транспорта на наличие запроса в
         очереди ожидающих запросов.
        */
        rc = Write(req, req_len);
        if (rc == 0)
        {
            if (advItem->isAsync)
            {
                ((advertAsync_item*)advItem)->setLastTime(srv_timeout); // установка тайм-аута
                AsyncKeys.Push(advItem->TransactID);        // вставка ключа в стэк асинхронных запросов
            }
            else
            {
                if (advItem->eventMon.wait(srv_timeout))    // ожидание ответа сервера
                {
                    rc = ERR_ADV_TIMEOUT;
                    advItem->pakLen = -rc;
                }
            }
        }
        else  rc = ERR_ADV_SOCK_WRITE;        //ошибка записи в сокет
    }
    if (rc || advItem->isAsync == false)
        RemoveAdvert(advItem->TransactID);
    return rc;
}
//------------------------------------------------------------------------------
// удаление из хэша
//
void AdvertisingTh::RemoveAdvert(int advId)
{
    MutexGuard guard(hash_mtx);            // общий мьютех хэша  (insert/delete)
    advPtrs.Delete(advId);
}
//------------------------------------------------------------------------------
//
int AdvertisingTh::Write(SerializationBuffer* req, uint32_t req_len)
{
    if (wait_timeout != MIN_WAIT_TIME) // не пытаться блокировать закрытый сокет
        return ERR_ADV_SOCKET;

    MutexGuard sock_guard(sock_mtx);

    if (sock.canWrite(1) <= 0)       // можно ли писать в сокет?
        return  ERR_ADV_SOCKET;

    int rc = sock.Write((char *)req->getBuffer(), req_len); // пишем пакет
    if (rc < 0)
        return  ERR_ADV_SOCK_WRITE;

    return 0;
}

//------------------------------------------------------------------------------
//
int AdvertisingTh::Execute()
{
    TmpBuf<char, MAX_PACKET_LEN> buf(0);// буфер чтения данных из сокета

    smsc_log_debug(logger, "AdvertisingTh::Execute() started");
    wait_timeout = MIN_WAIT_TIME;

    advertising_item* curAdvItem;
    int rc;

    while(stop_th == false)
    {
        try
        {
            //[пере]инициализация сокета
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
                ReadAll(buf.GetCurPtr(), len);                  // читаем в буфер заголовок
          buf.SetPos(len);

                // проверка корректности пакета
                uint32_t word = ntohl(s[0]);
                if (word != CMD_BANNER_RSP)                     // тип должен быть ответом на запрос баннера
                {
                    smsc_log_error(logger, "AdvertisingTh::Execute(), incorrect packet type %d", word);
                    throw std::exception();
                }

                uint32_t pak_len = ntohl(s[1]) + CMD_HEADER;    // длина всего пакета
                if (pak_len > MAX_PACKET_LEN)                   // проверка на длину
                {
                    smsc_log_warn(logger, "AdvertisingTh::Execute(), bad packet length");
                    throw std::exception();
                }

        buf.setSize(pak_len);                   // место под весь пакет
                ReadAll(buf.GetCurPtr(), pak_len-len);  // вычитываем из сокета оставшeюся часть пакета

        // определение приемника
                word = ntohl(s[3]);                     //  значение TransactID(оно же = hash key)
                /*{
                    // теоретически остается возможность уничтожения элемента в потоке AdvertisAsync
                    // уже после проверки его наличия в общей очереди
                    MutexGuard mg(AsyncItemDeleteMtx);      // лочим удаление
                */
                    curAdvItem = GetAdvertItem(word);       // проверка наличия
                    if (curAdvItem)                         // еще в хэше
                    {
                        MutexGuard mg(curAdvItem->eventMon);
                        if (curAdvItem->pakLen == 0)   // пакет еще ожидается
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
                                curAdvItem->buf->SetPos(0);         // вообще-то лишнее
                                dest = curAdvItem->buf->GetCurPtr();
                            }
                            memcpy(dest, (char*)s, pak_len);
                            curAdvItem->pakLen = pak_len;
                            if (curAdvItem->isAsync)
                                  AsyncAdverts.Push((advertAsync_item*)curAdvItem); // очередь полученных от сервера указателей
                            else  curAdvItem->eventMon.notify();                    // ServBanner уведомляется о завершении операции
                        }//MutexGuard(curAdvItem->eventMon);
                   }//if (curAdvItem)
               // }//MutexGuard mg(AsyncItemDeleteMtx
            }//while(stop_th == false)
        }//try
        catch(...)
        {
            //при ошибке соединения, время между попытками соединиться увеличивается в 2 раза
            //пока не достигнет MAX_WAIT_TIME или не восстановится соединение
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
        MutexGuard guard(hash_mtx);         // общий мьютех хэша  (insert/delete)
        if (advPtrs.Exist(key))
            curAdvItem = advPtrs.Get(key);
    }
    return curAdvItem;
}

//------------------------------------------------------------------------------
// Socket::ReadAll(), но  со вставленным в цикл  вызовом CanRead
// и обработкой ошибок (запись в лог, генерация exeptiona)
int AdvertisingTh::ReadAll(char* buf,int size)
{
  int rd = 0, res;
  while(rd < size)
  {
    res = sock.canRead();               // можно ли читать из сокета?
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
// очистка асинхронной очереди
void AdvertisingTh::ClearAsyncs()
{
    advertising_item* asyncItem;
    int key;
    while(AsyncKeys.Pop(key)) // пока очередь не пуста
    {
        asyncItem = (advertAsync_item*) GetAdvertItem(key);
        if( asyncItem)
        {
            RemoveAdvert(asyncItem->TransactID);    // удаление из общего списка
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
