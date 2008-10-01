package mobi.eyeline.smsquiz.manager;


import mobi.eyeline.smsquiz.storage.SubscriptionDataSource;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.storage.Subscription;
import mobi.eyeline.smsquiz.storage.impl.SubscriptionDataSourceImpl;
import org.apache.log4j.Logger;

import java.util.Date;


public class SubscriptionManager {

    private SubscriptionDataSource dataSource;
    private static Logger logger = Logger.getLogger(SubscriptionManager.class);
    private static SubscriptionManager manager;

    public static void init(String configFile) throws StorageException {
        manager = new SubscriptionManager(configFile);
    }
    public static SubscriptionManager getInstance() {
        return manager;
    }

    public boolean subscribe(String address) throws Exception{
        Subscription subscription = new Subscription();
        subscription.setAddress(address);
        dataSource.save(subscription);
        return true;
    }

    public boolean unsubscribe(String address) throws Exception{
       Subscription subscription = dataSource.get(address);
       if(subscription!=null) {
           if(subscription.getEndDate()!=null) {
                throw new Exception("Abonent wasn't subscribed");
           }
           subscription.setEndDate(new Date());
           dataSource.save(subscription);
           return true;
       }
       else {
            throw new Exception("Abonent wasn't subscribed");
       }
    }

    public boolean subscribed(String address) throws Exception{
        Subscription subscription = dataSource.get(address);
        return subscription != null && subscription.getEndDate() == null;
    }

    public void shutdown() {
        dataSource.close();
    }

    private SubscriptionManager(String configFile) throws StorageException {
        dataSource = SubscriptionDataSourceImpl.getInstance(configFile,"");
    }
}
