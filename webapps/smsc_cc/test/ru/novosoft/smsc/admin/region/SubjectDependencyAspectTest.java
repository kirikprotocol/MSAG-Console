package ru.novosoft.smsc.admin.region;

import org.junit.*;

import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
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

  private AdminContext ctx;

  @Before
  public  void before() throws Exception {
    try {
      MemoryFileSystem fs = new MemoryFileSystem();
      ctx = new TestAdminContext(fs.mkdirs("ctx"), new File("test", "webconfig.xml"), 2, fs);
    } catch (Exception e) {
      e.printStackTrace();
      throw e;
    }
  }

  @After
  public void after() throws AdminException {
    ctx.shutdown();
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
