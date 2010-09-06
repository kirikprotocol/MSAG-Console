package ru.novosoft.smsc.admin.timezone;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.RouteSubjectManager;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;
import testutils.TestUtils;

import java.io.File;
import java.util.*;

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

  @Test (expected = TimezoneException.class)
  public void testBeforeUpdateTimezones() throws AdminException {
    Timezone tz;
    TimezoneSettings s;
    try {
      tz = new Timezone("unknown subject", TimeZone.getTimeZone("Europe/Moscow"));
      s = ctx.getTimezoneManager().getSettings();
      s.setTimezones(Arrays.asList(tz));
    } catch (AdminException e) {
      e.printStackTrace();
      assertFalse(true);
      return;
    }

    ctx.getTimezoneManager().updateSettings(s);
  }

  @Test(expected = TimezoneException.class)
  public void testBeforeUpdateRouteSettings() throws Exception {
    RouteSubjectManager rsm = ctx.getRouteSubjectManager();

    RouteSubjectSettings s = rsm.getSettings();

    List<Subject> subjects = s.getSubjects();
    for (Iterator<Subject> iter = subjects.iterator(); iter.hasNext();)
      if (iter.next().getName().equals("forTimezones"))
        iter.remove();

    s.setSubjects(subjects);

    rsm.updateSettings(s);
  }
}
