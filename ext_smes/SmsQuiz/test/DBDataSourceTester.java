import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.AfterClass;
import org.junit.Ignore;
import storage.impl.SubscriptionDataSourceImpl;
import storage.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.util.Date;


public class DBDataSourceTester {

    private static SubscriptionDataSource dataSource;
    private String addressOff ="+791394899off";
    private String addressOn ="+7913948999on";



    @BeforeClass
    public static void initPool () {
        try {
            ConnectionPoolFactory.init("/home/alkhal/cvs/smsc/ext_smes/SmsQuiz/conf/config.xml");
            dataSource = SubscriptionDataSourceImpl.getInstance("/home/alkhal/cvs/smsc/ext_smes/SmsQuiz/conf/smsquiz.properties","");
        } catch (StorageException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }
   // @Ignore
    @Test
    public  void addSubscription() {
        try {
            Subscription subscription = new Subscription();
            subscription.setAddress(addressOn);
            subscription.setStartDate(new Date());
            subscription.setEndDate(null);
            dataSource.save(subscription);
            subscription.setAddress(addressOff);
            subscription.setStartDate(new Date());
            subscription.setEndDate(new Date());
            dataSource.save(subscription);
        } catch (StorageException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }
    @AfterClass
    public static  void close() {
           dataSource.close();
    }

  //  @Ignore
    @Test
    public void getByAddress() {
       try {
           assertNotNull(dataSource.get(addressOff));
           assertNotNull(dataSource.get(addressOn));
       } catch (StorageException e) {
           e.printStackTrace();
           assertTrue(false);
       }
   }
  //  @Ignore
    @Test
    public void subscribed() {
        try {
            assertFalse(dataSource.subscribed(addressOff));
            assertTrue(dataSource.subscribed(addressOn));
        } catch (StorageException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @Test
    public void list() {
        try {
            ResultSet result = dataSource.list(new Date());
            while(result.next()) {
                System.out.println(result.get());
            }
            result.close();
        } catch (StorageException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }   

}
