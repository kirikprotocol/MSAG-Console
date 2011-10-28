package ru.novosoft.smsc.admin.route;

import org.junit.*;

import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
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
    r.setSrcSmeId("123");
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
