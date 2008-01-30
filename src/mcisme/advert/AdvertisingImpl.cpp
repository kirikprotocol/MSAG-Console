#include"AdvertisingImpl.h"


namespace scag {
namespace advert {


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//поток след€щий за асинхронными вызовами
//
int AdvertisAsync::Execute()
{

    advertAsync_item* curAdvItem;
    int key;

    while(stop == false)
    {
        AsyncAdverts->WaitTime(timeout);   // ждем изменени€ (пополнени€) очереди
        // обработка полученных от сервера ответов
        while (stop == false && AsyncAdverts->Pop(curAdvItem))
        {
            {
                MutexGuard mg(curAdvItem->eventMon);

                int rc = curAdvItem->pakLen < 0 ? -curAdvItem->pakLen:0;
                AdvertImpl->ServAsyncAdvert(curAdvItem, rc);
            }
//            MutexGuard mg(AdvertTh->AsyncItemDeleteMtx);
            delete curAdvItem;
        }
        AdvertImpl->CheckAsyncTimes();  // проверка тайм-аутов
    }
    return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// поиск устаревших асинхронных запросов и удаление их из общего списка
/*/
int AdvertisTimed::Execute()
{
    int key;
    advertAsync_item* asyncItem;
    while(stop == false)
    {
        pause.Wait(timeout);                            // пауза
        AdvertImpl->CheckAsyncTimes();
    }
    return 0;
}
*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
AdvertisingImpl::AdvertisingImpl()
{
//        Logger::Init();
      logger=Logger::getInstance("scag.advert.Advertising");
        smsc_log_debug(logger, "adv->AdvertisingImpl()");

        adv_thread.logger = logger;
}

//------------------------------------------------------------------------------
//
AdvertisingImpl::~AdvertisingImpl()
{
  smsc_log_debug(logger, "AdvertisingImpl::~AdvertisingImpl()");
}
//------------------------------------------------------------------------------
//
void AdvertisingImpl::init(const std::string& host, int port, int _timeout, int _maxcount)
{
        timeout  = _timeout;
        maxcount = _maxcount;

        AsyncAdverts = &adv_thread.AsyncAdverts;
        AsyncKeys    = &adv_thread.AsyncKeys;

        adv_asyncs.Init(this, AsyncAdverts, timeout);
        adv_asyncs.Start();

        adv_thread.Init(host, port, timeout);
        adv_thread.Start();

};

//------------------------------------------------------------------------------
//
//
void AdvertisingImpl::CheckBannerRequest(BannerRequest& banReq, int async)
{

    smsc_log_debug(logger,"Advert::askBanner() abonent = %s, serviceName = %s, transportType = %d, charSet = %d, maxLen = %d, async = %d",
            banReq.abonent.c_str(), banReq.serviceName.c_str(), banReq.transportType, banReq.charSet, banReq.maxLen, async);

  __require__(banReq.abonent.length() > 5 && banReq.abonent.length() < 21);

/*  if (banReq.dispatcher)
    {
        BannerDispatcher* bd =  banReq.dispatcher;
        void (scag::advert::BannerDispatcher:: *procBanner)(const BannerRequest&) = bd->processBanner;
        void (scag::advert::BannerDispatcher:: *procError)(const BannerRequest& , uint32_t) = bd->processError;

      __require__(procBanner != NULL  && procError != NULL);
    }   */
}

//------------------------------------------------------------------------------
// обертка
//
uint32_t AdvertisingImpl::getBanner(const std::string& abonent,
                                    const std::string& serviceName,
                                    uint32_t transportType, uint32_t charSet,
                                    std::string &banner, uint32_t maxLen)
{
    BannerRequest req(abonent, serviceName, transportType, charSet, maxLen);
    uint32_t rc = getBanner(req);
    banner = req.banner;
  return  rc;
}
//------------------------------------------------------------------------------
//
//
uint32_t AdvertisingImpl::getBanner(BannerRequest& banReq)
{
    CheckBannerRequest(banReq);

    TmpBuf<char, BANNER_LEN> rsp(0);         // буфер дл€ ответов от сервера
    advertising_item curAdvItem(&banReq, &rsp);

    SerializationBuffer req;
    // заполнение буфера протокольной команды
    uint32_t len = PrepareCmnd(&req, &banReq, CMD_BANNER_REQ);


    int rc = adv_thread.ServBanner(&curAdvItem, &req, len);
    if (rc != 0)
    {
        WriteErrorToLog((char*)"AdvertisingImpl::ServBanner", rc);
        if (rc == ERR_ADV_TIMEOUT)
        {
            //если таймаут сервера - сообщить серверу об этой ошибке
            len = PrepareCmnd(&req, &banReq, CMD_ERR_INFO, ERR_ADV_TIMEOUT);
            len = adv_thread.Write(&req, len);
            if (len)    //используем len, чтобы не забивать rc
                WriteErrorToLog((char*)"AdvertisingImpl::Write", len);
        }
        return rc;
    }//if (rc != 0)

    rc = AnaliseResponse(&curAdvItem);
    if (rc)
       WriteErrorToLog((char*)"AdvertisingImpl::AnaliseResponse", rc);

    return rc;
};
//------------------------------------------------------------------------------
//
//
void AdvertisingImpl::requestBanner(const BannerRequest& constbanReq)
{
    BannerRequest* banReq = (BannerRequest*)&constbanReq;
    CheckBannerRequest(*banReq, 1);

    int rc;
    if (AsyncKeys->Count() >= maxcount)
    {
        AsyncAdverts->Notify();     //вдруг большой тайм-аут - разбудить
        rc = ERR_ADV_QUEUE_FULL;
    }
    else
    {
        advertising_item *curAdvItem = new advertAsync_item(banReq, timeout);

        SerializationBuffer req;
        // заполнение буфера протокольной команды
        uint32_t len = PrepareCmnd(&req, banReq, CMD_BANNER_REQ);
        rc = adv_thread.ServBanner(curAdvItem, &req, len);
        if (rc)
            delete curAdvItem;
    }
    if (rc)
    {
      WriteErrorToLog((char*)"AdvertisingImpl::requestBanner", rc);
        banReq->dispatcher->processError(constbanReq, rc);
    }
}

//------------------------------------------------------------------------------
// поиск устаревших асинхронных запросов и удаление их из общего списка
//
void AdvertisingImpl::CheckAsyncTimes()
{
    int key;
    advertAsync_item* asyncItem;
    timestruc_t now={-1, 0};
    clock_gettime(CLOCK_REALTIME,&now); // используем одно врем€ на всех

    while(AsyncKeys->Front(key))        // пока очередь не пуста
    {
        asyncItem = (advertAsync_item*) adv_thread.GetAdvertItem(key);
        if (asyncItem)                  // еще в списке
        {
            MutexGuard mg(asyncItem->eventMon);
            if (asyncItem->pakLen == 0) // еще не обработан
            {
                if ( now.tv_sec  <  asyncItem->tv.tv_sec  ||
                   ( now.tv_sec  == asyncItem->tv.tv_sec &&
                     now.tv_nsec <  asyncItem->tv.tv_nsec  ) )  break;// еще можно ждать

                asyncItem->pakLen = -ERR_ADV_TIMEOUT;   // признак просроченности
                AsyncAdverts->Push(asyncItem);          // вставка в очередь обработанных указателей
            }
        }//if (asyncItem)
        AsyncKeys->Pop(key);                            // удаление ключа из очереди
    }
}
//------------------------------------------------------------------------------
// обслуживание асинхронного запроса
//
int AdvertisingImpl::ServAsyncAdvert(advertising_item *curAdvItem, int er)
{
    adv_thread.RemoveAdvert(curAdvItem->TransactID);    // удаление из общего списка

    int rc = er;
    if (rc == 0)
        rc = AnaliseResponse(curAdvItem);
    else
    {
         if (rc == ERR_ADV_TIMEOUT)
        {
            //если таймаут сервера - сообщить серверу об этой ошибке
            SerializationBuffer req;
            int len  = PrepareCmnd(&req, curAdvItem->banReq, CMD_ERR_INFO, rc);
            len  = adv_thread.Write(&req, len);
            if (len)    //используем len, чтобы не забивать rc
                WriteErrorToLog((char*)"AdvertisingImpl::ServAsyncAdvert:SayError", len);
         }
    }

    const BannerRequest* banReq = curAdvItem->banReq;
    if (rc)
    {
          WriteErrorToLog((char*)"AdvertisingImpl::ServAsyncAdvert", rc);
          banReq->dispatcher->processError (*banReq, rc);
    }
    else  banReq->dispatcher->processBanner(*banReq);

  return rc;
}

//------------------------------------------------------------------------------
// запись в журнал сообщени€ об ошибке
//
void AdvertisingImpl::WriteErrorToLog(char* where, int errCode)
{
    switch(errCode)
    {
        case ERR_ADV_SOCKET     : smsc_log_error(logger, "%s, server hangs", where); break;
        case ERR_ADV_SOCK_WRITE   : smsc_log_error(logger, "%s, socket closed", where); break;
        case ERR_ADV_PACKET_TYPE  : smsc_log_error(logger, "%s, bad packet type", where); break;
        case ERR_ADV_PACKET_LEN   : smsc_log_error(logger, "%s, bad packet length", where); break;
        case ERR_ADV_PACKET_MEMBER  : smsc_log_error(logger, "%s, bad packet member", where); break;
        case ERR_ADV_QUEUE_FULL     : smsc_log_error(logger, "%s, queue is overloaded", where); break;

        default           : smsc_log_error(logger, "%s, error %d", where, errCode);
    }
}

//------------------------------------------------------------------------------
// заполнение буфера протокольной команды
//
uint32_t AdvertisingImpl::PrepareCmnd(SerializationBuffer* req, BannerRequest* par,
                                      uint32_t cmndType, uint32_t cmdInfo)
{

    uint32_t req_len;

    switch(cmndType)   // длина  пакета  дл€ каждой из команд клиента
    {
        case CMD_BANNER_REQ : req_len = GET_BANNER_REQ_LEN
                                      + par->abonent.length()
                                      + par->serviceName.length();
                            break;

        case CMD_ERR_INFO   : req_len = ERR_INFO_LEN;
                            break;
    }

    req->resize(req_len + CMD_HEADER);
    req->setPos(0);

    //command
    req->WriteNetInt32(cmndType);       //Command Type
    req->WriteNetInt32(req_len);      //Command length

    //TransactId45
    req->WriteNetInt32(sizeof(uint32_t)); //Param length
    req->WriteNetInt32(par->getId());   //Param body

    if(cmndType == CMD_ERR_INFO)
    {
        req->WriteNetInt32(sizeof(uint32_t));
        req->WriteNetInt32(cmdInfo);
    }
    else
    {
        // abonent
        req->WriteNetInt32(par->abonent.length());
        req->Write(par->abonent.c_str(), par->abonent.length());

        // service_Name
        req->WriteNetInt32(par->serviceName.length());
        req->Write(par->serviceName.c_str(), par->serviceName.length());

        // transportType
        req->WriteNetInt32(sizeof(par->transportType));
        req->WriteNetInt32(par->transportType);

        //maxLen45
        uint32_t bannerLen = par->maxLen;
        if (bannerLen == (uint32_t)-1)
            switch (par->transportType)
            {
                case TRANSPORT_TYPE_SMS : bannerLen = TRANSPORT_LEN_SMS;  break;
                case TRANSPORT_TYPE_USSD: bannerLen = TRANSPORT_LEN_USSD; break;
                case TRANSPORT_TYPE_HTTP: bannerLen = TRANSPORT_LEN_HTTP; break;
                case TRANSPORT_TYPE_MMS : bannerLen = TRANSPORT_LEN_MMS;  break;
            }

        req->WriteNetInt32(sizeof(bannerLen));
        req->WriteNetInt32(bannerLen);

        //charSet
        req->WriteNetInt32(sizeof(par->charSet));
        req->WriteNetInt32(par->charSet);

    } //cmndType != CMD_ERR_INFO

    return req_len + CMD_HEADER;//возвращаетс€ ќЅўјя длина пакета
}
//------------------------------------------------------------------------------
// анализ полученного от сервера пакета, считывание банераError
// возвращает  0 - если банер считан,  иначе - код ошибки
//
uint32_t AdvertisingImpl::AnaliseResponse(advertising_item *curAdvItem)
{

    uint32_t pak_len = curAdvItem->pakLen;
    smsc_log_debug(logger, "AdvertisingImpl::AnaliseResponse(), len = %d", pak_len);

    if (pak_len < 0)        // перебдеть - вообще-то это отлавливаетс€ снаружи
        return -pak_len;    // код ошибки (выставленный ранее)
    if (pak_len == 0)
        return  ERR_ADV_PACKET_LEN;     // неверна€ длина пакета

    //char* buf = curAdvItem->buf;
    char* buf = curAdvItem->isAsync ? curAdvItem->asyncBuf : curAdvItem->buf->GetCurPtr();
    BannerRequest* banReq =  curAdvItem->banReq;

    SerializationBuffer res;
    res.setExternalBuffer(buf, pak_len);
    res.setPos(0);

    uint32_t len;

    //command
    len = res.ReadNetInt32();             // command type
    if(len != CMD_BANNER_RSP)             // тип должен быть ответом на запрос баннера
        return ERR_ADV_PACKET_TYPE;       // некорректный тип пакета

    len = res.ReadNetInt32() + CMD_HEADER;      // Command length
    if (pak_len != len)
  return  ERR_ADV_PACKET_LEN;       // неверна€ длина пакета

    //TransactionId
    len = res.ReadNetInt32();             // Param length
    if (len != sizeof(int32_t))
        return ERR_ADV_PACKET_MEMBER;       // неверный элемент пакета

    uint32_t transID = res.ReadNetInt32();  // значение TransactID(оно же = hash key)

     // banner string
    len = res.ReadNetInt32();
    if (len == 0) banReq->banner.clear();   // баннер 0 длины
    else
    {
        if (banReq->maxLen != ((uint32_t)-1) &&
            banReq->maxLen <= len )
        {
            smsc_log_warn(logger, "AdvertisingImpl::getBanner(), banner length (%d)> maxLen (%d), transact_id = %d",
                           len, banReq->maxLen, transID);
            return ERR_ADV_BANNER_LEN;    // длина баннера большe maxLen
        }
        // вычитываем банер

        banReq->banner.reserve(len);
        banReq->banner.assign(buf + res.getPos(), len);
    }
    return 0;
}
//------------------------------------------------------------------------------

} // advert
} // scag
