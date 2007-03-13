//------------------------------------------------------------------------------
//класс обслуживания "сокетного" транспорта 
//------------------------------------------------------------------------------
#ifndef SCAG_ADVERT_ADVERTISINGTH_CLIENT
#define SCAG_ADVERT_ADVERTISINGTH_CLIENT

#include <errno.h>

#include "Advertising.h"		//BannerRquest
#include "AdvertErrors.h"

#include "FrontMTQueue.hpp" // перенести в "core/buffers/FrontMTQueue.hpp" ?
#include "core/buffers/TmpBuf.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/exc/SCAGExceptions.h"
#include "scag/util/singleton/Singleton.h"
#include "util/BufferSerialization.hpp"
#include "logger/Logger.h"

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace smsc::core::network;
using namespace smsc::core::threads;
using namespace smsc::util;

using namespace scag::util::singleton;
using namespace scag::exceptions;
using namespace smsc::logger;

// коды команд 
enum 
{
     CMD_BANNER_REQ = 1,    // запрос от клиента к серверу
     CMD_BANNER_RSP,        // ответ  от сервера
     CMD_ERR_INFO = 0x100	// пакет от клиента к серверу с информацией об ошибке
};

#define BANNER_LEN 1024                     // max len баннера
#define MAX_PACKET_LEN 1048                 // максимальная длина всего пакета
#define CMD_HEADER   sizeof(uint32_t)*2     // cmd type + cmd length


#define MAX_WAIT_TIME 120000// (~2 мин) максимальное промежуток между восстановлением соединения
#define MIN_WAIT_TIME 500

namespace scag {
namespace advert {

class AdvertisingImpl;    
//-----------------------------------------------------------------------------------------
//
struct advertising_item
{
        uint32_t TransactID;            // номер транзакции
        EventMonitor eventMon;          // 
        int pakLen;						// длина полученного пакета
        bool isAsync;
        BannerRequest* banReq;	
        union
        {
            TmpBuf<char, BANNER_LEN> *buf;  // адрес буфера принимающего/ожидающего потока
            char * asyncBuf;                // указатель на будущий буфер
        };
        //char buf[MAX_PACKET_LEN];    // буфер принимающего/ожидающего потока - насколько все было бы проще!
        
    	advertising_item(BannerRequest* _banReq, TmpBuf<char, BANNER_LEN> *tmpbuf) 
        {
        	banReq 	= _banReq;
            buf     = tmpbuf;
        	pakLen  = 0;
            isAsync = false;
        	TransactID = banReq->getId() & 0xffffffff;// там 64 бита!
        }

 };

//-----------------------------------------------------------------------------------------
// расширение для асинхронных вызовов
struct advertAsync_item:advertising_item

{
        timestruc_t tv;

        advertAsync_item(BannerRequest* _banReq, int timeout)  
                      :advertising_item(_banReq, NULL) 
        {
            isAsync = true;
        //    setLastTime(timeout);
        }  

        void setLastTime(int timeout)
        {
            if (clock_gettime(CLOCK_REALTIME, &tv) == 0)
            {
                // далее фрагмент из Event
                tv.tv_sec+=timeout/1000;
                tv.tv_nsec+=(timeout%1000)*1000000L;
                if(tv.tv_nsec>1000000000L)
                {
                  tv.tv_sec++;
                  tv.tv_nsec-=1000000000L;
                }
            }            
            else tv.tv_sec = 0;// выводим на ошибку тайм-аута
        }
       
        ~advertAsync_item()
        {
            if (asyncBuf) 
                delete asyncBuf;
        }
       
};

//-----------------------------------------------------------------------------------------
//
class AdvertisingTh : public Thread
{
        
  protected:
                   
        Socket sock;
       	Mutex sock_mtx;             // для доступа к сокету
        Mutex hash_mtx;             // для доступа к хешу (insert/delete)
	
		int wait_timeout;           // таймаут восстановления соединения    
		uint32_t srv_timeout;       // таймаут ответа от сервера
		std::string srv_host;
		int srv_port;
	
        Event sock_wait;
        EventMonitor* asyncMonitor;// указатель на монитор потока асинхронных вызовов
		bool stop_th;
    
        
        int Execute();
        int ReadAll(char* buf,int size);
        void ClearAsyncs();
        
public:
        Logger    *logger;
		IntHash <advertising_item*> advPtrs;            // hash указателей 
        
        FrontMTQueue <advertAsync_item*> AsyncAdverts;	// очередь указателей на полученные от сервера ответы   
        FrontMTQueue <int> AsyncKeys;                   // очередь ключей асинхронных запросов   
        //Mutex AsyncItemDeleteMtx;       // мьютех, используемый при удалении из памяти асинхронных элементов

        void Init(const std::string& host, int port, int timeout);
        
        advertising_item* GetAdvertItem(int key);
        void RemoveAdvert(int advId);
        void RemoveAdvert(advertising_item* advItem);
        void AddAdvert(advertising_item* advItem);
        advertising_item* AdvertisingTh::GetNextHashItem(int i);

        int  ServBanner(advertising_item* advItem, SerializationBuffer* req, uint32_t req_len);
        void Stop();
        int  Write(SerializationBuffer* req, uint32_t rec_len);
        
         AdvertisingTh() { stop_th = false;}
		~AdvertisingTh() { Stop(); }
};

/**/
} // adverTh
} // scag

#endif //SCAG_ADVERT_ADVERTISINGIMPL_CLIENT

