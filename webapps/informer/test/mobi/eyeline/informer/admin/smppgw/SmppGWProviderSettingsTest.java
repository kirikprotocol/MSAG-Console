package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWProviderSettingsTest {

  private static SmppGWRoute createSmppGWRoute() {
    SmppGWRoute route = new SmppGWRoute();
    route.setUser("admin");
    route.setDeliveryId(123);
    route.addServiceNumber(new Address(".5.0.MTC"));
    return route;
  }

  private static SmppGWProvider createProvider() throws AdminException {
    SmppGWProvider p = new SmppGWProvider();
    p.setName("provider");
    p.setDescr("descr");
    p.addEndpoint("endpoint1");
    p.addRoute(createSmppGWRoute());
    return p;
  }


  private static SmppGWProviderSettings createSettings() throws AdminException {
    SmppGWProviderSettings s = new SmppGWProviderSettings();
    s.addProvider(createProvider());
    return s;
  }

  @Test
  public void testOk() throws AdminException {
    SmppGWProviderSettings s = createSettings();
    s.validate();
  }


  @Test(expected = AdminException.class)
  public void testEndpointIntersection() throws AdminException{
    SmppGWProviderSettings s = createSettings();
    SmppGWProvider p = createProvider();
    p.setName(p.getName()+"1");
    s.addProvider(p);
    s.validate();
  }



  @Test(expected = AdminException.class)
  public void testNameIntersection() throws AdminException{
    SmppGWProviderSettings s = createSettings();
    SmppGWProvider p = createProvider();
    s.addProvider(p);
  }

  @Test
  public void testSave() throws AdminException, XmlConfigException {
    SmppGWProvider p1 = createProvider();
    SmppGWProvider p2 = createProvider();
    {
      p2.clearEndpoints();
      p2.clearRoutes();

      p2.addEndpoint("endpoint_test_test");

      p2.setName(p2.getName() + '1');
      SmppGWRoute r = createSmppGWRoute();
      r.setDeliveryId(r.getDeliveryId()+1);
      r.clearServiceNumbers();
      r.addServiceNumber(new Address("+12441241"));
      p2.addRoute(r);
    }
    SmppGWProviderSettings s = new SmppGWProviderSettings();
    s.addProvider(p1);
    s.addProvider(p2);
    s.validate();

    XmlConfigSection section = new XmlConfigSection("section");
    s.save(section);

    SmppGWProviderSettings s1 = new SmppGWProviderSettings();
    s1.load(section);

    assertEquals(s, s1);

  }






}
