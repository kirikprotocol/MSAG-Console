#include <stdio.h>
#include <stdlib.h>

#include <StoreManager.h>

int main(void) 
{
    using namespace smsc::store;
    
    MessageStore* store;
    SMS           sms;
	
	static char* oa = "123.456.7.890";
	static char* da = "098.7.654.321";
	static char* body = "Test message's body !";

	sms.setState(ENROUTE);
	sms.setOriginatingAddress(strlen(oa), 1, 2, (uint8_t *)oa);
	sms.setDestinationAddress(strlen(da), 2, 1, (uint8_t *)da);
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
    
    try {
        store = StoreManager::getInstance();
        printf("Connect Ok !\n");
        
		/*time_t begTime = time(0L);
        for (int i=0; i<10000; i++)
		{
			SMSId id = store->store(sms);
        }
		time_t endTime = time(0L);
        printf("Time spent: %d !\n", endTime - begTime);*/
		
		SMSId id = store->store(sms);
		printf("Message stored, id = %d !\n", id);

        //sms = store->retrive(id);
    } 
    catch (StoreException& exc) {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    return 0;
}
