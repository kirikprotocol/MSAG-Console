package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.TestAdminContext;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManagerTest;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.*;

import static org.junit.Assert.assertTrue;

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
    deliveryManager = new TestDeliveryManager(null, null);
    usersManager = new TestUsersManager(infosme, usersConfigFile, backupDir, FileSystem.getFSForSingleInst());
    for(User u : usersManager.getUsers()) {
      infosme.addUser(u.getLogin());
    }
    restrictionsManager = new TestRestrictionsManager(restrictionConfigFile, backupDir, FileSystem.getFSForSingleInst());
    daemon = new RestrictionDaemon(new TestRestrictionsDaemonContext(deliveryManager,restrictionsManager,usersManager), restrictionsManager);
  }


  @Test
  public void startStop() throws Exception {
    Thread.currentThread().setPriority(Thread.MIN_PRIORITY);

    assertTrue(!daemon.isStarted());
    daemon.start();
    long start;

    assertTrue(daemon.isStarted());
    daemon.stop();
    assertTrue(!daemon.isStarted());

    Delivery da = _createDelivery("a");
    Delivery db = _createDelivery("b");
    deliveryManager.activateDelivery("a","1",da.getId());
    deliveryManager.activateDelivery("b","1",db.getId());
    deliveryManager.forceActivatePlannedDeliveries();

    start = System.currentTimeMillis();
    Restriction ra  = createRestriction("a",new Date(start),new Date(start+1000));
    Restriction rb  = createRestriction("b",new Date(start+1000),new Date(start+2000));
    Restriction rab = createRestriction(null,new Date(start+3000),new Date(start+4000));


    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);

    daemon.start();

    waitT(500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    checkNextTaskDate(1000,start);
    waitT(1500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Paused,true,start);
    checkNextTaskDate(2000,start);
    waitT(2500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    checkNextTaskDate(3000,start);
    waitT(3500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkDeliveryState(db,DeliveryStatus.Paused,true,start);
    checkNextTaskDate(4000,start);
    waitT(4500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    checkNextTaskDate(Long.MAX_VALUE-start,start);

    //test restriction change
    start = System.currentTimeMillis();
    System.out.println("====new time base====");
    ra.setEndDate(new Date(start+2000));
    restrictionsManager.updateRestriction(ra);
    daemon.rebuildSchedule();
    waitT(1000,start);
    checkNextTaskDate(2000,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    waitT(3000,start);
    checkNextTaskDate(Long.MAX_VALUE-start,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);

   //
    start = System.currentTimeMillis();
    System.out.println("====new time base====");
    ra.setEndDate(new Date(start+5000));
    restrictionsManager.updateRestriction(ra);
    daemon.rebuildSchedule();
    waitT(500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    ra.setStartDate(new Date(start+3000));
    restrictionsManager.updateRestriction(ra);
    daemon.rebuildSchedule();
    waitT(1000,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    checkNextTaskDate(3000,start);
    waitT(3500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    checkNextTaskDate(5000,start);


    start = System.currentTimeMillis();
    System.out.println("====new time base====");
    ra.setStartDate(new Date(start+1000));
    ra.setEndDate(new Date(start+3000));
    rb.setStartDate(new Date(start+2000));
    rb.setEndDate(new Date(start+4000));
    restrictionsManager.updateRestriction(ra);
    restrictionsManager.updateRestriction(rb);
    daemon.rebuildSchedule();

    waitT(500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    checkNextTaskDate(1000,start);
    waitT(1500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);
    checkNextTaskDate(2000,start);
    waitT(2500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkDeliveryState(db,DeliveryStatus.Paused,true,start);
    checkNextTaskDate(3000,start);
    waitT(3500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Paused,true,start);
    checkNextTaskDate(4000,start);
    waitT(4500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkDeliveryState(db,DeliveryStatus.Active,false,start);


    start = System.currentTimeMillis();
    System.out.println("====new time base====");
    ra.setStartDate(new Date(start+1000));
    ra.setEndDate(new Date(start+2000));
    deliveryManager.pauseDelivery("a","1",da.getId());
    restrictionsManager.updateRestriction(ra);
    daemon.rebuildSchedule();
    waitT(500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,false,start);
    checkNextTaskDate(1000,start);
    waitT(1500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,false,start);
    checkNextTaskDate(2000,start);
    waitT(2500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,false,start);

    deliveryManager.activateDelivery("a","1",da.getId());    
    deliveryManager.forceActivatePlannedDeliveries();




    start = System.currentTimeMillis();
    System.out.println("====new time base====");
    ra.setStartDate(new Date(start+1000));
    ra.setEndDate(new Date(start+3000));
    restrictionsManager.deleteRestriction(rb.getId());
    restrictionsManager.deleteRestriction(rab.getId());
    restrictionsManager.updateRestriction(ra);
    daemon.rebuildSchedule();
    waitT(500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);
    checkNextTaskDate(1000,start);
    waitT(1500,start);
    checkDeliveryState(da,DeliveryStatus.Paused,true,start);
    checkNextTaskDate(3000,start);

    waitT(2000,start);
    restrictionsManager.deleteRestriction(ra.getId());
    daemon.rebuildSchedule();
    waitT(2500,start);
    checkDeliveryState(da,DeliveryStatus.Active,false,start);    






  }

  private void checkDeliveryState(final Delivery d, final DeliveryStatus requiredStatus, final boolean requiredRestricted , final long start) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(new String[]{d.getOwner()});
    deliveryManager.getDeliveries(d.getOwner(),"1",filter,100,new Visitor<Delivery>(){
      public boolean visit(Delivery di) throws AdminException {
        System.out.println((System.currentTimeMillis()-start)+" "+d.getOwner()+" status="+di.getStatus()+" "+Boolean.valueOf(di.getProperty(UserDataConsts.RESTRICTION)));
        assertTrue(di.getStatus()==requiredStatus);
        assertTrue(Boolean.valueOf(di.getProperty(UserDataConsts.RESTRICTION))==requiredRestricted);
        return true;
      }
    });

  }

  private Restriction createRestriction(String user, Date startDate, Date endDate) throws AdminException {
    Restriction r = new Restriction();
    r.setAllUsers(user==null);
    r.setStartDate(startDate);
    r.setEndDate(endDate);
    r.setName("restr_"+user);
    if(user!=null) {
      List<String> users = new ArrayList<String>();
      users.add(user);
      r.setUserIds(users);
    }
    restrictionsManager.addRestriction(r);
    return r;
  }





  @AfterClass
  public static void shutdown() {
    if(deliveryManager != null) {
      deliveryManager.shutdown();
    }
    if(usersConfigFile != null) {
      usersConfigFile.delete();
    }
    if(restrictionConfigFile!= null) {
      restrictionConfigFile.delete();
    }    
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }
  }


  private Delivery _createDelivery(String userId) throws AdminException{
    DeliveryPrototype d = new DeliveryPrototype();
    d.setActivePeriodEnd(new Time(22,0,0));
    d.setActivePeriodStart(new Time(1,0,0));
    Calendar c = Calendar.getInstance();
    int weekDay = c.get(Calendar.DAY_OF_WEEK);
    Day[] days = {
        Day.Sun,
        Day.Mon,
        Day.Tue,
        Day.Wed,
        Day.Thu,
        Day.Fri,
        Day.Sat
    };
    Day notToday = days[(weekDay+3)%7];
    d.setActiveWeekDays(new Day[]{notToday});

    d.setStartDate(c.getTime());
    c.add(Calendar.DAY_OF_MONTH,1);
    d.setEndDate(c.getTime());

    d.setDeliveryMode(DeliveryMode.SMS);
    d.setName("Test delivery");
    d.setOwner(userId);
    d.setPriority(15);
    d.setReplaceMessage(true);
    d.setRetryOnFail(true);
    d.setRetryPolicy("1h");

    d.setSvcType("svc1");
    d.setValidityPeriod(new Time(1,0,0));
    d.setSourceAddress(new Address("+79123942341"));

    Delivery delivery = deliveryManager.createDeliveryWithIndividualTexts(userId,"1", d, new DataSource<Message>() {
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
    return delivery;
  }

  private synchronized void waitT(long t, long startTime) throws InterruptedException, AdminException {
    long tw = t+(startTime-System.currentTimeMillis());
    if(tw<0) return;
    wait(tw);
    deliveryManager.forceActivatePlannedDeliveries();
  }

  void checkNextTaskDate(long isAt, long startDate) {
    long taskT = daemon.getTaskDate()-startDate;
    System.out.println("nextTask "+daemon.getTaskNum()+" -> "+  taskT);
    assertTrue(taskT == isAt);
  }

}