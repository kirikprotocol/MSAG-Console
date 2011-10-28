package ru.novosoft.smsc.admin.timezone;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.config.XmlConfig;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.List;
import java.util.TimeZone;

/**
 * @author Artem Snopkov
 */
public class TimezonesConfigTest {

  private File configFile;
  private MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = fs.createNewFile("tz.xml", TimezonesConfigTest.class.getResourceAsStream("timezones.xml"));
  }

  private TimezoneSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new TimezonesConfig(), fs);
  }

  private void saveSettings(TimezoneSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new TimezonesConfig(), s, fs);
  }

  @Test
  public void testSave() throws Exception {
    XmlConfig oldConfig = new XmlConfig();
    oldConfig.load(fs.getInputStream(configFile));

    TimezoneSettings s = loadSettings();
    saveSettings(s);

    XmlConfig newConfig = new XmlConfig();
    newConfig.load(fs.getInputStream(configFile));

    assertEquals(oldConfig, newConfig);
  }

  @Test
  public void testLoad() throws Exception {
    TimezoneSettings s = loadSettings();

    assertEquals(TimeZone.getTimeZone("Europe/Moscow"), s.getDefaultTimeZone());

    Collection<Timezone> timezones = s.getTimezones();
    assertEquals(2, timezones.size());

    for (Timezone t : timezones) {
      if (t.getMask() != null) {
        assertEquals(new Address("+7913949????"), t.getMask());
        assertEquals(TimeZone.getTimeZone("Asia/Novosibirsk"), t.getTimezone());
      } else {
        assertEquals("forTimezones", t.getSubject());
        assertEquals(TimeZone.getTimeZone("Europe/Moscow"), t.getTimezone());
      }
    }
  }
}
