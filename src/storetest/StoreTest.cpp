#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <StoreManager.h>

int main(void) 
{
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
		StoreManager::init(config.getDatabase());
	} catch (ConfigException& exc) {
        printf("Exception : %s\n", exc.getMessage());
        return -1;
    }

	static char* oa = "123.456.7.890.123.456";
	static char* da = "098.7.654.321";
	static char* body = "Test message's body !";

	memset((void *)&sms, 0, sizeof(sms));

	sms.setState(ENROUTE);
	sms.setOriginatingAddress(strlen(oa), 1, 2, oa);
	sms.setDestinationAddress(strlen(da), 2, 1, da);
	sms.setWaitTime((time_t)1000L);
	sms.setValidTime((time_t)360000L);
	sms.setSubmitTime((time_t)0L);
	sms.setDeliveryTime((time_t)0L);
	sms.setMessageReference(5);
	sms.setMessageIdentifier(7);
	sms.setPriority(0);
	sms.setProtocolIdentifier(0);
	sms.setStatusReportRequested(true);
	sms.setRejectDuplicates(false);
	sms.setFailureCause(0);
	sms.setMessageBody(strlen(body), 1, false, (uint8_t *)body);
    
    const int NUM_OF_TEST_MESSAGES = 200000;

	try {
        store = StoreManager::getInstance();
        printf("Connect Ok !\n");
        
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
			sms = store->retrive((SMSId)i);
        }
		endTime = time(0L) - begTime;
        printf("Time spent for retriving: %d (sec)\nPerformance: %d (msg per sec)\n", 
			   endTime, NUM_OF_TEST_MESSAGES/endTime);*/

        SMSId id = store->store(sms);
		printf("Message stored, id = %d !\n", id);

		memset((void *)&sms, 0, sizeof(sms));

        sms = store->retrive(id);
		printf("Message retrived !\n", id);
    } 
    catch (StoreException& exc) {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    return 0;
}
