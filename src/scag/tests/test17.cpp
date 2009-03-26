#include "logger/Logger.h"
#include "scag/util/Drndm.h"

using namespace smsc::logger;
using namespace scag2::util;

enum {
    HISTOSIZE = 20
};

int main()
{
    Logger::Init();
    Logger* logger = Logger::getInstance("main");
    
    unsigned buf[HISTOSIZE];
    for ( int i = 0; i < HISTOSIZE; ++i ) buf[i] = 0;

    for ( int i = 0; i < 500; ++i ) {
        unsigned idx(Drndm::uniform( HISTOSIZE, Drndm::getRnd().getNextNumber()));
        ++buf[idx];
    }

    std::string histo;
    histo.reserve(HISTOSIZE*80);
    for ( int i = 0; i < HISTOSIZE; ++i ) {
        char msg[20];
        snprintf(msg,sizeof(msg),"\n%03u ",i);
        std::string histoData(buf[i],'*');
        histo += std::string(msg) + histoData;
    }
    smsc_log_info( logger, "A histogram follows:%s", histo.c_str() );

    return 0;
}
