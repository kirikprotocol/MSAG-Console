
#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSource.h>
#include <db/DataSourceLoader.h>

#include <exception>
using std::exception;

int main(void) 
{
    using namespace smsc::db;

    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    const char* OCI_DS_FACTORY_IDENTITY = "OCI";
    
    const char* sql1 = "INSERT INTO SMS_TEST (TEMP, STR, DT)"
                        " VALUES (:TEMP, :STR, :DT)";
    const char* sql2 = "SELECT * FROM SMS_TEST";
    const char* sql3 = "UPDATE SMS_TEST SET TEMP=TEMP";
    
    DataSourceLoader::loadupDataSourceFactory(
        "../db/oci/libdb_oci.so", OCI_DS_FACTORY_IDENTITY);

    try 
    {
        Manager::init("config.xml");
        DataSource* ds = 
            DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
        if (ds)
        {
            ConfigView* config = 
                new ConfigView(Manager::getInstance(),
                    "DBSme.DataProviders.SampleDataProvider1.DataSource");
            
            ds->init(config);
            printf("Init Ok !\n");
            Connection* connection = ds->getConnection();
            if (connection)
            {
                Statement* statement1 = connection->createStatement(sql1);
                if (statement1)
                {
                    statement1->setInt64(1, 0xffffffffffffffffLL);
                    statement1->setString(2, "Testing string value");
                    statement1->setDateTime(3, 1000000);
                    statement1->executeUpdate();
                     
                    /*for (int i=0; i<10; i++)
                    {
                        statement1->setInt64(1, -(0x0123456789000000+i));
                        statement1->setString(2, "Testing string value");
                        statement1->setDateTime(3, i*1000000);
                        int rows = statement1->executeUpdate();
                    }*/
                    
                    connection->commit();
                    delete statement1;
                }
                
                Statement* statement2 = connection->createStatement(sql2);
                if (statement2)
                {
                    ResultSet* rs = statement2->executeQuery();
                    if (rs)
                    {
                        int count = 0;
                        while (rs->fetchNext())
                        {
                            /*int8_t inum8 = rs->getInt8(1);
                            printf("8i  : %d value selected\n", inum8);
                            int16_t inum16 = rs->getInt16(1);
                            printf("16i : %d value selected\n", inum16);
                            int32_t inum32 = rs->getInt32(1);
                            printf("32i : %d value selected\n", inum32);*/
                            int64_t inum64 = rs->getInt64(1);
                            printf("64i : %llx value selected\n", inum64);

                            /*uint8_t unum8 = rs->getUint8(1);
                            printf("8u  : %d value selected\n", unum8);
                            uint16_t unum16 = rs->getUint16(1);
                            printf("16u : %d value selected\n", unum16);
                            uint32_t unum32 = rs->getUint32(1);
                            printf("32u : %d value selected\n", unum32);*/
                            /*uint64_t unum64 = rs->getUint64(1);
                            printf("64u : %llx value selected\n", unum64);*/

                            if (!rs->isNull(2))
                            {
                                const char* str = rs->getString(2);
                                printf("str %d: \"%s\" value selected\n", 
                                       ++count, str);
                            }
                        }
                        delete rs;
                    }
                    delete statement2;
                }
                
                /*Statement* statement3 = connection->createStatement(sql3);
                if (statement3)
                {
                    int wdTimer = ds->startTimer(connection, 10);
                    int rows = statement3->executeUpdate();
                    ds->stopTimer(wdTimer);
                    delete statement3;
                }*/

                ds->freeConnection(connection);
            }
            delete ds;
            if (config) delete config;
        }
    } 
    catch (Exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    return 0;
}
