package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.Test;

import java.util.Date;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryTest {

  @Test
  public void testSourceAddress() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setSourceAddress(null);
      assertTrue(false);
    }catch (AdminException e){}
    d.setSourceAddress(new Address("+791394"));
    assertEquals("+791394", d.getSourceAddress().getSimpleAddress());
  }

  @Test
  public void testName() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setName(null);
      assertTrue(false);
    }catch (AdminException e){}
    try{
      d.setName("");
      assertTrue(false);
    }catch (AdminException e){}
    d.setName("name1");
  }

  @Test
  public void testPriority() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setPriority(0);
      assertTrue(false);
    }catch (AdminException e){}
    try{
      d.setPriority(1001);
      assertTrue(false);
    }catch (AdminException e){}
    d.setPriority(32);
  }


  @Test
  public void testStartDate() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setStartDate(null);
      assertTrue(false);
    }catch (AdminException e){}
    d.setStartDate(new Date());
  }


  @Test
  public void testEndDate() {
    Delivery d = new Delivery();
    d.setEndDate(null);
    d.setEndDate(new Date());
  }

  @Test
  public void testActivePeriod() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setActivePeriodStart(null);
      assertTrue(false);
    }catch (AdminException e){}

    d.setActivePeriodStart(new Time(1,0,0));

    try{
      d.setActivePeriodEnd(null);
      assertTrue(false);
    }catch (AdminException e){}
    d.setActivePeriodEnd(new Time(1,0,0));
  }

  @Test
  public void testActiveDays() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setActiveWeekDays(null);
      assertTrue(false);
    }catch (AdminException e){}

    try{
      d.setActiveWeekDays(new Day[0]);
      assertTrue(false);
    }catch (AdminException e){}

    d.setActiveWeekDays(new Day[]{Day.Fri});
  }

  @Test
  public void testValidity() throws AdminException{
    Delivery d = new Delivery();
    try{
      d.setValidityPeriod(new Time(0,0,0));
      assertTrue(false);
    }catch (AdminException e){}
    d.setValidityPeriod(new Time(23,0,0));
  }

  @Test
  public void testDeliveryMode() throws AdminException{
    Delivery d = new Delivery();
    try{
      d.setDeliveryMode(null);
      assertTrue(false);
    }catch (AdminException e){}
    d.setDeliveryMode(DeliveryMode.USSD_PUSH);
  }

  @Test
  public void testOwner() throws AdminException{
    Delivery d = new Delivery();
    try{
      d.setOwner(null);
      assertTrue(false);
    }catch (AdminException e){}

    try{
      d.setOwner("");
      assertTrue(false);
    }catch (AdminException e){}

    d.setOwner("owner1");
  }

  @Test
  public void testRetry() {
    Delivery d = new Delivery();
    d.setRetryOnFail(false);
    d.setRetryPolicy("");
    d.setRetryPolicy(null);
    d.setRetryPolicy("dasd");
    d.setRetryOnFail(true);
    d.setRetryPolicy("");
    d.setRetryPolicy(null);
    d.setRetryPolicy("dasd");
  }


}
