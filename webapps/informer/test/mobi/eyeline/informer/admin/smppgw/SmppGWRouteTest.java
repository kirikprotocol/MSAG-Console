package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import static org.junit.Assert.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWRouteTest {

  private static SmppGWRoute createSmppGWRoute() {
    SmppGWRoute route = new SmppGWRoute();
    route.setUser("admin");
    route.setDeliveryId(123);
    route.addServiceNumber(new Address(".5.0.MTS"));
    return route;
  }

  @Test
  public void testOk() throws AdminException {
    createSmppGWRoute().validate();
  }

  @Test(expected = AdminException.class)
  public void testNullUser() throws AdminException {
    SmppGWRoute r = createSmppGWRoute();
    r.setUser(null);
    r.validate();
  }

  @Test(expected = AdminException.class)
  public void testEmptyUser() throws AdminException {
    SmppGWRoute r = createSmppGWRoute();
    r.setUser("");
    r.validate();
  }

  @Test(expected = AdminException.class)
  public void testNegativeDeliveryId() throws AdminException {
    SmppGWRoute r = createSmppGWRoute();
    r.setDeliveryId(-1);
    r.validate();
  }

  @Test(expected = AdminException.class)
  public void testEmptyNumbers() throws AdminException {
    SmppGWRoute r = createSmppGWRoute();
    r.clearServiceNumbers();
    r.validate();
  }

  @Test
  public void testEquals() {
    SmppGWRoute e1 = createSmppGWRoute();
    SmppGWRoute e2 = createSmppGWRoute();
    assertEquals(e1, e2);
  }

  @Test
  public void testNotEquals() {
    SmppGWRoute e1 = createSmppGWRoute();
    SmppGWRoute e2 = createSmppGWRoute();
    e2.addServiceNumber(new Address("+4234141"));
    assertNotSame(e1, e2);
  }

  @Test
  public void testCopy() {
    SmppGWRoute e1 = createSmppGWRoute();
    assertEquals(e1, new SmppGWRoute(e1));
  }

  @Test
  public void testAddRemoveServiceNumber() {
    SmppGWRoute route = createSmppGWRoute();
    Address ad = new Address("+4231412412412");
    route.addServiceNumber(ad);
    assertTrue(route.getServiceNumbers().contains(new Address(ad)));
    route.removeServiceNumber(ad.getSimpleAddress());
    assertFalse(route.getServiceNumbers().contains(new Address(ad)));
  }

}
