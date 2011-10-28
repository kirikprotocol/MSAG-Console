package ru.novosoft.smsc.admin.route;

import org.junit.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.util.Arrays;
import java.util.List;

import static org.junit.Assert.assertFalse;

/**
 * @author Artem Snopkov
 */
public class ProviderDependencyAspectTest {

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
  public void testBeforeUpdateRouteSettings() throws Exception {
    RouteSubjectManager rsm = ctx.getRouteSubjectManager();
    RouteSubjectSettings settings = rsm.getSettings();

    List<Route> routes = settings.getRoutes();
    Route r = new Route("myRoute");
    r.setSrcSmeId("123");
    r.setSources(Arrays.asList(new Source(new Address("+79139495113"))));
    r.setDestinations(Arrays.asList(new Destination(new Address("+79139495113"), "infosme")));
    r.setProviderId(223232L); // Invalid Provider id

    routes.add(r);
    settings.setRoutes(routes);

    try {
      rsm.updateSettings(settings);
      assertFalse(true);
    } catch (RouteException ignored) {
    }

  }
}
