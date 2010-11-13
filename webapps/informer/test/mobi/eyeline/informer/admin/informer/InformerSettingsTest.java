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
  public void adminHost() throws AdminException{
    try {
      cs.setAdminHost("");
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setAdminHost(null);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setAdminHost("host");
    assertEquals("host", cs.getAdminHost());
  }

  @Test
  public void persHost() throws AdminException{
    try {
      cs.setPersHost("");
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setPersHost(null);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setPersHost("host");
    assertEquals("host", cs.getPersHost());
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
  public void persPort() throws AdminException{
    try {
      cs.setPersPort(-1);
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setPersPort(65536);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setPersPort(1);
    assertEquals(1, cs.getPersPort());
  }

  @Test
  public void dcpHost() throws AdminException{
    try {
      cs.setDcpHost("");
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setDcpHost(null);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setDcpHost("host");
    assertEquals("host", cs.getDcpHost());
  }

  @Test
  public void dcpPort() throws AdminException{
    try {
      cs.setDcpPort(-1);
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setDcpPort(65536);
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setDcpPort(1);
    assertEquals(1, cs.getDcpPort());
  }

  @Test
  public void cloneSettings() throws AdminException {
    cs.setAdminHost("adminHost");
    cs.setAdminPort(1);
    cs.setDcpHost("dcpHost");
    cs.setDcpPort(2);
    cs.setPersHost("persHost");
    cs.setPersPort(3);
    cs.setStatDir("statDir");
    cs.setStatusLogsDir("statusLogsDir");

    InformerSettings cs1 = cs.cloneSettings();

    assertTrue(cs != cs1);
    assertEquals(cs.getAdminHost(), cs1.getAdminHost());
    assertEquals(cs.getAdminPort(), cs1.getAdminPort());
    assertEquals(cs.getDcpHost(), cs1.getDcpHost());
    assertEquals(cs.getDcpPort(), cs1.getDcpPort());
    assertEquals(cs.getPersHost(), cs1.getPersHost());
    assertEquals(cs.getPersPort(), cs1.getPersPort());
    assertEquals(cs.getStatDir(), cs1.getStatDir());
    assertEquals(cs.getStatusLogsDir(), cs1.getStatusLogsDir());
  }
}
