#include <iostream>

#include "logger/Logger.h"
#include "ProtoException.h"
#include "Controller.h"
#include "Processor.h"
#include "Sender.h"
#include "Task.h"
#include "Message.h"

using namespace scag2::prototypes::infosme;

int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* log = smsc::logger::Logger::getInstance("main");

    smsc::core::synchronization::EventMonitor sleepMon;

    smsc_log_info(log,"main started");
    smsc::core::threads::ThreadPool pool;
    {
        std::auto_ptr< Controller > controller( new Controller(pool) );
        smsc_log_info(log,"starting controller");
        pool.startTask( controller.get(), false );

        Controller::Tokenizer tok;
        while ( true ) {

            std::string line;
            std::getline(std::cin,line);
            if ( std::cin.eof() ) { break; }

            const std::vector< std::string > words(tok.tokenize(line));
            if ( words.empty() ) continue;
            smsc_log_info(log,"input line: %s", tok.toString(words).c_str());

            // processing
            try {
                std::vector< unsigned > ints;
                const std::string& action = words[0];
                if ( action == "conn" ) {
                    ints = tok.getInts(1,words);
                    controller->addConnector(ints[0]);
                } else if ( action == "create" ) {
                    ints = tok.getInts(3,words);
                    controller->createTask(ints[0],ints[1],ints[2]);
                } else if ( action == "disable" ) {
                    ints = tok.getInts(1,words);
                    controller->setTaskActive(ints[0],false);
                } else if ( action == "enable" ) {
                    ints = tok.getInts(1,words);
                    controller->setTaskActive(ints[0],true);
                } else if ( action == "destroy" ) {
                    ints= tok.getInts(1,words);
                    controller->destroyTask(ints[0]);
                } else if ( action == "addmsg" ) {
                    ints = tok.getInts(2,words);
                    controller->addMessages(ints[0],ints[1]);
                } else if ( action == "sleep" ) {
                    ints = tok.getInts(1,words);
                    MutexGuard mg(sleepMon);
                    sleepMon.wait(ints[0]);
                } else {
                    throw ProtoException("unknown command %s",action.c_str());
                }
            } catch ( std::exception& e ) {
                smsc_log_warn(log,"exc: %s", e.what());
            }
        } // while
    }
    smsc_log_info(log,"destroying the pool");
    pool.shutdown();
    return 0;
}
