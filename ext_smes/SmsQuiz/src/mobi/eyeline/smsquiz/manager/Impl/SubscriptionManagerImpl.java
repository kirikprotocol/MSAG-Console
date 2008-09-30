package mobi.eyeline.smsquiz.manager.Impl;

import mobi.eyeline.smsquiz.manager.SubscriptionManager;
import mobi.eyeline.smsquiz.storage.SubscriptionDataSource;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.storage.Subscription;
import mobi.eyeline.smsquiz.storage.impl.SubscriptionDataSourceImpl;
import org.apache.log4j.Logger;


public class SubscriptionManagerImpl implements SubscriptionManager {

    private SubscriptionDataSource dataSource;
    private static Logger logger = Logger.getLogger(SubscriptionManagerImpl.class);
    private static SubscriptionManager manager;

    public static void init(String configFile) throws StorageException {
        manager = new SubscriptionManagerImpl(configFile);
    }
    public static SubscriptionManager getInstance() {
        return manager;
    }

    public boolean subscribe(String address) {
        Subscription subscription = new Subscription();
        subscription.setAddress(address);
        try {
            dataSource.save(subscription);
            return true;
        } catch (StorageException e) {
            logger.error("Error during storing a subscription",e);
            return false;
        }
    }

    public boolean unsubscribe(String address) {
        Subscription subscription = new Subscription();
        subscription.setAddress(address);
        try {
            //dataSource.get();
            return true;
        } catch (Exception e) {
            logger.error("Error during storing a subscription",e);
            return false;
        }
    }

    public boolean subscribed(String address) {
        return false;
    }
    public void shutdown() {

    }

    private SubscriptionManagerImpl(String configFile) throws StorageException {
        dataSource = SubscriptionDataSourceImpl.getInstance(configFile,"");
    }
}
