package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.config.XmlConfigSection;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWEndpointSettingsTest {



  private static SmppGWEndpoint createEndpoint() {
    SmppGWEndpoint e = new SmppGWEndpoint();
    e.setName("enpoint1");
    e.setSystemId("systemid");
    e.setPassword("password");
    return e;
  }


  private static SmppGWEndpointSettings createSettings() throws AdminException {
    SmppGWEndpointSettings s = new SmppGWEndpointSettings();
    s.addEndpoint(createEndpoint());
    return s;
  }


  @Test
  public void testOk() throws AdminException{
    createSettings().validate();
  }


  @Test(expected =  AdminException.class)
  public void testSystemIdIntersection() throws AdminException {
    SmppGWEndpointSettings s = createSettings();
    SmppGWEndpoint e = createEndpoint();
    e.setName(e.getName()+'1');
    s.addEndpoint(e);
    s.validate();
  }

  @Test(expected =  AdminException.class)
  public void testNameIntersection() throws AdminException {
    SmppGWEndpointSettings s = createSettings();
    SmppGWEndpoint e = createEndpoint();
    s.addEndpoint(e);
  }


  @Test
  public void testSave() throws Exception {
    SmppGWEndpoint e1 = createEndpoint();
    SmppGWEndpoint e2 = createEndpoint();
    e2.setName(e2.getName() + '1');
    e2.setSystemId(e2.getSystemId() + "a");
    SmppGWEndpointSettings s = new SmppGWEndpointSettings();
    s.addEndpoint(e1);
    s.addEndpoint(e2);
    s.validate();

    XmlConfigSection section = new XmlConfigSection("section");
    s.save(section);

    SmppGWEndpointSettings s2 = new SmppGWEndpointSettings();
    s2.load(section);

    assertEquals(s, s2);
  }


}
