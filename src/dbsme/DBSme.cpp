
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSourceLoader.h>
#include "CommandProcessor.h"
#include "sme/SmppBase.hpp"
#include "sms/sms.h"

#include <dbsme/jobs/SampleJob.h>
#include <dbsme/jobs/SQLJob.h>

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;

using namespace smsc::dbsme;

bool        bDBSmeIsStopped = false;

uint64_t    requestsProcessingCount = 0;
uint64_t    requestsProcessedCount = 0;
uint64_t    failuresNoticedCount = 0;
uint64_t    errorsHandledCount = 0;

Mutex       countersLock;

class DBSmeTask : public ThreadedTask
{
protected:

    SmppHeader*         pdu;
    CommandProcessor&   processor;
    SmppTransmitter&    transmitter;

public:
    
    DBSmeTask(SmppHeader* pdu, CommandProcessor& cp, SmppTransmitter& trans) 
        : ThreadedTask(), pdu(pdu), processor(cp), transmitter(trans) {};
    virtual ~DBSmeTask() {};

    virtual const char* taskName() 
    {
        return "DBSmeTask";
    };

    virtual int Execute()
    {
        if (pdu && pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
        {
            printf("\nReceived:%s\n",
                   ((PduXSm*)pdu)->get_message().get_shortMessage());
            {
                MutexGuard guard(countersLock);
                requestsProcessingCount++;
            }

            PduDeliverySmResp smResp;
            smResp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
            smResp.set_messageId("");
            smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            transmitter.sendDeliverySmResp(smResp);
        
            SMS sms;
            fetchSmsFromSmppPdu((PduXSm*)pdu, &sms);

            Command command;
            command.setFromAddress(sms.getOriginatingAddress());
            command.setToAddress(sms.getDestinationAddress());
            command.setJobName(0);
            command.setInData((const char *)sms.getMessageBody().getBuffer());

            try 
            {
                processor.process(command);
            }
            catch (Exception& exc)
            {
                {
                    MutexGuard guard(countersLock);
                    requestsProcessingCount--;
                    failuresNoticedCount++;
                }
                command.setOutData("Error processing SMS !");
                disposePdu(pdu);
                return -1;
            }

            disposePdu(pdu);
            sms.setDestinationAddress(command.getFromAddress());
            sms.setOriginatingAddress(command.getToAddress());
            sms.setArchivationRequested(false);
            sms.setDeliveryReport(0);
            sms.setValidTime(time(NULL)+3600);
            const char* out = command.getOutData();
            sms.getMessageBody().setBuffer((uint8_t *)out, strlen(out));

            PduSubmitSm sm;
            fillSmppPduFromSms(&sm,&sms);
            PduSubmitSmResp *dlResp=transmitter.submit(sm);

            printf((dlResp && dlResp->get_header().get_commandStatus()==0) ?
                   "Responce sent !\n" : "Responce wasn't send !\n");
            
            if (dlResp) disposePdu((SmppHeader*)dlResp);
            
            {
                MutexGuard guard(countersLock);
                requestsProcessingCount--;
                requestsProcessedCount++;
            }
        }
        else if (pdu && pdu->get_commandId() == SmppCommandSet::SUBMIT_SM_RESP)
        {
            printf("\nReceived async submit sm resp\n");
        }
        else
        {
            printf("\nReceived HZ\n");
        }
        
        if (pdu) disposePdu(pdu);
        return 0;
    };
};

class DBSmeTaskManager 
{
private:

    ThreadPool          pool;
    //Array<DBSmeTask *>    tasks;
    
public:

    DBSmeTaskManager() {};
    DBSmeTaskManager(ConfigView* config)
        throw(ConfigException) 
    {
        init(config);
    };
    virtual ~DBSmeTaskManager() 
    {
        pool.shutdown();
    };

    void init(ConfigView* config)
        throw(ConfigException)
    {
        // load up Pool arguments from Config
        try
        {
            int maxThreads = config->getInt("max");
            pool.setMaxThreads(maxThreads);
            printf("Max treads count: %d\n", maxThreads);
            int initThreads = config->getInt("init");   
            pool.preCreateThreads(initThreads);
            printf("Inited treads count: %d\n", initThreads);
        }
        catch (ConfigException) {}
    };
    
    void startTask(DBSmeTask* task)
    {
        //(void)tasks.Push(task);
        pool.startTask(task);
    };
};

class DBSmePduListener: public SmppPduEventListener
{
protected:
    
    CommandProcessor&   processor;
    DBSmeTaskManager&   manager;
    SmppTransmitter*    trans;

public:
    
    DBSmePduListener(CommandProcessor& proc, DBSmeTaskManager& man)
        : SmppPduEventListener(), processor(proc), manager(man) {};

    void handleEvent(SmppHeader *pdu)
    {
        manager.startTask(new DBSmeTask(pdu, processor, *trans));
    }
    
    void handleError(int errorCode)
    {
        {
            MutexGuard guard(countersLock);
            requestsProcessingCount--;
            errorsHandledCount++;
        }
        printf("Oops, Error handled! Code is: %d\n", errorCode);
    }
    
    void setTrans(SmppTransmitter *t)
    {
        trans=t;
    }
};

int main(void) 
{
    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    SampleJobFactory    _sampleJobFactory;
    SQLJobFactory       _sqlJobFactory;

    JobFactory::registerFactory(&_sampleJobFactory,
                                SMSC_DBSME_SAMPLE_JOB_IDENTITY);
    JobFactory::registerFactory(&_sqlJobFactory,
                                SMSC_DBSME_SQL_JOB_IDENTITY);
    
    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
        
        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);

        ConfigView cpConfig(manager, "Applications.DBSme");
        CommandProcessor processor(&cpConfig);

        ConfigView mnConfig(manager, "Applications.DBSme.ThreadPool");
        DBSmeTaskManager runner(&mnConfig);
        
        DBSmePduListener listener(processor, runner);
        
        SmeConfig cfg;
        // TO DO: Loadup these values from DBSme config
        cfg.host="smsc"; cfg.port=9001; cfg.sid="1";
        cfg.timeOut=10; cfg.password="";
        
        SmppSession session(cfg, &listener);
        session.connect();
        sleep(1);
        SmppTransmitter *tr = session.getSyncTransmitter();
        //SmppTransmitter *atr=ss.getAsyncTransmitter();
        listener.setTrans(tr);
        
        while (!bDBSmeIsStopped) 
        {
            sleep(2);
            printf("\nRequests: %llu processing, %llu processed.\n"
                   "Failures noticed: %llu\n"
                   "SMPP transport errors handled: %llu\n",
                   requestsProcessingCount, requestsProcessedCount,
                   failuresNoticedCount, errorsHandledCount);
        };
    }
    catch (Exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    return 0;
}
