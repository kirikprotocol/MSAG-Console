#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <iconv.h>
#include "Managers.h"
#include "HttpCommand.h"
#include "HttpProcessor.h"
#include "HttpLogger.h"

#define ICONV_BLOCK_SIZE 32

#define LOG_HEADERS
//#define LOG_CONTENT
//#define LOG_TEXT
#define LOG_QUERY

//void printHex(const char *buffer, long length);

using namespace scag::transport::http;

namespace scag { namespace transport { namespace http 
{
    Logger *httpLogger = NULL;
    
    class MyHttpProcessor : public HttpProcessor
    {
    public:
        virtual void ReloadRoutes() {}

        virtual bool processRequest(HttpRequest& request) {        
#ifdef LOG_HEADERS        
            request.serialize();
            const std::string &hdr = request.getMessageHeaders();
            
            http_log_debug( "Request headers:\n%s", hdr.c_str() );
#endif

#ifdef LOG_QUERY
            HttpRequest::ParameterIterator& pi = request.getQueryParameterNames();
            
            while (pi.next()) {
                const std::string &value = request.getQueryParameter(pi.value());
                http_log_debug( "%s=%s", pi.value().c_str(), value.c_str() );
            }
#endif

#ifdef LOG_TEXT
            dumpText( request );
#endif                              
            return true;
        }
        virtual bool processResponse(HttpResponse& response) {
#ifdef LOG_HEADERS
            response.serialize();
            const std::string &hdr = response.getMessageHeaders();
            
            http_log_debug( "Response headers:\n%s", hdr.c_str() );
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

            return true;
        }
        virtual void statusResponse(HttpResponse& response,
                bool delivered = true)
        {
#ifdef LOG_HEADERS
            http_log_debug( "Response %s", delivered ? 
                "delivered" : "not delivered" );
#endif          
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
        http_log_debug( "%s", s.c_str() );          
    }
#endif    
    }    
    catch(Exception x) {
        http_log_debug( "getMessageText() exception %s", x.what() ); 
    }
}    
    
}}}


void http_mut_log(char *s, unsigned t, void *p) {
    http_log_debug(s, t, p);
}

HttpManagerConfig cfg = {
    5,  //int readerSockets;
    5,  //int writerSockets;
    2,  //int readerPoolSize;
    2,  //int writerPoolSize;
    2,  //int scagPoolSize;    
    100,  //int scagQueueLimit;
    20, //int connectionTimeout;
    //unsigned int maxHeaderLength;
    "0.0.0.0",  //const char *host;
    5000,       //int port;
};

int main() {
    HttpManager mg;
    MyHttpProcessor p;
    int k;
    sigset_t oset;
    sigset_t set;
        
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, &oset);

    Logger::Init();
    httpLogger = Logger::getInstance("scag.http");

    smsc_log_debug(httpLogger, "Started");
#if 0
    {
        Logger::LogLevels levels;
        levels["scag.http"] = Logger::LEVEL_FATAL;
        Logger::setLogLevels(levels);
    }
#endif    

    mg.init( p, cfg );
    smsc_log_debug(httpLogger, "Inited");

    k = 0;
#if 1
    do
         //k = getchar();
         sleep(10);
    while (!(k == 'q' || k == 'Q'));
#else    
    sleep(30);
#endif    

    mg.shutdown();
    Logger::Shutdown();
    
    pthread_sigmask(SIG_SETMASK, &oset, NULL);
}
