package mobi.eyeline.smsquiz.subscription.datasource;

import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.subscription.Subscription;

import java.util.Date;


public interface SubscriptionDataSource {
    public void save(Subscription subscription) throws StorageException;
    public Subscription get(String address) throws StorageException;
    public ResultSet list(Date date) throws StorageException;
    void close();
}
