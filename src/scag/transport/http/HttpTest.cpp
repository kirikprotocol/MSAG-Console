#include <xercesc/dom/DOM.hpp>
#include "scag/config/base/ConfigView.h"
#include "scag/config/base/Config.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <iconv.h>
#include "Managers.h"
#include "HttpCommand.h"
#include "HttpProcessor.h"

#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"
#include "util/debug.h"
#include "util/xml/DOMTreeReader.h"

#include <util/findConfigFile.h>

#define ICONV_BLOCK_SIZE 32

//#define LOG_HEADERS
//#define LOG_CONTENT
//#define LOG_TEXT
//#define LOG_QUERY

//void printHex(const char *buffer, long length);

using namespace scag::transport::http;
using namespace smsc::logger;
using namespace smsc::core::synchronization;
using namespace scag::config;
using namespace xercesc;
using namespace smsc::util::xml;
using namespace smsc::util;

smsc::logger::Logger *logger;
Mutex mtx;
uint32_t req_cnt = 0, resp_cnt = 0, st_resp_cnt = 0;

namespace scag { namespace transport { namespace http 
{
    //Logger *httpLogger = NULL;
    
    class MyHttpProcessor : public HttpProcessor
    {
    public:
        virtual int processRequest(HttpRequest& request) {        
#ifdef LOG_HEADERS        
            request.serialize();
            const std::string &hdr = request.getMessageHeaders();
            
            smsc_log_debug( logger,  "Request headers:\n%s", hdr.c_str() );
#endif

#ifdef LOG_QUERY
            HttpRequest::ParameterIterator& pi = request.getQueryParameterNames();
            
            while (pi.next()) {
                const std::string &value = request.getQueryParameter(pi.value());
                smsc_log_debug( logger,  "%s=%s", pi.value().c_str(), value.c_str() );
            }
#endif

#ifdef LOG_TEXT
            dumpText( request );
#endif                              
            {
                MutexGuard mt(mtx);
                req_cnt++;
            }

            return 1;
        }
        virtual int processResponse(HttpResponse& response) {
#ifdef LOG_HEADERS
            response.serialize();
            const std::string &hdr = response.getMessageHeaders();
            
            smsc_log_debug( logger,  "Response headers:\n%s", hdr.c_str() );
#endif
#ifdef LOG_TEXT
            dumpText( response );
#endif
#ifdef LOG_CONTENT
            {
                unsigned int length;
                char *data = response.getMessageContent(length);
                
                printHex(data, length > 50 ? 50 : length);
            }
#endif
            {
                MutexGuard mt(mtx);
                resp_cnt++;
            }

            return 1;
        }

        virtual int statusResponse(HttpResponse& response, bool delivered = true)
        {
#ifdef LOG_HEADERS
            smsc_log_debug( logger,  "Response %s", delivered ? 
                "delivered" : "not delivered" );
#endif          
            {
                MutexGuard mt(mtx);
                st_resp_cnt++;
            }
            return 1;

        }
    virtual void ReloadRoutes()
    {
        // Do nothing
    }
        
        virtual ~MyHttpProcessor() {
        }
        
        MyHttpProcessor(): content(2048) {
        }
        
        void dumpText(HttpCommand &cmd);
        
        TmpBuf<char, 2048> content;
    };

void MyHttpProcessor::dumpText(HttpCommand &cmd)
{
    std::string charset( "windows-1251" );

    try {
#if 0       
        const wstring &text = cmd.getMessageText();
        
        printHex((char *)text.data(), text.size());
        
        if (!text.empty())
    {
        size_t result = 0;
        size_t outbytesleft;    
        size_t inbytesleft = text.size() * sizeof(wchar_t);
        char *outbufptr;
        const char *inbufptr = (char *)text.data();
        iconv_t cd = iconv_open(charset.c_str(), "wchar_t");

        if (cd == (iconv_t)(-1))
            throw Exception("iconv_open() failed");

        content.SetPos(0);
        while (inbytesleft) {
            content.setSize(content.GetPos() + ICONV_BLOCK_SIZE);
            outbufptr = content.GetCurPtr();
            outbytesleft = ICONV_BLOCK_SIZE;        

            result = iconv(cd, &inbufptr, &inbytesleft, &outbufptr, &outbytesleft);
            if (result == (size_t)(-1) && errno != E2BIG)
                break;      
                
            content.SetPos(content.GetPos() + ICONV_BLOCK_SIZE - outbytesleft);
        }

        iconv_close(cd);

        if (result == (size_t)(-1))
            throw Exception("iconv() failed");
            
        std::string s( content.GetCurPtr() - content.GetPos(),
                            content.GetPos() );                 
        smsc_log_debug( logger,  "%s", s.c_str() );          
    }
#endif    
    }    
    catch(Exception x) {
        smsc_log_debug( logger,  "getMessageText() exception %s", x.what() ); 
    }
}    
    
}}}


void http_mut_log(char *s, unsigned t, void *p) {
    smsc_log_debug( logger, s, t, p);
}



HttpManagerConfig HttpManCfg(
    10,  //int readerSockets;
    10,  //int writerSockets;
    2,  //int readerPoolSize;
    2,  //int writerPoolSize;
    1,  //int scagPoolSize;    
    10,  //int scagQueueLimit;
    10, //int connectionTimeout;
    //unsigned int maxHeaderLength;
    "0.0.0.0",  //const char *host;
    5001       //int port;
);

//void load_config(const ConfigView & cv)
void load_config(ConfigView & cv)
{
 
        try {

        std::auto_ptr<char> host_( cv.getString("host") );
        HttpManCfg.readerSockets =cv.getInt("readerSockets");;
        HttpManCfg.writerSockets =cv.getInt("writerSockets");;
        HttpManCfg.readerPoolSize=cv.getInt("readerPoolSize");;
        HttpManCfg.writerPoolSize=cv.getInt("writerPoolSize");;
        HttpManCfg.scagPoolSize  =cv.getInt("scagPoolSize");;
        HttpManCfg.scagQueueLimit=cv.getInt("scagQueueLimit");;
        HttpManCfg.connectionTimeout=cv.getInt("connectionTimeout");;
        HttpManCfg.host = host_.get();
        HttpManCfg.port = cv.getInt("port");
   

        printf("%s:%d\n",HttpManCfg.host.c_str(),HttpManCfg.port);
        printf("readerSockets %d\n",HttpManCfg.readerSockets);
        printf("writerSockets %d\n",HttpManCfg.writerSockets);
        printf("readerPoolSize %d\n",HttpManCfg.readerPoolSize);
        printf("writerPoolSize %d\n",HttpManCfg.writerPoolSize);
        printf("scagPoolSize %d\n",HttpManCfg.scagPoolSize);
        printf("scagQueueLimit %d\n",HttpManCfg.scagQueueLimit);
        printf("connectionTimeout %d\n",HttpManCfg.connectionTimeout);


        fflush(stdout);
    }
    catch(ConfigException& e)
    {
        smsc_log_error(logger,"%s",e.what());
    }
    catch(...)
    {
        smsc_log_debug(logger," Unknown exception.");
    }
    
}

int configure() 
{
  initXerces();
  
  try
  {
    Config config;
    __trace__("reading config...");
    DOMTreeReader reader;

    const char* cfgFile=smsc::util::findConfigFile("config.xml");
    char * filename = new char[strlen(cfgFile) + 1];
    std::strcpy(filename, cfgFile);
    std::auto_ptr<char>config_filename(filename);

    DOMDocument *document = reader.read(config_filename.get());
    if (document && document->getDocumentElement())
    {
      DOMElement *elem = document->getDocumentElement();
      __trace__("config readed");
      config.parse(*elem);
      __trace2__("parsed %u ints, %u booleans, %u strings\n",
                 config.intParams.GetCount(),
                 config.boolParams.GetCount(),
                 config.strParams.GetCount());
    } else {
      smsc_log_debug(logger,"Parse result is null");
      return 0;
    }

     load_config(ConfigView(config, "HttpTransport"));

  } catch (ParseException &e) {
      smsc_log_debug(logger,"%s",e.what());
      return 0;
  }catch(ConfigException& e){
      smsc_log_debug(logger,"%s",e.what());
      return 0;
  }catch(Exception &e) {
      smsc_log_debug(logger,"%s",e.what());
      return 0;
  }catch(...) {
      smsc_log_debug(logger,"exception, unknown exception");
      return 0;
  }
  return 1;
}




int main() 
{
    MyHttpProcessor p;
    int k;
    sigset_t oset;
    sigset_t set;
    uint32_t last_req_cnt = 0, last_resp_cnt = 0, last_st_resp_cnt = 0;
    time_t t;
        
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, &oset);

    Logger::Init();
    logger = Logger::getInstance("scag.http");
    if(!configure())
    {
        printf("error in config file!");
        return 0;
    }

//#if 1
//    {
//        Logger::LogLevels levels;
//        levels["scag.http"] = Logger::LEVEL_FATAL;
//        Logger::setLogLevels(levels);
//    }
//#endif    

    try {
    scag::transport::http::HttpManager::Init(p, HttpManCfg);
    }
    catch(Exception x) {
    smsc_log_error( logger,  "Cannot init the HTTP transport: %s", x.what());    
    }
    HttpManager& mg = scag::transport::http::HttpManager::Instance();

    k = 0;
#if 1
    printf("\n");
    t = time(NULL);
    do{
         //k = getchar();
         sleep(1);
         {
            MutexGuard mt(mtx);
            int d = time(NULL) - t;
            printf("\rTime: %d. Total: %d, %d, %d. Average: %d, %d, %d. Last second: %d, %d, %d\t", d, req_cnt, resp_cnt, st_resp_cnt, req_cnt/d, resp_cnt/d, st_resp_cnt/d, req_cnt - last_req_cnt, resp_cnt - last_resp_cnt, st_resp_cnt - last_st_resp_cnt);
            last_req_cnt = req_cnt;
            last_resp_cnt = resp_cnt;
            last_st_resp_cnt = st_resp_cnt;
         }
         fflush(stdout);
   }
    while (!(k == 'q' || k == 'Q'));
#else    
    sleep(60);
#endif    

    mg.shutdown();
    Logger::Shutdown();
    
    pthread_sigmask(SIG_SETMASK, &oset, NULL);
}
