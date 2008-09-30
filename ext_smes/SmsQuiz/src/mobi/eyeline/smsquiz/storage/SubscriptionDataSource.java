package mobi.eyeline.smsquiz.storage;

import java.util.Date;


public interface SubscriptionDataSource {
    public void save(Subscription subscription) throws StorageException;
    public Subscription get(String address) throws StorageException;
    public ResultSet list(Date date) throws StorageException;
    void close();
}
