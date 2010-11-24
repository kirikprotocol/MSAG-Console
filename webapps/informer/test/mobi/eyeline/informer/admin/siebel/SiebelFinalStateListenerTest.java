package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.DeliveryMessageNotification;
import mobi.eyeline.informer.admin.delivery.DeliveryNotification;
import mobi.eyeline.informer.admin.delivery.DeliveryNotificationType;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelFinalStateListener;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import mobi.eyeline.informer.admin.siebel.impl.SiebelUserManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelFinalStateListenerTest {

  private static SiebelFinalStateListener listener;

  private static SiebelManager siebelManager;

  private static SiebelDeliveries deliveries;

  private static Map<String, SiebelMessage.DeliveryState> messagesState = new HashMap<String, SiebelMessage.DeliveryState>();
  private static Set<String> processedDeliveries = new HashSet<String>();

  private static File workDir = new File("siebel"+System.currentTimeMillis());

  @BeforeClass
  public static void before() throws Exception{

    final User siebelUser = new User();
    siebelUser.setAllRegionsAllowed(true);
    siebelUser.setLogin("siebel");
    siebelUser.setPassword("siebel");

    siebelUser.setStatus(User.Status.ENABLED);
    siebelUser.setLocale(new Locale("en"));
    siebelUser.setSourceAddr(new Address("+79130000000"));
    siebelUser.setDeliveryDays(new ArrayList<Integer>(){{for(int i=0;i<7;i++){add(i);}}});
    siebelUser.setDeliveryStartTime(new Time(8,0,0));
    siebelUser.setDeliveryEndTime(new Time(20,0,0));
    siebelUser.setDeliveryType(User.DeliveryType.SMS);

    final Delivery d = Delivery.newCommonDelivery();
    d.setProperty(UserDataConsts.SIEBEL_DELIVERY_ID, "123");

    siebelManager = new TestSiebelManager(deliveries = new SiebelDeliveries() {
      public void createDelivery(String login, String password, Delivery delivery, DataSource<Message> msDataSource) throws AdminException {}
      public void dropDelivery(String login, String password, int deliveryId) throws AdminException {}
      public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {}
      public Delivery getDelivery(String login, String password, int id) {
        return d;
      }
      public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {}
      public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {}
      public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {}
      public void activateDelivery(String login, String password, int deliveryId) throws AdminException {}
      public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<DeliveryInfo> visitor) throws AdminException {}
      public void getDefaultDelivery(String user, Delivery delivery) throws AdminException {}
    }, new SiebelRegionManager() {public Region getRegion(Address msisdn) throws AdminException {return null;}}) {
      @Override
      public void setDeliveryStatuses(Map<String, SiebelDelivery.Status> statuses) throws AdminException {
        SiebelFinalStateListenerTest.processedDeliveries.addAll(statuses.keySet());
      }
      @Override
      public void setMessageStates(Map<String, SiebelMessage.DeliveryState> deliveryStates) throws AdminException {
         messagesState.putAll(deliveryStates);
      }
    };

    listener = new SiebelFinalStateListener(siebelManager, siebelManager.deliveries, new SiebelUserManager() {
      public User getUser(String login) throws AdminException {
        return siebelUser;
      }
    }, workDir, 20);

  }

  @Test
  public void test() throws Exception{
    listener.onDeliveryFinishNotification(new DeliveryNotification(DeliveryNotificationType.DELIVERY_FINISHED, new Date(), 1, "siebel"));
    listener.onMessageNotification(
        new DeliveryMessageNotification(DeliveryNotificationType.MESSAGE_FINISHED,
            new Date(), 1, "siebel", 1234567l, MessageState.Failed, 1179, new Address("+79139489906"),
            DcpConverter.convertUserData(new HashMap<String, String>(1){{put(UserDataConsts.SIEBEL_MESSAGE_ID, "321");}}))
    );
    Thread.sleep(30000);
    assertTrue(processedDeliveries.contains("123"));
    assertTrue(messagesState.containsKey("321"));
    SiebelMessage.DeliveryState s = messagesState.get("321");
    assertEquals(s.getSmppCode(), "1179");
    assertEquals(s.getState(), SiebelMessage.State.ERROR);
  }



  @AfterClass
  public static void after() throws Exception{
    if(listener != null) {
      listener.shutdown();
    }
    if(siebelManager != null) {
      siebelManager.stop();
    }
    TestUtils.recursiveDeleteFolder(workDir);
  }
}
