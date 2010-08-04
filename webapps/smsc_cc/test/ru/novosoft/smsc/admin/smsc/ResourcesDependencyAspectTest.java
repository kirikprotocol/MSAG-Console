package ru.novosoft.smsc.admin.smsc;

import org.junit.AfterClass;
import static org.junit.Assert.*;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import testutils.TestUtils;

import java.io.File;
import java.util.Locale;

/**
 * @author Artem Snopkov
 */
public class ResourcesDependencyAspectTest {

  private static File baseDir;
  private static AdminContext ctx;

  @BeforeClass
  public static void before() throws Exception {
    try {
      baseDir = TestUtils.createRandomDir("ctx");
      ctx = new TestAdminContext(baseDir, new File("test", "webconfig.xml"));
    } catch (Exception e) {
      e.printStackTrace();
      throw e;
    }
  }

  @AfterClass
  public static void after() throws AdminException {
    if (baseDir != null)
      TestUtils.recursiveDeleteFolder(baseDir);
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
