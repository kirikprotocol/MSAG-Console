package storage.impl;

import storage.SubscriptionDataSource;
import storage.StorageException;


public class SubscriptionDataSourceImpl {
    public static SubscriptionDataSource getInstance(String configFile, String prefix) throws StorageException {
        return new DBSubscriptionDataSource(configFile,prefix); 
    }
}
