package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class CdrDaemonTest {

  private static File workDir;

  private static File dir;

  private static CdrProviderContext context;

  private static int deliveryId;

  private static TestDeliveryManager manager;


  @BeforeClass
  public static void init() throws Exception{

    workDir = new File("cdr_"+System.currentTimeMillis());
    workDir.mkdirs();
    dir = new File(workDir, "cdrOut");
    dir.mkdirs();

    prepareRestoringFiles();

    Set<Long> s = new HashSet<Long>();
    String date = CdrDaemon.restore(FileSystem.getFSForSingleInst(), workDir,s);

    assertEquals(s.size(), 2);
    assertEquals(date, "20101112030410");

    final User user = new User();
    user.setLogin("a");
    user.setPassword("1");
    user.setCreateCDR(true);
    user.setCdrDestination("dest");
    user.setCdrOriginator("orig");

    final DeliveryPrototype p = new DeliveryPrototype();
    p.setOwner("a");
    p.setSourceAddress(new Address("+79139489906"));
    p.setActivePeriodStart(new Time(10,0,0));
    p.setActivePeriodEnd(new Time(22,0,0));
    p.setActiveWeekDays(Day.values());
    p.setDeliveryMode(DeliveryMode.SMS);
    p.setStartDate(new Date());
    p.setName("Delivery1");
    p.setPriority(10);


    manager = new TestDeliveryManager(null, null);

    context = new CdrProviderContext() {
      {
        deliveryId = manager.createDeliveryWithIndividualTexts("a","1", p, null).getId();
      }

      public Delivery getDelviery(String user, int deliveryId) throws AdminException {
        return manager.getDelivery("a","1", deliveryId);
      }

      public User getUser(String login) {
        return user;
      }

      public DeliveryChangesDetector getDeliveryChangesDetector() {
        return null;
      }
    };
  }


  private static void prepareRestoringFiles() throws Exception{
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new FileWriter(new File(workDir, "20101112030210.csv"))));
      writer.print("20101112030210");
      writer.print(',');
      writer.print("a");
      writer.print(',');
      writer.print("1");
      writer.print(',');
      writer.println("1");

      writer.print("20101112030410");
      writer.print(',');
      writer.print("a");
      writer.print(',');
      writer.print("1");
      writer.print(',');
      writer.println("2");

      writer.print("20101112030410");
      writer.print(',');
      writer.print("a");
      writer.print(',');
      writer.print("1");
      writer.print(',');
      writer.println("3");

    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");

  @Test
  public void test() throws Exception{
    CdrDaemon cdr = new CdrDaemon(workDir, dir, FileSystem.getFSForSingleInst(), context);
    ChangeMessageStateEvent e = new ChangeMessageStateEvent(
        sdf.parse("20101112030210"), deliveryId, "a", 1, MessageState.Delivered, 0, new Address("+79529223755"), 3, null
    );
    cdr.messageStateChanged(e);

    e = new ChangeMessageStateEvent(
        sdf.parse("20101112030410"), deliveryId, "a", 2, MessageState.Delivered, 0, new Address("+79529223755"), 3, null
    );
    cdr.messageStateChanged(e);

    e = new ChangeMessageStateEvent(
        sdf.parse("20101112030410"), deliveryId, "a", 3, MessageState.Delivered, 0, new Address("+79529223755"), 3, null
    );
    cdr.messageStateChanged(e);


    e = new ChangeMessageStateEvent(
        sdf.parse("20101112030410"), deliveryId, "a", 4, MessageState.Delivered, 0, new Address("+79529223755"), 3, null
    );
    cdr.messageStateChanged(e);


    Date now = new Date();
    e = new ChangeMessageStateEvent(
        now, deliveryId, "a", 5, MessageState.Delivered, 0, new Address("+79529223755"), 3, null
    );
    cdr.messageStateChanged(e);

    cdr.setFileCompletedInterval(1);
    cdr.roll();

    File[] fs = dir.listFiles();
    assertEquals(fs.length, 3);

    File f = new File(dir, "201011120304.csv");
    assertTrue(f.exists());
    int countLines = 0;
    BufferedReader reader = null;
    try{
      reader = new BufferedReader(new FileReader(f));
      while(reader.readLine() != null) {
        countLines++;
      }
    }finally {
      if(reader != null) {
        try{
          reader.close();
        }catch (IOException ex){}
      }
    }
    assertEquals(countLines, 1);

    cdr.setFileCompletedInterval(100000);
    e = new ChangeMessageStateEvent(
        new Date(), deliveryId, "a", 7, MessageState.Delivered, 0, new Address("+79529223755"), 3, null
    );
    cdr.messageStateChanged(e);
    cdr.roll();       
    fs = dir.listFiles();
    assertEquals(fs.length, 3);

    e = new ChangeMessageStateEvent(
        new Date(), deliveryId, "a", 8, MessageState.Delivered, 0, new Address("+79529223755"), 3, null
    );
    cdr.messageStateChanged(e);
    fs = dir.listFiles();
    assertEquals(fs.length, 3);

    fs = workDir.listFiles(new FileFilter() {
      public boolean accept(File pathname) {
        return pathname.isFile();
      }
    });
    assertEquals(fs.length, 1);

  }

  @AfterClass
  public static void shutdown() throws Exception {
    if(workDir != null) {
      TestUtils.recursiveDeleteFolder(workDir);
    }
    if(manager != null) {
      manager.shutdown();      
    }
  }
}
