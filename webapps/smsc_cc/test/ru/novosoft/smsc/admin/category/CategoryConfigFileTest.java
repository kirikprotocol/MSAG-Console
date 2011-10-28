package ru.novosoft.smsc.admin.category;

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

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class CategoryConfigFileTest  {

  private File configFile;
  private MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = fs.createNewFile("webconfig.xml", CategoryConfigFileTest.class.getResourceAsStream("webconfig.xml"));
  }

  private CategorySettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new CategoryConfigFile(), fs);
  }

  private void saveSettings(CategorySettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new CategoryConfigFile(), s, fs);
  }

  @Test
  public void testSave() throws Exception {
    XmlConfig cfg = new XmlConfig();
    cfg.load(fs.getInputStream(configFile));

    CategorySettings s = loadSettings();
    saveSettings(s);

    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(fs.getInputStream(configFile));

    assertEquals(cfg, cfg1);
  }

  @Test
  public void testLoad() throws Exception {
    CategorySettings s = loadSettings();

    assertEquals("hello", s.getCategory(1).getName());
    assertEquals("mark", s.getCategory(2).getName());
    assertEquals("burger", s.getCategory(3).getName());
    assertEquals(3, s.getLastCategoryId());
  }
}
