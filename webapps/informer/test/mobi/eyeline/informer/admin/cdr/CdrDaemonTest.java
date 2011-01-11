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


  @BeforeClass
  public static void init() throws Exception {

    workDir = TestUtils.createRandomDir("_cdr");
    dir = TestUtils.createRandomDir("_cdrOut");

    prepareRestoringFiles();

    Set<Long> s = new HashSet<Long>();
    String date = CdrDaemon.restore(FileSystem.getFSForSingleInst(), workDir, s);

    assertEquals(s.size(), 2);
    assertEquals(date, "20101112030410");

    TestDeliveryManager manager = null;
    try {
      manager = new TestDeliveryManager(null, null);

      final User user = new User();
      user.setLogin("a");
      user.setPassword("1");
      user.setCreateCDR(true);
      user.setCdrDestination("dest");
      user.setCdrOriginator("orig");

      final DeliveryPrototype p = new DeliveryPrototype();
      p.setOwner("a");
      p.setSourceAddress(new Address("+79139489906"));
      p.setActivePeriodStart(new Time(10, 0, 0));
      p.setActivePeriodEnd(new Time(22, 0, 0));
      p.setActiveWeekDays(Day.values());
      p.setDeliveryMode(DeliveryMode.SMS);
      p.setStartDate(new Date());
      p.setName("Delivery1");
      p.setPriority(10);

      final Delivery delivery = manager.createDeliveryWithIndividualTexts("a", "1", p, null);
      deliveryId = delivery.getId();

      context = new CdrProviderContext() {
        public Delivery getDelivery(String user, int deliveryId) throws AdminException {
          return delivery;
        }

        public User getUser(String login) {
          return user;
        }

        public DeliveryChangesDetector getDeliveryChangesDetector() {
          return null;
        }
      };
    } finally {
      if (manager != null)
        manager.shutdown();
    }
  }

  @AfterClass
  public static void shutdown() throws Exception {
    if (workDir != null)
      TestUtils.recursiveDeleteFolder(workDir);
    if (dir != null)
      TestUtils.recursiveDeleteFolder(dir);
  }


  private static void prepareRestoringFiles() throws Exception {
    PrintWriter writer = null;
    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(new File(workDir, "20101112030210.csv"))));
      writer.println("20101112030210,a,1,1");
      writer.println("20101112030410,a,1,2");
      writer.println("20101112030410,a,1,3");
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
  }

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");

  private void fireMsgDeliveredEvent(CdrDaemon cdr, Date date, long msgId, String address) throws AdminException {
    cdr.messageStateChanged(new ChangeMessageStateEvent(
        date, deliveryId, "a", msgId, MessageState.Delivered, 0, new Address(address), 3, null
    ));
  }

  private static int countLines(File f) throws IOException {
    int countLines = 0;
    BufferedReader reader = null;
    try {
      reader = new BufferedReader(new FileReader(f));
      while (reader.readLine() != null)
        countLines++;
    } finally {
      if (reader != null) {
        try {
          reader.close();
        } catch (IOException ex) {
        }
      }
    }
    return countLines;
  }

  @Test
  public void test() throws Exception {
    CdrDaemon cdr = null;
    try {
      cdr = new CdrDaemon(workDir, dir, FileSystem.getFSForSingleInst(), context);

      fireMsgDeliveredEvent(cdr, sdf.parse("20101112030210"), 1, "+79529223755");
      fireMsgDeliveredEvent(cdr, sdf.parse("20101112030410"), 2, "+79529223755");
      fireMsgDeliveredEvent(cdr, sdf.parse("20101112030410"), 3, "+79529223755");
      fireMsgDeliveredEvent(cdr, sdf.parse("20101112030410"), 4, "+79529223755");
      fireMsgDeliveredEvent(cdr, new Date(), 5, "+79529223755");

      cdr.setFileCompletedInterval(1);
      cdr.roll();

      File[] fs = dir.listFiles();
      assertEquals(fs.length, 3);

      File f = new File(dir, "201011120304.csv");
      assertTrue(f.exists());
      assertEquals(countLines(f), 1);

      cdr.setFileCompletedInterval(100000);

      fireMsgDeliveredEvent(cdr, new Date(), 7, "+79529223755");

      cdr.roll();
      fs = dir.listFiles();
      assertEquals(fs.length, 3);

      fireMsgDeliveredEvent(cdr, new Date(), 8, "+79529223755");

      fs = dir.listFiles();
      assertEquals(fs.length, 3);

      fs = workDir.listFiles(new FileFilter() {
        public boolean accept(File pathname) {
          return pathname.isFile();
        }
      });
      assertEquals(fs.length, 1);
    } finally {
      if (cdr != null)
        cdr.stop();
    }
  }


}
