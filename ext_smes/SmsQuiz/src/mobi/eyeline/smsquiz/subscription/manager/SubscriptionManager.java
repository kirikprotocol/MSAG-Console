package mobi.eyeline.smsquiz.subscription.manager;


import mobi.eyeline.smsquiz.subscription.datasource.SubscriptionDataSource;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.subscription.Subscription;
import mobi.eyeline.smsquiz.subscription.datasource.impl.DBSubscriptionDataSource;
import org.apache.log4j.Logger;

import java.util.Date;


public class SubscriptionManager {

    private SubscriptionDataSource dataSource;
    private static Logger logger = Logger.getLogger(SubscriptionManager.class);
    private static SubscriptionManager manager;

    public static void init() throws SubManagerException {
        manager = new SubscriptionManager();
    }
    public static SubscriptionManager getInstance() {
        return manager;
    }

    public void subscribe(String address) throws SubManagerException {
        Subscription subscription = new Subscription();
        subscription.setAddress(address);
        try {
            dataSource.save(subscription);
        } catch (StorageException e) {
            logger.error("Unable to save subscription",e);
            throw new SubManagerException("Unable to save subscription",e);
        }
    }

    public void unsubscribe(String address) throws SubManagerException {
       try{
           Subscription subscription = dataSource.get(address);
           if(subscription!=null) {
               if(subscription.getEndDate()!=null) {
                    throw new SubManagerException("Abonent wasn't subscribed");
               }
               subscription.setEndDate(new Date());
               dataSource.save(subscription);
           }
           else {
                throw new SubManagerException("Abonent wasn't subscribed");
           }
       }
       catch (StorageException preExc) {
           logger.error("Error during unsubscription address: "+address,preExc);
           throw new SubManagerException("Error during unsubscription address: "+address,preExc);
       }
    }

    public boolean subscribed(String address) throws SubManagerException {
        Subscription subscription = null;
        try {
            subscription = dataSource.get(address);
            return subscription != null && subscription.getEndDate() == null;
        } catch (StorageException e) {
            logger.error("Error getting subscription info for address: "+address, e);
            throw new SubManagerException("Error getting subscription info for address: "+address,e);
        }
    }

    public void shutdown() {
        dataSource.close();
    }

    private SubscriptionManager() throws SubManagerException {
        try {
            dataSource = new DBSubscriptionDataSource();
        } catch (StorageException e) {
            logger.error("Error init manager",e);
            throw new SubManagerException("Error init manager",e);
        }
    }
}
