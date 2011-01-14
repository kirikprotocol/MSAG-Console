package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.Before;
import org.junit.Test;

import java.util.Date;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryTest {

  private Delivery d;

  @Before
  public void before() throws AdminException{
    d = new Delivery();
    d.setType(Delivery.Type.IndividualTexts);
    d.setSourceAddress(new Address("+7901111"));
    d.setActivePeriodEnd(new Time(20,0,0));
    d.setActivePeriodStart(new Time(9,0,0));
    d.setActiveWeekDays(new Day[]{Day.Fri, Day.Mon, Day.Thu, Day.Wed, Day.Thu,});
    d.setDeliveryMode(DeliveryMode.SMS);
    d.setName("Test delivery");
    d.setOwner("a");
    d.setPriority(31);
    d.setStartDate(new Date(System.currentTimeMillis() - 86400000L));
    d.setSvcType("svc1");
    d.setValidityPeriod(new Time(1,0,0));
    d.validate();
  }

  @Test
  public void testSourceAddress() throws AdminException {

    try{
      d.setSourceAddress(null);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}
    d.setSourceAddress(new Address("+791394"));
    assertEquals("+791394", d.getSourceAddress().getSimpleAddress());
  }

  @Test
  public void testName() throws AdminException {

    try{
      d.setName(null);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}
    try{
      d.setName("");
      d.validate(); assertTrue(false);
    }catch (AdminException e){}
    d.setName("name1");
  }

  @Test
  public void testPriority() throws AdminException {

    try{
      d.setPriority(0);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}
    try{
      d.setPriority(1001);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}
    d.setPriority(32);
  }


  @Test
  public void testStartDate() throws AdminException {

    try{
      d.setStartDate(null);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}
    d.setStartDate(new Date());
  }


  @Test
  public void testEndDate() {

    d.setEndDate(null);
    d.setEndDate(new Date());
  }

  @Test
  public void testActivePeriod() throws AdminException {

    try{
      d.setActivePeriodStart(null);
      d.validate(); fail();
    }catch (AdminException e){}

    try{
      d.setActivePeriodStart(new Time(24,0,0));
      d.validate(); fail();
    }catch (AdminException e){}

    d.setActivePeriodStart(new Time(1,0,0));

    try{
      d.setActivePeriodEnd(null);
      d.validate(); fail();
    }catch (AdminException e){}

    try{
      d.setActivePeriodEnd(new Time(25,0,0));
      d.validate(); fail();
    }catch (AdminException e){}

    d.setActivePeriodEnd(new Time(1,0,0));
  }

  @Test
  public void testActiveDays() throws AdminException {

    try{
      d.setActiveWeekDays(null);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}

    try{
      d.setActiveWeekDays(new Day[0]);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}

    d.setActiveWeekDays(new Day[]{Day.Fri});
  }

  @Test
  public void testValidity() throws AdminException{

    try{
      d.setValidityPeriod(new Time(0,0,1));
      d.validate(); fail();
    }catch (AdminException e){}

    d.setValidityPeriod(new Time(23,0,0));
    d.setValidityPeriod(new Time(25,0,0));
    d.setValidityPeriod(new Time(0,1,0));
  }

  @Test
  public void testDeliveryMode() throws AdminException{

    try{
      d.setDeliveryMode(null);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}
    d.setDeliveryMode(DeliveryMode.USSD_PUSH);
  }

  @Test
  public void testOwner() throws AdminException{

    try{
      d.setOwner(null);
      d.validate(); assertTrue(false);
    }catch (AdminException e){}

    try{
      d.setOwner("");
      d.validate(); assertTrue(false);
    }catch (AdminException e){}

    d.setOwner("owner1");
  }

  @Test
  public void testRetry() throws AdminException{
    d.setRetryOnFail(false);
    d.setRetryPolicy("");
    d.validate();
    d.setRetryPolicy(null);
    d.validate();
    d.setRetryPolicy("1h");
    d.validate();
    d.setRetryOnFail(true);
    d.setRetryPolicy("");
    d.validate();
    d.setRetryPolicy(null);
    d.setRetryPolicy("1m");
    d.validate();
  }


}
