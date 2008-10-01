import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.manager.SubscriptionManager;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.AfterClass;

/**
 * author: alkhal
 */
public class SubscriptionManagerTest {

    private static SubscriptionManager manager;
    private String address="+79139489906";
    @BeforeClass
    public static void init() {
        try {
            ConnectionPoolFactory.init("conf/config.xml");
        } catch (StorageException e) {
            e.printStackTrace();
            assertTrue(false);
        }
        try {
            SubscriptionManager.init("conf/smsquiz.properties");
            manager = SubscriptionManager.getInstance();
        } catch (StorageException e) {
            e.printStackTrace();
        }
    }

    @Test
    public void subscribe() {
        try {
            manager.subscribe(address);
        } catch (Exception e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }
    @Test
    public void subscribeb() {
        try {
            assertTrue(manager.subscribed(address));
        } catch (Exception e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }
    @Test
    public void unsubscribe() {
        try {
            manager.unsubscribe(address);
        } catch (Exception e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }
    @Test
    public void subscribedAfterUnsubsr() {
        try {
            assertFalse(manager.subscribed(address));
        } catch (Exception e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }
    @Test(expected = Exception.class)
    public void unsubscribeAfterUnsubscr() throws Exception{
        manager.unsubscribe(address);
    }
    @AfterClass
    public static void shutdown() {
        manager.shutdown();
    }
}
