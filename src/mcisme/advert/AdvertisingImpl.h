#ifndef SCAG_ADVERT_ADVERTISINGIMPL_CLIENT
#define SCAG_ADVERT_ADVERTISINGIMPL_CLIENT

#include "AdvertErrors.h"
#include "Advertising.h"
#include "AdvertisingTh.h"

#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
//#include "core/buffers/FastMTQueue.hpp"
#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "scag/util/singleton/Singleton.h"
#include "util/BufferSerialization.hpp"
#include "scag/exc/SCAGExceptions.h"


#include "util/debug.h"

using namespace smsc::core::buffers;
using namespace smsc::core::network;
using namespace smsc::core::threads;
using namespace smsc::util;

using namespace scag::util::singleton;
using namespace scag::exceptions;
using namespace smsc::logger;

// ���� �����������
enum 
{
    TRANSPORT_TYPE_SMS = 1,
    TRANSPORT_TYPE_USSD,
    TRANSPORT_TYPE_HTTP,
    TRANSPORT_TYPE_MMS
};
// ������������ ����� ������� �����������
enum 
{
    TRANSPORT_LEN_SMS  =  140,
    TRANSPORT_LEN_USSD =  160,
    TRANSPORT_LEN_HTTP = 1024,
    TRANSPORT_LEN_MMS  = 1024
};

#define GET_BANNER_REQ_LEN 40       // ����� ����� ������������� ���������� � getBannerReq
#define ERR_INFO_LEN 16             // ����� �����std::vector <char > buf; ������������� ���������� � errInfo

static int CommonTransactID = 0;    // ����� ������� ����������
static Mutex CommTransId_mtx;       // ������� ��� ������� � ��������


namespace scag {
namespace advert {

//------------------------------------------------------------------------------
// ����� �������������� ����������� ������
//
//class AdvertisingImpl;   
class AdvertisAsync : public Thread
{
    AdvertisingImpl* AdvertImpl;            // ��������� ���������� Advrtise
    FrontMTQueue <advertAsync_item*> *AsyncAdverts; // ��������� �� �������
    int timeout;
    
    int Execute();
    
public:
    bool stop;

    inline void Stop() { stop = true;
                         //WaitFor();
                         pthread_cancel(thread);
                        }    
     
    void Init( AdvertisingImpl* adv,        // ��������� ���������� Advrtise
               FrontMTQueue <advertAsync_item*> *asyncs,     
               int time               )     // ����� �������� 
    {
        AdvertImpl   = adv;
        AsyncAdverts = asyncs;
               
        timeout = time/2 + 11;              // msec
    }
    
//    void Notify() { AsyncAdverts->Notify();}
    
    AdvertisAsync() { stop=false; }

};

//-----------------------------------------------------------------------------------------
//   
class AdvertisingImpl : public Advertising
{
    
public:	
    
	 // ������ ������
    uint32_t getBanner( const std::string& abonent, 
                        const std::string& serviceName,
                        uint32_t transportType, uint32_t charSet, 
			std::string &banner, uint32_t maxLen = (uint32_t)-1);
    
    uint32_t getBanner(BannerRequest& req);
    void requestBanner(const BannerRequest& req);
      
    // �������������
    void init(const std::string& host, int port, int timeout, int maxcount);
    
    // �������� ����������� ������� �� �������������� 
    void CheckAsyncTimes();     
    
    // ������������ "�������" ����������� ��������
    int  ServAsyncAdvert(advertising_item *curAdvItem, int er=0);                              
    
    void Stop()// Implement shutdown !
    {
        adv_thread.Stop();
        adv_asyncs.Stop();
    } 

             AdvertisingImpl();
    virtual ~AdvertisingImpl();
    
protected:
        
    AdvertisingTh adv_thread; // ��������� ������ ��� ������ � ������� � ������� ������� �������
    AdvertisAsync adv_asyncs; // ��������� ������ ��� ������� ������� ������� �� ����������� ��������
    FrontMTQueue <advertAsync_item*> *AsyncAdverts;     // ������� ���������� �� ���������� �� ������� ������   
    FrontMTQueue <int> *AsyncKeys;                      // ������� ������ ����������� ��������   
        
    Logger    *logger;                                  // ��������� �� ����� �������
    uint32_t  timeout;
    int maxcount;             // ������������ ���������� ����������� �������� � �������
    
	// �������� ������������ ������������ �������
    void CheckBannerRequest(BannerRequest& banReq, int async = 0);  

    //  ���������� ������ ������������ �������
    uint32_t PrepareCmnd(SerializationBuffer* rec,      // �����
                         BannerRequest* par,            // ��������� ������� ������
                         uint32_t cmndType,             // ��� �������
                         uint32_t cmdInfo = 0);         // ��� ����������
	
	// ������ ����������� �� ������� ������
    uint32_t AdvertisingImpl::AnaliseResponse(advertising_item *curAdvItem);            
    // ������ � ������ ��������� �� ������
    void WriteErrorToLog(char* where, int errCode);                  
};


} // advert
} // scag

#endif //SCAG_ADVERT_ADVERTISINGIMPL_CLIENT

