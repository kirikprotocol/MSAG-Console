package ru.novosoft.smsc.admin.timezone;

import org.junit.*;

import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
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
