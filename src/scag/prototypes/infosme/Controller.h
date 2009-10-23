#ifndef _SCAG_PROTOTYPES_INFOSME_CONTROLLER_H
#define _SCAG_PROTOTYPES_INFOSME_CONTROLLER_H

#include <memory>
#include <vector>

#include "Processor.h"
#include "TaskDispatcher.h"
#include "logger/Logger.h"
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Controller : public util::WatchedThreadedTask
{
public:
    Controller( smsc::core::threads::ThreadPool& pool );

    ~Controller();

    void createTask( unsigned priority, unsigned speed, unsigned msgs );
    void setTaskActive( unsigned index, bool active );
    void addMessages( unsigned index, unsigned msgs );
    void addConnector( unsigned bandwidth );

    class Tokenizer {
    public:
        Tokenizer() : whitespaces_(" \t\f\v\r\n") {}
        std::vector< std::string > tokenize( const std::string& line );
        std::string toString( const std::vector< std::string >& words );
        std::vector< unsigned > getInts( unsigned n, const std::vector< std::string >& words );
        
    private:
        std::string whitespaces_;
    };

protected:
    virtual int doExecute();
    virtual const char* taskName() { return "control"; }

private:
    smsc::logger::Logger* log_;
    smsc::core::threads::ThreadPool& pool_;
    std::auto_ptr< Processor >  processor_;
    std::auto_ptr< Sender >     sender_;
    std::auto_ptr< TaskDispatcher > dispatcher_;
};

}
}
}

#endif
