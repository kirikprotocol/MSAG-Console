
#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSource.h>
#include <db/DataSourceLoader.h>

int main(void) 
{
    using namespace smsc::db;

    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    const char* OCI_DS_FACTORY_IDENTITY = "OCI";
    
    const char* sql1 = "INSERT INTO SMS_TEST (TEMP, STR) VALUES (:TEMP, :STR)";
    const char* sql2 = "SELECT * FROM SMS_TEST";
    
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
                    "Applications.DBSme.DataProviders.1111111.DataSource");
            
            ds->init(config);
            printf("Init Ok !\n");
            Connection* connection = ds->getConnection();
            if (connection)
            {
                Statement* statement1 = connection->createStatement(sql1);
                if (statement1)
                {
                    for (int i=0; i<10; i++)
                    {
                        statement1->setInt32(1, i*10);
                        statement1->setString(2, "Testing string value");
                        int rows = statement1->executeUpdate();
                    }
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
                            int8_t inum8 = rs->getInt8(1);
                            printf("8i  : %d value selected\n", inum8);
                            int16_t inum16 = rs->getInt16(1);
                            printf("16i : %d value selected\n", inum16);
                            int32_t inum32 = rs->getInt32(1);
                            printf("32i : %d value selected\n", inum32);

                            uint8_t unum8 = rs->getUint8(1);
                            printf("8u  : %d value selected\n", unum8);
                            uint16_t unum16 = rs->getUint16(1);
                            printf("16u : %d value selected\n", unum16);
                            uint32_t unum32 = rs->getUint32(1);
                            printf("32u : %d value selected\n", unum32);

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
