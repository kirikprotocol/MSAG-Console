#include <stdio.h>
#include <stdlib.h>

#include <OCIMessageStore.h>

int main(void) 
{
    using namespace smsc::store;
    using namespace smsc::store::oci;

    OCIMessageStoreFactory factory;
    OCIStoreConfig* config = new OCIStoreConfig("ORCL", "smsc", "smsc");
    
    MessageStore* store = factory.createMessageStore(config);
    
    try {
        store->open();
	printf("Connect Ok !\n");
    } 
    catch (exception& ae) {
	printf("Exception : %s\n", ae.what());
    }
    
    if (store) delete store;
    if (config) delete config;
    
    return 0;
}
