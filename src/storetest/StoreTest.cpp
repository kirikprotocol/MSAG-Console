#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <StoreManager.h>

int main(void) 
{
    int result = 0;

    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    using namespace smsc::store;
    using namespace smsc::sms;
    
    MessageStore* store;
    SMS           sms;
    
    try 
    {
        Manager::init("config.xml");
        Manager& config = Manager::getInstance();
        
        unsigned poolSize = (unsigned)config.getInt("db.connections.max");
        unsigned initSize = (unsigned)config.getInt("db.connections.init");
        const char* dbInstance = config.getString("db.instance");
        const char* dbUserName = config.getString("db.user");
        const char* dbUserPassword = config.getString("db.password");
        
        store = StoreManager::startup(dbInstance, dbUserName, 
                                      dbUserPassword, poolSize, initSize);
        printf("Connect Ok !\n");
    } 
    catch (exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }

    //static char* oa = "123.456.7.890.123.456";
    static char* oa = "";
    static char* da = "098.7.654.321";
    static char* body = "Test message's body !";

    memset((void *)&sms, 0, sizeof(sms));

    sms.setState(ENROUTE);
    sms.setOriginatingAddress(strlen(oa), 1, 2, oa);
    sms.setDestinationAddress(strlen(da), 2, 1, da);
    sms.setWaitTime((time_t)1000L);
    sms.setValidTime((time_t)360000L);
    sms.setSubmitTime((time_t)100L);
    sms.setDeliveryTime((time_t)100L);
    sms.setMessageReference(5);
    sms.setMessageIdentifier(7);
    sms.setPriority(1);
    sms.setProtocolIdentifier(1);
    sms.setStatusReportRequested(true);
    sms.setRejectDuplicates(true);
    sms.setFailureCause(5);
    sms.setMessageBody(strlen(body), 1, true, (uint8_t *)body);
    
    const int NUM_OF_TEST_MESSAGES = 10000;

    try 
    {
        /*time_t begTime, endTime;
        printf("\nStoring %d messages, please wait ... \n", 
                NUM_OF_TEST_MESSAGES);
        begTime = time(0L);
        for (int i=0; i<NUM_OF_TEST_MESSAGES; i++)
        {
            SMSId id = store->store(sms);
        }
        endTime = time(0L) - begTime;
        printf("Time spent for storing: %d (sec)\nPerformance: %d (msg per sec)\n",
                endTime, NUM_OF_TEST_MESSAGES/endTime);
        
        begTime = time(0L);
        printf("\nRetriving %d messages, please wait ... \n",
               NUM_OF_TEST_MESSAGES);
        for (int i=1; i<NUM_OF_TEST_MESSAGES+1; i++)
        {
            store->retrive((SMSId)i, sms);
            store->remove((SMSId)i);
        }
        endTime = time(0L) - begTime;
        printf("Time spent for retriving: %d (sec)\nPerformance: %d (msg per sec)\n", 
               endTime, NUM_OF_TEST_MESSAGES/endTime);*/

        SMSId id = store->store(sms);
        printf("Message stored, id = %d !\n", id);

        memset((void *)&sms, 0, sizeof(sms));

        store->retrive(id, sms);
        printf("Message retrived ! Body: %s\n", (char *)sms.messageBody.data);
        
        /*store->remove(id);
        printf("Message removed !\n", id);*/
    } 
    catch (StoreException& exc) {
        printf("Exception : %s\n", exc.what());
        result = -1;
    }
    
    StoreManager::shutdown();
    return result;
}
