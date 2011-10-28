package ru.novosoft.smsc.admin.smsc;

import org.junit.*;

import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import testutils.TestUtils;

import java.io.File;
import java.util.Locale;

/**
 * @author Artem Snopkov
 */
public class ResourcesDependencyAspectTest {

  private AdminContext ctx;

  @Before
  public void before() throws Exception {
    try {
      MemoryFileSystem fs = new MemoryFileSystem();
      ctx = new TestAdminContext(fs.mkdirs("ctx"), new File("test", "webconfig.xml"), 2, fs);
    } catch (Exception e) {
      e.printStackTrace();
      throw e;
    }
  }

  @After
  public  void after() throws AdminException {
    ctx.shutdown();
  }

  @Test
  public void testBeforeUpdateSettings() throws Exception {
    SmscManager m = ctx.getSmscManager();

    SmscSettings s = m.getSettings();
    CommonSettings cs = s.getCommonSettings();
    cs.setLocales(new String[]{"ru_ru", "en_en", "de_de"});

    s.setCommonSettings(cs);
    
    try {
      m.updateSettings(s);
      assertTrue(false);
    } catch (AdminException e) {
      System.out.println(e.getMessage(new Locale("en")));
    }

    cs.setLocales(new String[]{"ru_ru", "en_en"});

    s.setCommonSettings(cs);
    m.updateSettings(s);
  }

  @Test
  public void testBeforeRemoveResource() throws Exception {
    ResourceManager m = ctx.getResourceManager();

    try {
      m.removeResourceSettings("en_en");
      assertTrue(false);
    } catch (AdminException e) {
      System.out.println(e.getMessage(new Locale("en")));
    }

  }
}
