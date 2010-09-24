package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class InformerSettingsTest {

  private InformerSettings cs;

  @Before
  public void before() {
    cs = new InformerSettings();
  }

  @Test
  public void host() throws AdminException{
    try {
      cs.setHost("");
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setHost(null);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setHost("host");
    assertEquals("host", cs.getHost());
  }

  @Test
  public void adminPort() throws AdminException{
    try {
      cs.setAdminPort(-1);
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setAdminPort(65536);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setAdminPort(1);
    assertEquals(1, cs.getAdminPort());
  }

  @Test
  public void deliveriesPort() throws AdminException{
    try {
      cs.setDeliveriesPort(-1);
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setDeliveriesPort(65536);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setDeliveriesPort(1);
    assertEquals(1, cs.getDeliveriesPort());
  }

  @Test
  public void save() throws AdminException, XmlConfigException {
    cs.setAdminPort(1);
    cs.setDeliveriesPort(2);
    cs.setHost("dsa");
    XmlConfig c = new XmlConfig();
    cs.save(c);

    InformerSettings cs1 = new InformerSettings();
    cs1.load(c);

    assertEquals(cs.getHost(), cs1.getHost());
    assertEquals(cs.getAdminPort(), cs1.getAdminPort());
    assertEquals(cs.getDeliveriesPort(), cs1.getDeliveriesPort());
  }

  @Test
  public void cloneSettings() throws AdminException {
    cs.setAdminPort(1);
    cs.setDeliveriesPort(2);
    cs.setHost("dsa");

    InformerSettings cs1 = cs.cloneSettings();

    assertTrue(cs != cs1);
    assertEquals(cs.getHost(), cs1.getHost());
    assertEquals(cs.getAdminPort(), cs1.getAdminPort());
    assertEquals(cs.getDeliveriesPort(), cs1.getDeliveriesPort());
  }
}
