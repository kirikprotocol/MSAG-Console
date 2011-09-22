package ru.novosoft.smsc.admin.route;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.util.Arrays;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class SmeDependencyAspectTest {

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

  @Test(expected = RouteException.class)
  public void testBeforeRemoveSme() throws AdminException {
    SmeManager smeManager = ctx.getSmeManager();
    smeManager.removeSme("infosme");
  }

  @Test
  public void testBeforeUpdateRouteSettings1() throws AdminException {
    RouteSubjectManager m = ctx.getRouteSubjectManager();

    Subject s= new Subject("mySubject");

    s.setDefaultSmeId("unknown sme");

    RouteSubjectSettings settings= m.getSettings();

    List<Subject> subjects = settings.getSubjects();
    subjects.add(s);

    settings.setSubjects(subjects);

    try {
      m.updateSettings(settings);
      assertTrue(false);
    } catch (RouteException ignored) {}    
  }

  @Test
  public void testBeforeUpdateRouteSettings2() throws AdminException {
    RouteSubjectManager m = ctx.getRouteSubjectManager();
    RouteSubjectSettings settings= m.getSettings();

    Route r = new Route("myRoute");
    r.setSrcSmeId("123");
    r.setSrcSmeId("unknown sme");
    r.setSources(Arrays.asList(new Source(new Address("+79139495113"))));
    r.setDestinations(Arrays.asList(new Destination(new Address("+79139495113"), "infosme")));

    List<Route> routes = settings.getRoutes();
    routes.add(r);

    settings.setRoutes(routes);

    try {
      m.updateSettings(settings);
      assertTrue(false);
    } catch (RouteException ignored) {}
  }

  @Test
  public void testBeforeUpdateRouteSettings3() throws AdminException {
    RouteSubjectManager m = ctx.getRouteSubjectManager();
    RouteSubjectSettings settings= m.getSettings();

    Route r = new Route("myRoute");
    r.setSources(Arrays.asList(new Source(new Address("+79139495113"))));
    r.setDestinations(Arrays.asList(new Destination(new Address("+79139495113"), "unknown sme")));

    List<Route> routes = settings.getRoutes();
    routes.add(r);

    settings.setRoutes(routes);

    try {
      m.updateSettings(settings);
      assertTrue(false);
    } catch (RouteException ignored) {}
  }



}
