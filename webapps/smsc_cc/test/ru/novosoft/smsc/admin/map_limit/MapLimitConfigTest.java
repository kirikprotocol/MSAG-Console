package ru.novosoft.smsc.admin.map_limit;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

/**
 * @author Artem Snopkov
 */
public class MapLimitConfigTest {
  
  private File configFile;

  @Before
  public void before() throws IOException {
    configFile = TestUtils.exportResourceToRandomFile(MapLimitConfigTest.class.getResourceAsStream("maplimits.xml"), ".maplimits");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
  }
  
  private MapLimitSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new MapLimitConfig());
  }

  private void saveSettings(MapLimitSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new MapLimitConfig(), s);
  }
  
  @Test
  public void testLoad() throws Exception {
    MapLimitSettings settings = loadSettings();

    assertEquals(10000, settings.getDlgLimitIn());
    assertEquals(10000, settings.getDlgLimitInSri());
    assertEquals(500, settings.getDlgLimitInUssd());
    assertEquals(1000, settings.getDlgLimitOutSri());
    assertEquals(1000, settings.getDlgLimitUssd());

    int[] noSriCodes = settings.getUssdNoSriCodes();
    assertNotNull(noSriCodes);
    assertEquals(3, noSriCodes.length);
    assertArrayEquals(new int[]{12, 15, 18}, noSriCodes);

    CongestionLevel clevels[] = settings.getCongestionLevels();
    assertNotNull(clevels);
    assertEquals(MapLimitManager.MAX_CONGESTON_LEVELS, clevels.length);
    assertEquals(new CongestionLevel(4000, 800, 1000, 100), clevels[0]);
    assertEquals(new CongestionLevel(3000, 1800, 2000, 100), clevels[1]);
    assertEquals(new CongestionLevel(2000, 2800, 3000, 100), clevels[2]);
    assertEquals(new CongestionLevel(1500, 3800, 4000, 100), clevels[3]);
    assertEquals(new CongestionLevel(1000, 4800, 5000, 100), clevels[4]);
    assertEquals(new CongestionLevel(800, 5800, 6000, 100), clevels[5]);
    assertEquals(new CongestionLevel(600, 6800, 7000, 100), clevels[6]);
    assertEquals(new CongestionLevel(400, 7800, 8000, 80), clevels[7]);
  }

  @Test
  public void testSave() throws Exception {

    MapLimitSettings s = loadSettings();

    s.setDlgLimitIn(1231);
    s.setDlgLimitInSri(321);

    saveSettings(s);

    MapLimitSettings s1 = loadSettings();

    assertEquals(1231, s1.getDlgLimitIn());
    assertEquals(321, s1.getDlgLimitInSri());
  }
}
