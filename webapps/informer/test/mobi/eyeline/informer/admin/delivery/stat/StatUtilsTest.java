package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.net.URISyntaxException;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.List;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class StatUtilsTest {

  static File baseDir;

  @BeforeClass
  public static void before() throws URISyntaxException {
    URL u = DeliveryStatProviderTest.class.getResource("./");
    baseDir = new File(u.toURI());
  }

  @Test
  public void testLookupFilesWithNoDates() {

    List<StatFile> files = StatUtils.lookupFiles(new TestFileSystem(), baseDir, new SimpleDateFormat("'p'yyyyMMdd" + File.separator + "'dlv'HH'.log'"), null, null);

    assertEquals(3, files.size());
  }

  @Test
  public void testLookupFilesWithFromDate() {

    Calendar c = Calendar.getInstance();
    c.set(Calendar.YEAR, 2010);
    c.set(Calendar.MONTH, 9);
    c.set(Calendar.DATE, 16);
    c.set(Calendar.HOUR_OF_DAY, 12);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);

    List<StatFile> files = StatUtils.lookupFiles(new TestFileSystem(), baseDir, new SimpleDateFormat("'p'yyyyMMdd" + File.separator + "'dlv'HH'.log'"), c.getTime(), null);

    assertEquals(1, files.size());
  }

  @Test
  public void testLookupFilesWithTillDate() {

    Calendar c = Calendar.getInstance();
    c.set(Calendar.YEAR, 2010);
    c.set(Calendar.MONTH, 9);
    c.set(Calendar.DATE, 16);
    c.set(Calendar.HOUR_OF_DAY, 11);
    c.set(Calendar.MINUTE, 30);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);

    List<StatFile> files = StatUtils.lookupFiles(new TestFileSystem(), baseDir, new SimpleDateFormat("'p'yyyyMMdd" + File.separator + "'dlv'HH'.log'"), null, c.getTime());

    assertEquals(2, files.size());
  }

}
