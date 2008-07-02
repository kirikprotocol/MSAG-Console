#include "logger/Logger.h"
#include "testlogger1.h"

struct Test
{
    Test( const std::string& s ) : data(s) {}
    const std::string& getKey() const {
        return data;
    }
private:
    std::string data;
};


int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* log = smsc::logger::Logger::getInstance("test");
    smsc_log_info( log, "hello, world" );

    unsigned int i = 10;

    Testlogger< Test > tt;
    const Test ttt( "hello, world" );
    tt.prepare( ttt );
    tt.show( i );

    smsc::logger::Logger::Shutdown();
    return 0;
}
