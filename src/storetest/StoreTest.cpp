#include <stdio.h>
#include <stdlib.h>

#include <StoreManager.h>

int main(void) 
{
    using namespace smsc::store;
    
    MessageStore* store;
    SMS           sms;
    
    try {
        store = StoreManager::getInstance();
        printf("Connect Ok !\n");
        SMSId id = store->store(&sms);
        printf("Message stored !\n");
        //store->retrive(id);
    } 
    catch (exception& exc) {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    return 0;
}
