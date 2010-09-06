package ru.novosoft.smsc.admin.region;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.route.RouteSubjectManager;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;
import testutils.TestUtils;

import java.io.File;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class SubjectDependencyAspectTest {

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
  public void testBeforeUpdateRegions() throws AdminException {
    RegionManager rm = ctx.getRegionManager();
    RegionSettings s = rm.getSettings();

    s.createRegion("newRegion", 0, null, Arrays.asList("unknown subject"));

    try {
      rm.updateSettings(s);
      assertFalse(true);
    } catch (RegionException ignored) {}
  }

  @Test
  public void testBeforeUpdateRegionSettings() throws AdminException {
    RouteSubjectManager rsm = ctx.getRouteSubjectManager();

    RouteSubjectSettings s = rsm.getSettings();

    List<Subject> subjects = s.getSubjects();
    for (Iterator<Subject> iter = subjects.iterator(); iter.hasNext();)
      if (iter.next().getName().equals("forRegions"))
        iter.remove();

    s.setSubjects(subjects);

    try {
      rsm.updateSettings(s);
      assertFalse(true);
    } catch (RegionException ignored) {}
  }
}
