package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.net.URISyntaxException;
import java.net.URL;

import static org.junit.Assert.*;

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
}
