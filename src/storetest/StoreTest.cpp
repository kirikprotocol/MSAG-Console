#include <stdio.h>
#include <stdlib.h>

#include <StoreManager.h>

int main(void) 
{
    using namespace smsc::store;
    
    MessageStore* store;
    
    try {
        store = StoreManager::getInstance();
        printf("Connect Ok !\n");
    } 
    catch (exception& exc) {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    return 0;
}
