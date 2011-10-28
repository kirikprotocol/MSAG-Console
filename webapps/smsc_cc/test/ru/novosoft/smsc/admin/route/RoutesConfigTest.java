package ru.novosoft.smsc.admin.route;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RoutesConfigTest {

  private File configFile;
  private MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = fs.createNewFile("routes.xml", RoutesConfigTest.class.getResourceAsStream("routes.xml"));
  }

  private RouteSubjectSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new RoutesConfig(null), fs);
  }

  private void saveSettings(RouteSubjectSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new RoutesConfig(null), s, fs);
  }

  private void checkSettings(RouteSubjectSettings s) {
    List<Route> routes = s.getRoutes();
    Route r = getRoute(routes, "websms > abonents");

    assertNotNull(r);

    List<Source> sources = r.getSources();
    assertNotNull(sources);
    assertEquals(1, sources.size());

    Source src = sources.get(0);
    assertNull(src.getSubject());
    assertEquals(new Address(".5.0.sms.mts.ru"), src.getMask());

    List<Destination> destinations = r.getDestinations();
    assertNotNull(destinations);
    assertEquals(1, destinations.size());

    Destination dst = destinations.get(0);
    assertNull(dst.getSubject());
    assertEquals("SILENT", dst.getSmeId());
    assertEquals(new Address(".1.1.???????????"), dst.getMask());

    assertEquals(TrafficMode.PROHIBITED, r.getTrafficMode());
  }

  @Test
  public void testSave() throws Exception {
    RouteSubjectSettings s = loadSettings();
    saveSettings(s);
    s = loadSettings();
    checkSettings(s);
  }

  @Test
  public void testLoad() throws Exception {
    RouteSubjectSettings s = loadSettings();
    checkSettings(s);
  }

  private static Route getRoute(List<Route> routes, String name) {
    for (Route r : routes)
      if (r.getName().equals(name))
        return r;
    return null;
  }
}
