package ru.novosoft.smsc.admin.region;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.TimeZone;

/**
 * @author Artem Snopkov
 */
public class RegionsConfigTest {

  private File configFile;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(RegionsConfigTest.class.getResourceAsStream("regions.xml"), ".regions");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
  }

  RegionSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new RegionsConfig());
  }

  private void saveSettings(RegionSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new RegionsConfig(), s);
  }

  private void checkSettings(RegionSettings s) {
    assertEquals(123, s.getDefaultBandwidth());
    assertEquals(null, s.getDefaultEmail());
    Region r = s.getRegion(1);
    assertNotNull(r);

    assertEquals("test", r.getName());
    assertEquals(1, r.getId());
    assertEquals(12, r.getBandwidth());
    assertEquals("ee@eyeline.mobi", r.getEmail());
    assertEquals(TimeZone.getTimeZone("AST"), r.getTimezone());

    assertEquals(1, s.getRegions().size());
  }

  @Test
  public void testSave() throws Exception {
    RegionSettings s = loadSettings();
    saveSettings(s);
    s = loadSettings();
    checkSettings(s);
  }

  @Test
  public void testLoad() throws Exception {
    RegionSettings s = loadSettings();
    checkSettings(s);
  }
}
