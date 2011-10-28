package ru.novosoft.smsc.admin.route;

import org.junit.*;

import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.util.Arrays;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class AclDependencyAspectTest {

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
  public void testBeforeRemoveAcl() throws Exception {
    ctx.getAclManager().removeAcl(2);

    try {
      ctx.getAclManager().removeAcl(1);
      assertFalse(true);
    } catch (RouteException ignored) {}
  }

  @Test
  public void testBeforeUpdateRouteSettings() throws Exception {
    RouteSubjectManager rsm = ctx.getRouteSubjectManager();
    RouteSubjectSettings settings = rsm.getSettings();

    List<Route> routes = settings.getRoutes();
    Route r = new Route("myRoute");
    r.setSrcSmeId("test");
    r.setSources(Arrays.asList(new Source(new Address("+79139495113"))));
    r.setDestinations(Arrays.asList(new Destination(new Address("+79139495113"), "infosme")));
    r.setAclId(223232); // Invalid Acl id

    routes.add(r);
    settings.setRoutes(routes);

    try {
      rsm.updateSettings(settings);
      assertFalse(true);
    } catch (RouteException ignored) {}

  }
}
