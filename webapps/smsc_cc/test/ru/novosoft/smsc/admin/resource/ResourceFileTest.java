package ru.novosoft.smsc.admin.resource;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class ResourceFileTest {

  private File configFile;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ResourceFileTest.class.getResourceAsStream("resources_en_en.xml"), ".resource");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
  }

  private ResourceSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new ResourceFile());
  }

  private void saveSettings(ResourceSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new ResourceFile(), s);
  }


  @Test
  public void testSave() throws Exception {
    ResourceSettings s = loadSettings();
    saveSettings(s);

    testLoad();
  }

  @Test
  public void testLoad() throws Exception {
    ResourceSettings s = loadSettings();

    assertEquals(".", s.getDecimalDelimiter());
    assertEquals(",", s.getListDelimiter());

    assertEquals("access denied", s.getResources().getChild("dl").getChild("reason").getString("access_denied"));
  }
}
