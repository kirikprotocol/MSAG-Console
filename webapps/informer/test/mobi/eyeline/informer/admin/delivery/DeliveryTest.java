package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import java.util.Date;

import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryTest {

  /*

private boolean replaceMessage;
private String svcType;
*/

  @Test
  public void testUserId() {
    Delivery d = new Delivery();
    d.setUserId("name1");
    d.setUserId(null);
    d.setUserId("");
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
  public void testEndDate() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setEndDate(null);
      assertTrue(false);
    }catch (AdminException e){}
    d.setEndDate(new Date());
  }


  @Test
  public void testStartEndDate() throws AdminException {
    Delivery d = new Delivery();
    d.setEndDate(new Date(100));
    try{
      d.setStartDate(new Date());
      assertTrue(false);
    }catch (AdminException e){}

    d.setStartDate(new Date(99));


    d = new Delivery();
    d.setStartDate(new Date(100));
    try{
      d.setEndDate(new Date(99));
      assertTrue(false);
    }catch (AdminException e){}

    d.setEndDate(new Date(101));
  }

  @Test
  public void testActivePeriod() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setActivePeriodStart(null);
      assertTrue(false);
    }catch (AdminException e){}

    d.setActivePeriodStart(new Date());

    try{
      d.setActivePeriodEnd(null);
      assertTrue(false);
    }catch (AdminException e){}
    d.setActivePeriodEnd(new Date(32132231));
  }

  @Test
  public void testActiveDays() throws AdminException {
    Delivery d = new Delivery();
    try{
      d.setActiveWeekDays(null);
      assertTrue(false);
    }catch (AdminException e){}

    try{
      d.setActiveWeekDays(new Delivery.Day[0]);
      assertTrue(false);
    }catch (AdminException e){}

    d.setActiveWeekDays(new Delivery.Day[]{Delivery.Day.Fri});
  }

  @Test
  public void testValidity() {
    Delivery d = new Delivery();
    d.setValidityDate(null);
    d.setValidityDate(new Date());
    d.setValidityPeriod(null);
    d.setValidityPeriod("");
    d.setValidityPeriod("dsadas");
    d.setValidityPeriod("23");
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
