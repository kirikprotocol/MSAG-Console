#include "Controller.h"
#include "Sender.h"
#include "Connector.h"
#include "ProtoException.h"

namespace scag2 {
namespace prototypes {
namespace infosme {


Controller::Controller( smsc::core::threads::ThreadPool& pool ) :
log_(smsc::logger::Logger::getInstance(taskName())),
pool_(pool)
{
    dispatcher_.reset( new TaskDispatcher() );
    sender_.reset( new Sender(*dispatcher_.get()) );
    processor_.reset( new Processor(*sender_.get(),*dispatcher_.get()) );
}


Controller::~Controller()
{
    processor_->stop();
    stop();
    checkDestroy();
}


void Controller::createTask( unsigned priority, unsigned speed, unsigned msgs )
{
    MutexGuard mg(releaseMon_);
    if ( processor_.get() ) {
        Task* task = new Task( priority, speed, sender_->connectorCount(), msgs );
        processor_->addTask( task );
    }
}


void Controller::setTaskActive( unsigned index, bool active )
{
    MutexGuard mg(releaseMon_);
    if ( processor_.get() ) {
        processor_->setTaskActive(index,active);
    }
}


void Controller::destroyTask( unsigned index )
{
    MutexGuard mg(releaseMon_);
    if ( processor_.get() ) {
        TaskGuard taskGuard = processor_->getTask(index);
        Task* task = taskGuard.get();
        if ( task ) {
            task->setDestroy();
            processor_->notify();
        }
    }
}


void Controller::addMessages( unsigned index, unsigned msgs )
{
    MutexGuard mg(releaseMon_);
    if ( processor_.get() ) {
        TaskGuard taskGuard = processor_->getTask(index);
        Task* task = taskGuard.get();
        if ( task ) {
            task->addMessages( sender_->connectorCount(), msgs );
            processor_->notify();
        }
    }
}


void Controller::addConnector( unsigned bandwidth )
{
    sender_->addConnector( new Connector(bandwidth ) );
}


int Controller::doExecute()
{
    smsc_log_info(log_,"started");

    addConnector(10);
    pool_.startTask(processor_.get(),false);

    while ( ! stopping() ) {

        {
            MutexGuard mg(releaseMon_);
            releaseMon_.wait(300);
        }

        /*
        const uint64_t r = util::Drndm::uniform(100);
        if ( r > 97 ) {
            int taskSpeed = int(util::Drndm::uniform(10));
            int taskPrio = int(util::Drndm::uniform(10));
            char taskname[100];
            sprintf(taskname,"%u",int(tasks_.size()));
            std::auto_ptr<Task> task(new Task(taskname,taskSpeed,taskPrio));
            processor_->addTask(task);
        }
         */
    }
    smsc_log_info(log_,"finishing");
    return 0;
}


std::vector< std::string > Controller::Tokenizer::tokenize( const std::string& line )
{
    std::vector< std::string > words;
    size_t pos = 0;
    while ( pos < line.size() ) {
        const size_t found = line.find_first_not_of(whitespaces_,pos);
        if ( found == std::string::npos ) break;
        pos = line.find_first_of(whitespaces_,found);
        if ( pos == std::string::npos ) { pos = line.size(); }
        words.push_back( line.substr(found,pos-found) );
    }
    return words;
}

std::string Controller::Tokenizer::toString( const std::vector<std::string>& words ) {
    size_t size = words.size();
    for ( std::vector<std::string>::const_iterator i = words.begin(); i != words.end(); ++i ) {
        size += i->size();
    }
    std::string line;
    line.reserve(size);
    for ( std::vector<std::string>::const_iterator i = words.begin(); i != words.end(); ++i ) {
        if ( !line.empty() ) line.push_back(' ');
        line.append(*i);
    }
    return line;
}


std::vector< unsigned > 
    Controller::Tokenizer::getInts( unsigned n,
                                    const std::vector< std::string >& words )
{
    if ( n+1 != words.size() ) throw ProtoException("cmd '%s': number of words mismatch: %u, required %u",
                                                    words[0].c_str(), unsigned(words.size()-1), n );
    std::vector< unsigned > res;
    res.reserve(n);
    for ( std::vector< std::string >::const_iterator i = words.begin()+1;
          i != words.end();
          ++i ) {
        char* endptr;
        unsigned long v = strtoul(i->c_str(),&endptr,0);
        if ( *endptr != '\0' ) {
            throw ProtoException("cmd '%s': not int: '%s'",words[0].c_str(),i->c_str());
        }
        res.push_back( unsigned(v) );
    }
    return res;
}

}
}
}
