#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>

#include "StoreManager.h"

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
        StoreManager::startup(Manager::getInstance());
        printf("Connect Ok !\n");
    } 
    catch (exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }

    static char* oa = "123.456.7.890.123.456";
    static char* da = "098.7.654.321";
    //static char* body = "";
    static char* body = "Test message's body !";

    memset((void *)&sms, 0, sizeof(sms));

    Descriptor dsc(4, "MSC1", 5, "IMSI1", 511);

    sms.setOriginatingAddress(strlen(oa), 1, 2, oa);
    sms.setOriginatingDescriptor(dsc);
    sms.setDestinationAddress(strlen(da), 2, 1, da);
    sms.setWaitTime((time_t)1000L);
    sms.setValidTime((time_t)360000L);
    sms.setSubmitTime((time_t)100L);
    sms.setMessageReference(5);
    sms.setPriority(1);
    sms.setProtocolIdentifier(1);
    sms.setDeliveryReport(24);
    sms.setArchivationRequested(true);
    sms.setEServiceType("GSM");
    sms.setMessageBody(strlen(body), 1, true, (uint8_t *)body);
    
    const int NUM_OF_TEST_MESSAGES = 10000;

    try 
    {
        store = StoreManager::getMessageStore();

        SMSId id = store->getNextId();
        store->createSms(sms, id);
        printf("Message was stored, id = %u !\n", id);

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
            //store->remove((SMSId)i);
        }
        endTime = time(0L) - begTime;
        printf("Time spent for retriving: %d (sec)\nPerformance: %d (msg per sec)\n", 
               endTime, NUM_OF_TEST_MESSAGES/endTime);*/

        /*store->remove(1);
        printf("Message removed !\n");*/

        /*SMSId id = store->store(sms);
        printf("Message stored, id = %d !\n", id);

        memset((void *)&sms, 0, sizeof(sms));

        store->retrive(id, sms);
        printf("Message retrived ! Body: %s\n", (char *)sms.messageBody.data);
        
        store->remove(id);
        printf("Message removed !\n");*/
    } 
    catch (StoreException& exc) {
        printf("Exception : %s\n", exc.what());
        result = -1;
    }
    
    StoreManager::shutdown();
    return result;
}
