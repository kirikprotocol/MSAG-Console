package ru.novosoft.smsc.admin.sme;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import static org.junit.Assert.*;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.resource.ResourceSettings;
import ru.novosoft.smsc.admin.resource.Resources;
import testutils.TestUtils;

import java.io.File;
import java.util.Locale;

/**
 * @author Artem Snopkov
 */
public class ResourceDependencyAspectTest {

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
    ctx.shutdown();
    if (baseDir != null)
      TestUtils.recursiveDeleteFolder(baseDir);
  }

  @Test
  public void testBeforeAddSme() throws Exception {

    SmeManager m = ctx.getSmeManager();

    Sme sme = m.getSme("DSTRLST");
    sme.setReceiptSchemeName("empty");

    try {
      m.addSme("DSTRLST", sme);
      assertFalse(true);
    } catch (AdminException e) {
      System.out.println(e.getMessage(new Locale("en")));
    }
  }

  @Test
  public void testBeforeAddResource() throws Exception {

    ResourceManager m = ctx.getResourceManager();
    ResourceSettings s = new ResourceSettings(".", ",", new Resources());

    try {
      m.addResourceSettings("en_en", s);
      assertFalse(true);
    } catch (AdminException e) {
      System.out.println(e.getMessage(new Locale("en")));
    }

  }
}
