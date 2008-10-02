package mobi.eyeline.smsquiz.storage.impl;

import mobi.eyeline.smsquiz.storage.SubscriptionDataSource;
import mobi.eyeline.smsquiz.storage.StorageException;


public class SubscriptionDataSourceImpl {
    public static SubscriptionDataSource getInstance() throws StorageException {
        return new DBSubscriptionDataSource(); 
    }
}
