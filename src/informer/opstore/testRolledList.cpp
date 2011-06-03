#include <vector>
#include "RolledList.h"
#include "core/synchronization/MutexReportContentionRealization.h"
#include "logger/Logger.h"
#include "informer/io/Drndm.h"
#include "informer/io/FileGuard.h"
#include "core/buffers/IntHash.hpp"
#include "core/threads/Thread.hpp"
#include "util/TimeSource.h"

using namespace eyeline::informer;
using smsc::core::synchronization::MutexGuard;


namespace {
const unsigned MAXLIST = 100;
const unsigned MAXNAME = 0x7fffffff;
}


// =========================================================
struct Message
{
    int numberOfLocks;
    // place some data here
    unsigned id;
    char*    name;
    int      status;
};


// =========================================================
struct Generator
{
    int   id;
    Drndm drndm;
    
    Generator( int idv ) : id(idv) {
        drndm.setSeed( reinterpret_cast<uint64_t>(reinterpret_cast<const void*>(this)) / 7 +
                       reinterpret_cast<uint64_t>(reinterpret_cast<const void*>(pthread_self())) / 11 );
    }

    unsigned gen( unsigned maxv ) {
        return unsigned(drndm.uniform(maxv,drndm.getNextNumber()));
    }
};


// =========================================================
class Storage
{
    typedef eyeline::informer::RolledList< Message > RollList;
    typedef RollList::iterator_type                  MsgIter;
public:

    Storage() :
        log_(smsc::logger::Logger::getInstance("store")),
        msgList_(), currentId_(0)
    {
        smsc_log_debug(log_,"ctor");
        fg_.create("store.log",0666,true,false);
        fg_.seek(0,SEEK_END);
    }


    ~Storage()
    {
        smsc_log_debug(log_,"dtor");
        RollList::container_type& c( msgList_.getContainer());
        msgHash_.Empty();
        while ( !c.empty() ) {
            free(c.front().name);
            c.pop_front();
        }
    }


    void createElement( unsigned id, unsigned newName )
    {
        RollList::ItemLock ml(msgList_);
        MsgIter iter;
        {
            MutexGuard mg(lock_);
            MsgIter* iptr = msgHash_.GetPtr(id);
            if (iptr) {
                // already exists, return
                smsc_log_warn(log_,"id=%u already found",id);
                return;
            }
            // not found, may be created
            iter = msgHash_.Insert(id,ml.createElement(Message()));
        }
        smsc_log_debug(log_,"created %p id=%u %x",&*iter,id,newName);
        iter->id = id;
        iter->name = makeName(newName);
        iter->status = 0;
        storeItem(iter);
    }


    bool deleteElement( unsigned id )
    {
        MsgIter iter;
        {
            MutexGuard mg(lock_);
            if ( !msgHash_.Pop(id,iter) ) {
                // smsc_log_debug(log_,"id=%u not found",id);
                return false;
            }
        }
        smsc_log_debug(log_,"deleting %p id=%u",&*iter,id);
        RollList::ItemLock ml(msgList_);
        RollList::container_type holder;
        if (!ml.pop(iter,holder)) {
            smsc_log_warn(log_,"id=%u cannot lock",id);
            return false;
        }
        iter->status = -1;
        storeItem(iter);
        free(iter->name);
        return true;
    }


    bool modifyElement( unsigned id, unsigned newName )
    {
        RollList::ItemLock ml(msgList_);
        MsgIter iter;
        {
            MutexGuard mg(lock_);
            MsgIter* iptr = msgHash_.GetPtr(id);
            if (!iptr) {
                // smsc_log_debug(log_,"id=%u not found",id);
                return false;
            }
            iter = *iptr;
            smsc_log_debug(log_,"modifying %p id=%u %x",&*iter,id,newName);
            if (!ml.lock(iter)) {
                smsc_log_warn(log_,"id=%u cannot lock",id);
                return false;
            }
        }
        ++iter->status;
        free(iter->name);
        iter->name = makeName(newName);
        storeItem(iter);
        return true;
    }


    void createMany( Generator& rnd, unsigned nelts )
    {
        smsc_log_debug(log_,"createMany %u",nelts);
        RollList::container_type holder;
        for ( unsigned i = 0; i < nelts; ++i ) {
            MsgIter iter = holder.insert(holder.end(),Message());
            iter->numberOfLocks = 0;
            iter->id = ++currentId_;
            iter->name = makeName(rnd.gen(MAXNAME));
        }
        RollList::StopRollLock srl(msgList_);
        for ( MsgIter iter = holder.begin(); iter != holder.end(); ++iter ) {
            storeItem(iter);
        }
        MutexGuard mg(lock_);
        for ( MsgIter iter = holder.begin(); iter != holder.end(); ++iter ) {
            if (msgHash_.GetPtr(iter->id)) {
                throw InfosmeException(EXC_LOGICERROR,"msg %d already there",iter->id);
            }
            msgHash_.Insert(iter->id,iter);
        }
        srl.insert(holder);
    }


    unsigned getCurrentId() const {
        return currentId_;
    }


    void storeAll()
    {
        smsc_log_debug(log_,"store all started");
        RollList::ItemLock ml(msgList_);
        for ( msgList_.startRolling(); msgList_.advanceRolling(ml); ) {
            MsgIter iter = ml.getIter();
            storeItem(iter);
        }
        smsc_log_debug(log_,"store all finished");
    }


    void rollOver()
    {
        smsc_log_debug(log_,"rolling over");
        rename("store.log", "store.log.1");
        FileGuard fg;
        fg.create("store.log");
        {
            MutexGuard mg(fileLock_);
            fg.swap(fg_);
        }
    }


    void wakeup()
    {
        MutexGuard mg(lock_);
        lock_.notifyAll();
    }


    void sleep( int msec ) {
        MutexGuard mg(lock_);
        lock_.wait(msec);
    }


private:
    void storeItem( MsgIter iter ) {
        smsc_log_debug(log_,"store %p id=%u st=%d name=%s",&*iter,iter->id,iter->status,iter->name);
        char buf[200];
        int w = sprintf(buf,"%d id=%u name=%s\n",iter->status,iter->id,iter->name);
        MutexGuard mg(fileLock_);
        fg_.write(buf,w);
    }

    char* makeName(unsigned name) {
        char buf[40];
        sprintf(buf,"0x%x",name);
        return strdup(buf);
    }

private:
    smsc::logger::Logger*                     log_;
    smsc::core::synchronization::EventMonitor lock_;
    smsc::core::buffers::IntHash<MsgIter>     msgHash_;

    RollList                                  msgList_;
    unsigned currentId_;

    smsc::core::synchronization::Mutex        fileLock_;
    FileGuard                                 fg_;
};


// =========================================================
class Worker : public smsc::core::threads::Thread
{
protected:
    Worker(int id, Storage& storage) :
    storage_(storage), gen_(id), stopping_(false), operations_(0) {}

public:
    ~Worker() {
        stop();
    }

    /*
    virtual int Execute()
    {
        while (!stopping_) {
            const unsigned op = gen_.gen(MAXOP);
            switch (op) {
            case 0:
                // add element
                storage_.createElement(gen_,MAXID,MAXNAME);
                break;
            case 1:
                // delete element
                storage_.deleteElement(gen_,MAXID);
                break;
            case 2:
                // modify element
                storage_.modifyElement(gen_,MAXID,MAXNAME);
                break;
            case 3:
                // create a number of elements
                storage_.createMany(gen_,MAXLIST,MAXID,MAXNAME);
                break;
            default:
                break;
            }
        }
    }
     */

    void stop() {
        stopping_ = true;
        storage_.wakeup();
        WaitFor();
    }

    unsigned operations() const {
        return operations_;
    }

protected:
    Storage&      storage_;
    Generator     gen_;
    volatile bool stopping_;
    unsigned      operations_;
};


class Processor : public Worker
{
public:
    Processor( int id, Storage& storage ) : Worker(id,storage) {}
    virtual int Execute()
    {
        while (!stopping_) {
            const unsigned op = gen_.gen(40);
            if ( op < 10 ) {
                // trying to recreate an element
                if ( !ids.empty() ) {
                    const unsigned id = ids.front();
                    ids.pop_front();
                    storage_.createElement(id,gen_.gen(MAXNAME));
                }
            } else if ( op < 20 ) {
                // trying to delete element
                const unsigned id = gen_.gen(storage_.getCurrentId()+1);
                if (storage_.deleteElement(id)) {
                    ids.push_back(id);
                }
            } else {
                // trying to modify element
                const unsigned id = gen_.gen(storage_.getCurrentId()+1);
                storage_.modifyElement(id,gen_.gen(MAXNAME));
            }
            ++operations_;
        }
        return 0;
    }
protected:
    std::list<unsigned> ids;
};
 


// =========================================================
class Input : public Worker
{
public:
    Input( int id, Storage& storage ) : Worker(id,storage) {}
    virtual int Execute()
    {
        while (!stopping_) {
            storage_.createMany(gen_,gen_.gen(MAXLIST));
            storage_.sleep(2000);
        }
        return 0;
    }
};


// =========================================================
class Roller : public Worker
{
public:
    Roller( int id, Storage& storage ) : Worker(id,storage) {}
    virtual int Execute()
    {
        while (!stopping_) {
            storage_.storeAll();
            storage_.sleep(3000);
            if (!stopping_) {
                storage_.rollOver();
            }
        }
        return 0;
    }
};


// =========================================================
int main( int argc, char** argv )
{
    // smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    smsc::logger::Logger::Init();

    const int nworkersmin = 5;
    int nworkers = nworkersmin;
    if ( argc > 1 ) {
        nworkers = atoi(argv[1]);
        if (nworkers < nworkersmin) nworkers = nworkersmin;
    }

    Storage storage;
    {
        Generator gen(0);
        storage.createMany(gen,100);
    }

    typedef std::vector< Worker* > WorkList;
    WorkList workers;
    workers.reserve(nworkers+2);
    {
        int id = 0;
        // workers.push_back(new Input(id++,storage));
        workers.push_back(new Roller(id++,storage));
        for ( int i = 0; i < nworkers; ++i ) {
            workers.push_back(new Processor(id++,storage));
        }
    }

    typedef smsc::util::TimeSourceSetup::HRTime HRTime;
    typedef HRTime::hrtime_type hrtime_type;
    const hrtime_type started = HRTime::getHRTime();

    for ( WorkList::iterator i = workers.begin(); i != workers.end(); ++i ) {
        (*i)->Start();
    }

    timespec ts = {100,0};
    nanosleep(&ts,0);

    for ( WorkList::iterator i = workers.begin(); i != workers.end(); ++i ) {
        (*i)->stop();
    }
    const hrtime_type stopped = HRTime::getHRTime();

    unsigned operations = 0;
    for ( WorkList::iterator i = workers.begin(); i != workers.end(); ++i ) {
        operations += (*i)->operations();
        delete *i;
    }
    workers.clear();

    printf("elapsed time: %umsec\n",unsigned((stopped-started)/(HRTime::ticksPerSec/1000)));
    printf("operations: %u\n",operations);
    return 0;
}
