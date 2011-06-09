package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Calendar;
import java.util.LinkedList;
import java.util.List;
import java.util.TimeZone;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class UserStatProviderTest {

  static File baseDir;

  @BeforeClass
  public static void before() throws URISyntaxException {
    URL u = DeliveryStatProviderTest.class.getResource("./");
    baseDir = new File(u.toURI());
  }

  private UserStatProvider createProvider() {
    return new UserStatProvider(baseDir, FileSystem.getFSForSingleInst(), "'p'yyyyMMdd");
  }

  @Test
  public void acceptTest() throws Exception {

    UserStatProvider provider = createProvider();

    UserStatFilter filter = new UserStatFilter();
    filter.setUser("siebel");

    final int[] counters = new int[7]; // PAUSED, PLANNED, ACTIVE, FINISH, CANCEL, CREATED, DELETED
    provider.accept(filter, new UserStatVisitor() {
      public boolean visit(UserStatRecord rec, int total, int current) {
        counters[0] += rec.getPaused();
        counters[1] += rec.getPlanned();
        counters[2] += rec.getActive();
        counters[3] += rec.getFinish();
        counters[4] += rec.getCancel();
        counters[5] += rec.getCreated();
        counters[6] += rec.getDeleted();
        return true;
      }
    });

    assertEquals(3, counters[0]);
    assertEquals(3, counters[1]);
    assertEquals(3, counters[2]);
    assertEquals(0, counters[3]);
    assertEquals(2, counters[4]);
    assertEquals(2, counters[5]);
    assertEquals(0, counters[6]);
  }




  private void testExpectedRecorsSize(UserStatProvider statProvider, int h_f, int h_t,  int m_f, int m_t, int expected) throws AdminException {

    final List<UserStatRecord> records = new LinkedList<UserStatRecord>();
    UserStatFilter filter = new UserStatFilter();
    Calendar c = Calendar.getInstance();
    c.setTimeZone(TimeZone.getTimeZone("UTC"));
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    c.set(2010, 9, 16, h_f, m_f);  // months starts from 0 ...
    filter.setFromDate(c.getTime());
    c.set(2010, 9, 16, h_t, m_t);
    filter.setTillDate(c.getTime());

    statProvider.accept(filter, new UserStatVisitor() {
      @Override
      public boolean visit(UserStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertEquals(records.size(), expected);

  }

  @Test
  public void testMinuteOffset() throws AdminException {

    UserStatProvider statProvider = createProvider();

    testExpectedRecorsSize(statProvider, 12, 12, 14, 15, 1);

    testExpectedRecorsSize(statProvider, 12, 12, 15, 16, 1);

    testExpectedRecorsSize(statProvider, 12, 12, 15, 15, 1);

    testExpectedRecorsSize(statProvider, 12, 12, 14, 16, 1);

    testExpectedRecorsSize(statProvider, 12, 12, 16, 17, 0);

    testExpectedRecorsSize(statProvider, 12, 12, 14, 14, 0);

    testExpectedRecorsSize(statProvider, 12, 12, 13, 14, 0);
  }


  @Test
  public void testOOOORecordFromAnotherFile()  throws AdminException{

    UserStatProvider statProvider = createProvider();

    testExpectedRecorsSize(statProvider, 9, 9, 0, 59, 1);

  }
}
