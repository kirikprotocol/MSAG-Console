package mobi.eyeline.informer.web.config;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

/**
 * @author Artem Snopkov
 */
//@Ignore
public class TimezonesConfigTest {

  private File configFile;


  @Before
  public void setUp() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(TimezonesConfigTest.class.getResourceAsStream("timezones.xml"), ".timezones");
  }

  @SuppressWarnings("ResultOfMethodCallIgnored")
  @After
  public void tearDown() throws Exception {
    if (configFile != null)
      configFile.delete();
  }

  @Test
  public void testGetTimezones() throws Exception {
    TimezonesConfig cfg = new TimezonesConfig(configFile);

    List<InformerTimezone> timezones = cfg.getTimezones().getTimezones();
    assertNotNull(timezones);
    assertEquals(2, timezones.size());

    {
      InformerTimezone msktz = timezones.get(0);
      assertNotNull(msktz.getTimezone().getID());
      assertEquals("Europe/Moscow", msktz.getTimezone().getID());
      assertNotNull(msktz);
      assertNotNull(msktz.getAliases());

      assertEquals(2, msktz.getAliases().size());
      assertEquals("(UTC+3) Москва", msktz.getAliases().get("ru"));
      assertEquals("(UTC+3) Moscow", msktz.getAliases().get("en"));
    }

    {
      InformerTimezone nsktz = timezones.get(1);
      assertNotNull(nsktz.getTimezone().getID());
      assertEquals("Asia/Novosibirsk", nsktz.getTimezone().getID());
      assertNotNull(nsktz);
      assertNotNull(nsktz.getAliases());
    }
  }
}
