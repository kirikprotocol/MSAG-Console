package ru.novosoft.smsc.admin.reschedule;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

/**
 * @author Artem Snopkov
 */
public class RescheduleConfigTest {
  
  private File configFile;
  private MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = fs.createNewFile("schedule.xml", RescheduleManagerImplTest.class.getResourceAsStream("schedule.xml"));
  }

  private RescheduleSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new RescheduleConfig(), fs);
  }

  private void saveSettings(RescheduleSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new RescheduleConfig(), s, fs);
  }
  
  @Test
  public void testLoad() throws Exception {
    RescheduleSettings s = loadSettings();

    assertEquals(20, s.getRescheduleLimit());
    assertEquals("30s,1m,5m,15m,30m,1h,6h,12h,1d:*", s.getDefaultReschedule());

    Collection<Reschedule> reschedules = s.getReschedules();
    assertNotNull(reschedules);
    assertEquals(2, reschedules.size());

    Iterator<Reschedule> iter = reschedules.iterator();

    assertEquals(new Reschedule("30s,11m,15m", 8), iter.next());
    assertEquals(new Reschedule("30s,1m,5m,15m,30m,1h,6h,12h,1d", 1028, 255, 20, 1027, 88, 100, 69), iter.next());
  }

  @Test
  public void testSave() throws Exception {
    {
      RescheduleSettings s = loadSettings();
      s.setDefaultReschedule("13m");
      saveSettings(s);
    }

    {
      RescheduleSettings s = loadSettings();
      assertEquals("13m", s.getDefaultReschedule());
    }
  }

  @Test
  public void testSaveUnusedParams() throws Exception {
    XmlConfig cfg = new XmlConfig();
    cfg.load(fs.getInputStream(configFile));

    RescheduleSettings s = loadSettings();
    saveSettings(s);

    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(fs.getInputStream(configFile));

    assertEquals(cfg, cfg1);
  }
}
