package mobi.eyeline.smsquiz.subscription.storage.impl;

import mobi.eyeline.smsquiz.subscription.storage.SubscriptionDataSource;
import mobi.eyeline.smsquiz.subscription.storage.StorageException;


public class SubscriptionDataSourceImpl {
    public static SubscriptionDataSource getInstance() throws StorageException {
        return new DBSubscriptionDataSource(); 
    }
}
