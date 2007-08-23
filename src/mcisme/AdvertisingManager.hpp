//------------------------------------
//  AdvertisingManager.hpp
//  Routman Michael, 2006-2007
//------------------------------------


/*#ifndef ___ADVERTISINGMANAGER_H
#define ___ADVERTISINGMANAGER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <logger/Logger.h>

#include <core/buffers/DiskHash.hpp>

#include <mcisme/AbntAddr.hpp>
#include <mcisme/Profiler.h>
#include <mcisme/ProfilesStorage.hpp>
#include <core/buffers/File.hpp>
#include <admin/util/SocketListener.h>
#include "list"

namespace smsc { namespace mcisme
{

using smsc::core::network::Socket;
using smsc::core::threads::ThreadedTask;
using smsc::admin::util::SocketListener;
using std::list;

class AdvertRun : public Thread
{
    counter* Counter;           // cчетчик
    Advertising* Advert;        // экземпляр реализации Advrtise
    int runMode;				// режим выполнения
    TestBannerDispatcher dispatch;
    
    protected:
	int Execute()
	{
            if (!Advert)
            {
                puts("не определена реализация Advert!");
                return 1;
            }
            while(stop == false)    
            {
            	int abnt_indx, rc;
            	string banner;
                                
                int curmode = runMode;
            	if (curmode < 0 || curmode >= RANDOM_MODE)
            		curmode =  Counter->all % RANDOM_MODE;
                
                switch(curmode)
            	{ 
            		case SYNC_MODE_0:
            			{
			                rc= Advert->getBanner(  Params.GetAbonent(abnt_indx),
			                                        Params.GetService(),  
			                                        Params.GetTransportType(),
			                                        UTF16BE, banner);
			    
			                BannerRequest tmpBR("", "", 0, 0); 
			                tmpBR.banner = banner;
			                AnalizeRezult(tmpBR, rc);
			                break;
            			}   
            		case SYNC_MODE_1:
            			{
			                BannerRequest tmpBR(Params.GetAbonent(abnt_indx),
                                                Params.GetService(),
			                					Params.GetTransportType(), UTF16BE); 
			                
			                rc= Advert->getBanner(tmpBR);
			                AnalizeRezult(tmpBR, rc);
			                break;
            			}   
            		case  ASYNC_MODE:
            			{
                            BannerRequest* tmpBR = new  BannerRequest
                                                       (Params.GetAbonent(abnt_indx),
                                                        Params.GetService(),
                                                        Params.GetTransportType(), 
                                                        UTF16BE, -1, (BannerDispatcher*)&dispatch);
                            AsyncsCountGuard.Add(tmpBR);
                            Advert->requestBanner(*tmpBR);
                            
			                break;
            			}  
            	}
                Params.FreeUseAbonent(abnt_indx);
	        }	
            return 0;
        }

public:
     bool stop;
    inline void Stop() { stop = true;
                         //WaitFor();
                         pthread_cancel(thread);
                        }    
    
    // засылка параметров
    void Init(Advertising* adv, counter* cnt, int mode = SYNC_MODE_1)
    { 
        Advert  = adv;  // экземпляр реализации Advrtise
        Counter = cnt;  // cчетчик
        runMode = mode; // режим вызова 
    }
    //------------------------------------------------------------------------------
	// обработчик результатов выполнения     //заглушка
    void AnalizeRezult(const BannerRequest &banReq,  uint32_t rc)
	{
	    bool empty = true;
		   
	    switch (rc)
	    {
	        case 0                  :   
	                                    empty = banReq.banner.empty();
	                                    /** /
	                                    if (!empty)
	                                    {
	                                        string text;
	                                        Convertor::convert( "UTF16BE", "UTF8",
	                                                     		banReq.banner.c_str(),
	                                                     		banReq.banner.length(), text);
	                                        printf("%s\n", text.c_str());
	                                    }   
	                                break;
	                                
	        case ERR_ADV_TIMEOUT    : //puts("getBanner was timed out"); 
			break;
	        case ERR_ADV_SOCK_WRITE :// puts("error on writing to socket");  
			break;     
	        case ERR_ADV_SOCKET     : puts("getBanner was disconnected from server"); break;
	        case ERR_ADV_PACKET_MEMBER: 
	        case ERR_ADV_PACKET_TYPE: 
	        case ERR_ADV_PACKET_LEN : printf("error on packet data: %d\n", rc);       break;
	        case ERR_ADV_QUEUE_FULL : //puts("adverts queue overloaded");  
			break;
	        default                 : printf("getBanner error: %d\n", rc);
	    }
        if (stop) return;
        
        if (Counter)
            Counter->Increment(rc, empty);
                
        BannerRequest* banPtr = (BannerRequest*) &banReq;
        if (banPtr &&
            banPtr->dispatcher)
        {
            AsyncsCountGuard.Delete(banPtr);
            delete banPtr;                                        
        }   
    }           
    
    AdvertRun()
    {
        Counter = NULL;
        Advert  = NULL;
        dispatch.adv = this; // диспетчер у каждого потока свой - вызывает каждый свой счетчик
        
        stop=false;
    }

};


class AdvertisingManager
{
	typedef list<Socket*>	SocketsList;
	
	SocketsList	connectedSockets;
	SocketsList	disconnectedSockets;
	EventMonitor	connSockMon;
	EventMonitor	disconnSockMon;

	string		advertServer;
	int			advertPort;


};









};	//  namespace msisme
};	//  namespace smsc
#endif
//

	//class AdvertConnection
	//{
	//	Socket	sock;
	//	bool isConnected;
	//public:
	//	AdvertConnection();
	//	~AdvertConnection();
	//	int connect(void)
	//	{

	//	}
	//	void disconnect(void)
	//	{
	//		sock.Close();
	//	}
	//	void getBanner(void);

	//};

//	AdvertConnection**	conn;

/*
const int GET_BANNER_REQ	= 0x01;
const int GET_BANNER_RSP	= 0x02;
const int ERR_INFO			= 0x100;

struct AdvertHeader
{
	uint32_t	cmdLen;
	uint32_t	cmdType;
};

struct AdvertBannerReq
{
	uint32_t	transactionId;
	string		abonent;				//	MSISDN абонента в формате .TON.NPI.VALUE например .1.1.78432909209 строка передается в ASCII кодировке	6-20
	string		serviceId;				//	Строковый идентификатор сервиса строка передается в ASCII кодировке	1-255
	uint32_t	transportType;			//	Тип транспорта
	uint32_t	maxLen;					//	Максимальная длина баннера
	uint32_t	charSet;				//	Тип кодировки
	
	int	getPacket(char* buff, size_t buff_len)
	{
		return 0;
	}
};

struct AdvertBannerRsp
{
	uint32_t	transactionId;			//	Сквозной идентификатор транзакции, генерируется клиентом	4
	string		banner;					//	Закодированный баннер	0-1024

	int	setPacket(char* buff)
	{
		return 0;
	}
};

struct AdvertErrInfo
{
	uint32_t	transactionId;			//	Сквозной идентификатор транзакции, генерируется клиентом
	uint32_t	errorCode;				//	Коды ошибок (1 – ответ от сервера не получен за определенный таймаут )

	int	getPacket(char* buff, size_t buff_len)
	{
		return 0;
	}
};

class AdvertisingManager: public Thread
{
	typedef list<Socket*>	SocketsList;
	
	SocketsList	connectedSockets;
	SocketsList	disconnectedSockets;
	EventMonitor	connSockMon;
	EventMonitor	disconnSockMon;

	string		advertServer;
	int			advertPort;
	int			connCount;

	bool		bStarted, bNeedExit;
	smsc::logger::Logger*   logger;

public:
	AdvertisingManager(const string& _serv, int _port, int _connCount):
	  advertServer(_serv), advertPort(_port), connCount(_connCount),
	  logger(Logger::getInstance("mci.Advert")){}
	~AdvertisingManager(){}
	
	void getBanner();

	virtual int Execute();
	virtual void Start();
	virtual void Stop();
};
*/
