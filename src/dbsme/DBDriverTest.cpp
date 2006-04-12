
#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSource.h>
#include <db/DataSourceLoader.h>

#include <core/threads/ThreadedTask.hpp>
#include <core/threads/ThreadPool.hpp>

#include <exception>
using std::exception;
using namespace smsc::core::threads;
using namespace smsc::db;

static smsc::logger::Logger *logger = 0;

static const char* SQL_1 = "INSERT INTO SMS_TEST (TEMP, STR, DT) VALUES (:TEMP, :STR, :DT)";
static const char* SQL_2 = "SELECT * FROM SMS_TEST";
static const char* SQL_3 = "UPDATE SMS_TEST SET TEMP=TEMP";

class Executor : public ThreadedTask
{
private:

    EventMonitor waitMonitor;
    DataSource* ds;
    int threadNum;
    bool threadIdle;

public:

    Executor(DataSource* _ds, int thn, bool thi) 
        : ThreadedTask(), ds(_ds), threadNum(thn), threadIdle(thi) {};

    virtual const char* taskName() { return "Executor"; };

    virtual void stop() 
    {
        ThreadedTask::stop();
        MutexGuard guard(waitMonitor);
        waitMonitor.notify();
    };

    int Execute() 
    {
        try
        {
            while (!isStopping) 
            {
                Connection* connection = ds->getConnection();
                if (!connection) {
                    smsc_log_warn(logger, "%d Get connection failed", threadNum);
                    return -1;
                }

                if (!threadIdle) 
                {
                    {
                        std::auto_ptr<Statement> s1guard(connection->createStatement(SQL_1));
                        Statement* statement1 = s1guard.get();

                        statement1->setInt64(1, 0xffffffffffffffffLL);
                        statement1->setString(2, "Testing string value");
                        statement1->setDateTime(3, 1000000);
                        statement1->executeUpdate();

                        connection->commit();
                    }
                    {
                        std::auto_ptr<Statement> s2guard(connection->createStatement(SQL_2));
                        Statement* statement2 = s2guard.get();
                        if (statement2) 
                        {
                            std::auto_ptr<ResultSet> rsguard(statement2->executeQuery());
                            ResultSet* rs = rsguard.get(); 

                            int count = 0;
                            while (rs && rs->fetchNext()) 
                            {
                                int64_t inum64 = rs->getInt64(1);
                                //smsc_log_debug(logger, "%d 64i : %llx value selected", threadNum, inum64);

                                if (!rs->isNull(2))
                                {
                                    const char* str = rs->getString(2);
                                    //smsc_log_debug(logger, "%d str %d: \"%s\" value selected", threadNum, ++count, str);
                                }
                            }
                        }
                    }
                }

                ds->freeConnection(connection);

                MutexGuard guard(waitMonitor);
                waitMonitor.wait(100);
            }
        }
        catch (std::exception& exc) {
            smsc_log_error(logger, "Executor %d error: %s", threadNum, exc.what());
        }
        return 0;
    };
};

const int MAX_DB_POOL_THREADS = 10;
int main(void) 
{
    using namespace smsc::db;

    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    const char* OCI_DS_FACTORY_IDENTITY = "OCI";
    
    smsc::logger::Logger::Init();
    logger = Logger::getInstance("DBSme.test");

    DataSourceLoader::loadupDataSourceFactory("../db/oci/libdb_oci.so", OCI_DS_FACTORY_IDENTITY);

    try 
    {
        Manager::init("config.xml");
        DataSource* ds = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
        if (ds)
        {
            ConfigView* config = new ConfigView(Manager::getInstance(), "DBSme.DataProviders.DataSource");
            ds->init(config);
            smsc_log_info(logger, "DS Init Ok, starting threads...");

            ThreadPool pool; pool.setMaxThreads(MAX_DB_POOL_THREADS);
            for (int i=0; i<MAX_DB_POOL_THREADS; i++) {
                pool.startTask(new Executor(ds, i, (bool)(i%2)));
            }

            Event stopEvent;
            stopEvent.Wait(300000);
            smsc_log_info(logger, "Stopping...");
            pool.shutdown();
            smsc_log_info(logger, "Stopped, deliting ds...");

            delete ds;
            if (config) delete config;
        }
    } 
    catch (Exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    smsc_log_info(logger, "All stopped ok.");
    return 0;
}
