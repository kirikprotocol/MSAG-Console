import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.AfterClass;
import storage.impl.DBSubscriptionDataSource;
import storage.ConnectionPoolFactory;
import storage.StorageException;
import storage.Subscription;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertNotNull;

import java.util.Date;


/**
 * Created by IntelliJ IDEA.
 * User: alkhal
 * Date: 30.09.2008
 * Time: 9:13:08
 * To change this template use File | Settings | File Templates.
 */
public class DBDataSourceTester {

    private static  DBSubscriptionDataSource dataSource;
    private String address ="+7913948990";

    @BeforeClass
    public static void initPool () {
        try {
            ConnectionPoolFactory.init("/home/alkhal/cvs/smsc/ext_smes/SmsQuiz/conf/config.xml");
            dataSource = new DBSubscriptionDataSource("/home/alkhal/cvs/smsc/ext_smes/SmsQuiz/conf/smsquiz.properties","");
        } catch (StorageException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @Test
    public  void addSubscription() {
            Subscription subscription = new Subscription();
            subscription.setAddress(address);
            subscription.setStartDate(new Date());
            subscription.setEndDate(null);
        try {
            dataSource.save(subscription);
        } catch (StorageException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
            assertTrue(false);
        }
    }
    @AfterClass
    public static  void close(){
           dataSource.close();
    }


    @Test
    public void getByAddress() {
       try {
           assertNotNull(dataSource.get(address));
       } catch (StorageException e) {
           e.printStackTrace();
           assertTrue(false);
       }
   }

  

}
