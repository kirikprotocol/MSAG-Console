package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;
import mobi.eyeline.informer.admin.users.UsersManagerTest;
import mobi.eyeline.informer.util.Address;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
public class RestrictionsDaemonTest {

  private static Infosme infosme;
  private static File backupDir,usersConfigFile,restrictionConfigFile;
  private static TestDeliveryManager deliveryManager;
  private static TestUsersManager usersManager;
  private static TestRestrictionsManager restrictionsManager;
  private static RestrictionDaemon daemon;


  @BeforeClass
  public static void init() throws IOException {
    restrictionConfigFile = TestUtils.exportResourceToRandomFile(RestrictionsManager.class.getResourceAsStream("emptyrestrictions.csv"), ".restrictions");
    usersConfigFile = TestUtils.exportResourceToRandomFile(UsersManagerTest.class.getResourceAsStream("users.xml"), ".user");
    backupDir  = TestUtils.createRandomDir(".restrDaemon.backup");
  }



  @Before
  public void before() throws Exception {
    infosme = new TestInfosme();
    deliveryManager = new TestDeliveryManager(null);
    usersManager = new TestUsersManager(infosme, usersConfigFile, backupDir, FileSystem.getFSForSingleInst());
    for(User u : usersManager.getUsers()) {
      infosme.addUser(u.getLogin());
    }
    restrictionsManager = new TestRestrictionsManager(infosme, restrictionConfigFile, backupDir, FileSystem.getFSForSingleInst());
    daemon = new RestrictionDaemon(deliveryManager,restrictionsManager,usersManager);
  }


  @Test
  public void startStop() throws Exception {

    assertTrue(!daemon.isStarted());
    daemon.start();
    assertTrue(daemon.isStarted());
    daemon.stop();
    assertTrue(!daemon.isStarted());

    int aId = _createDelivery("a").getId();
    int bId = _createDelivery("b").getId();


    
  }







  @AfterClass
  public static void shutdown() {
    if(deliveryManager != null) {
      deliveryManager.shutdown();
    }
    if(usersConfigFile != null) {
      usersConfigFile.delete();
    }
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }
  }


  private Delivery _createDelivery(String userId) throws AdminException{
    Delivery d = Delivery.newCommonDelivery();
    d.setActivePeriodEnd(new Date());
    d.setActivePeriodStart(new Date(0));
    Calendar c = Calendar.getInstance();
    int weekDay = c.get(Calendar.DAY_OF_WEEK);
    Delivery.Day[] days = {
        Delivery.Day.Sun,
        Delivery.Day.Mon,
        Delivery.Day.Tue,
        Delivery.Day.Wed,
        Delivery.Day.Thu,
        Delivery.Day.Fri,
        Delivery.Day.Sat
    };
    Delivery.Day notToday = days[(weekDay+3)%7];
    d.setActiveWeekDays(new Delivery.Day[]{notToday});

    d.setStartDate(c.getTime());
    c.add(Calendar.DAY_OF_MONTH,1);
    d.setEndDate(c.getTime());

    d.setDeliveryMode(DeliveryMode.SMS);
    d.setName("Test delivery");
    d.setOwner(userId);
    d.setPriority(15);
    d.setReplaceMessage(true);
    d.setRetryOnFail(true);
    d.setRetryPolicy("policy1");

    d.setSvcType("svc1");
    d.setValidityDate(new Date());
    d.setValidityPeriod("1:00:00");
    d.setSourceAddress(new Address("+79123942341"));

    deliveryManager.createDelivery(userId,"1", d, new DataSource<Message>() {
      private LinkedList<Message> ms = new LinkedList<Message>() {
        {
          Message m1 = Message.newMessage("text1");
          m1.setAbonent(new Address("+79139489906"));
          add(m1);
          Message m2 = Message.newMessage("text2");
          m2.setAbonent(new Address("+79139489907"));
          add(m2);
        }
      };

      public Message next() throws AdminException {
        if(ms.isEmpty()) {
          return null;
        }
        return ms.removeFirst();
      }
    });
    return d;
  }

}