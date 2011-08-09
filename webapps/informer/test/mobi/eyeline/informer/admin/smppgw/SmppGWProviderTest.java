package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import static org.junit.Assert.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWProviderTest {

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

  @Test
  public void testOk() throws AdminException{
    createProvider().validate();
  }

  @Test(expected = AdminException.class)
  public void testNullName() throws AdminException {
    SmppGWProvider p = createProvider();
    p.setName(null);
    p.validate();
  }

  @Test(expected = AdminException.class)
  public void testEmptyName() throws AdminException {
    SmppGWProvider p = createProvider();
    p.setName("");
    p.validate();
  }

  @Test
  public void testAddRemoveRoute() throws AdminException {
    SmppGWProvider p = createProvider();
    SmppGWRoute route = createSmppGWRoute();
    route.setDeliveryId(3112312);

    p.addRoute(route);
    assertTrue(p.getRoutes().contains(new SmppGWRoute(route)));
    p.removeRoute(route.getDeliveryId());
    assertFalse(p.getRoutes().contains(new SmppGWRoute(route)));
  }

  @Test
  public void testAddRemoveEndpoint() throws AdminException {
    SmppGWProvider p = createProvider();
    String endpoint = "endpointtest";

    p.addEndpoint(endpoint);
    assertTrue(p.getEndpoints().contains(endpoint));
    p.removeEndpoint(endpoint);
    assertFalse(p.getEndpoints().contains(endpoint));
  }

  @Test
  public void testEquals() throws AdminException {
    SmppGWProvider p1 = createProvider();
    SmppGWProvider p2 = createProvider();
    assertEquals(p1, p2);
  }

  @Test
  public void testNotEquals() throws AdminException {
    SmppGWProvider p1 = createProvider();
    SmppGWProvider p2 = createProvider();
    p2.addEndpoint("endsadsadas1");
    assertNotSame(p1, p2);
  }

  @Test
  public void testCopy() throws AdminException {
    SmppGWProvider p1 = createProvider();
    assertEquals(p1, new SmppGWProvider(p1));
  }

  @Test(expected = AdminException.class)
  public void testAddressIntersection() throws AdminException {
    SmppGWProvider p = createProvider();
    SmppGWRoute r = createSmppGWRoute();
    r.setDeliveryId(r.getDeliveryId()+1);
    p.addRoute(r);
    p.validate();
  }

  @Test(expected = AdminException.class)
  public void testRouteNameIntersection() throws AdminException {
    SmppGWProvider p = createProvider();
    SmppGWRoute r = createSmppGWRoute();
    p.addRoute(r);
  }

}
