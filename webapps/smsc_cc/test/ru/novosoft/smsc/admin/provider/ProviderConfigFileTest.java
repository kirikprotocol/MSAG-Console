package ru.novosoft.smsc.admin.provider;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import ru.novosoft.smsc.util.config.XmlConfig;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class ProviderConfigFileTest {

  private File configFile;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ProviderConfigFileTest.class.getResourceAsStream("webconfig.xml"), ".providers");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
  }

  private ProviderSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new ProviderConfigFile());
  }

  private void saveSettings(ProviderSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new ProviderConfigFile(), s);
  }

  @Test
  public void testSave() throws Exception {
    XmlConfig cfg = new XmlConfig();
    cfg.load(configFile);

    ProviderSettings s = loadSettings();
    saveSettings(s);

    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    assertEquals(cfg, cfg1);
  }

  @Test
  public void testLoad() throws Exception {
    ProviderSettings s = loadSettings();

    assertEquals("hello", s.getProvider(1).getName());
    assertEquals("mark", s.getProvider(2).getName());
    assertEquals("burger", s.getProvider(3).getName());
    assertEquals(3, s.getLastProviderId());
  }
}
