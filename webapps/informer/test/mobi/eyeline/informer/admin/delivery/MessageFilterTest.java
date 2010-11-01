package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import java.util.Date;

import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class MessageFilterTest {


  @Test
  public void testDates() throws AdminException{
    try{
      new MessageFilter(null,new Date(), new Date());
      assertTrue(false);
    }catch (Exception e){}

    try{
       new MessageFilter(1, null, new Date());
      assertTrue(false);
    }catch (Exception e){}

    try{
      new MessageFilter(1, new Date(), null);        
      assertTrue(false);
    }catch (Exception e){}

    MessageFilter filter = new MessageFilter(1, new Date(), new Date());
    try{
      filter.setStartDate(null);
      assertTrue(false);
    }catch (AdminException e){}
    filter.setStartDate(new Date());

    filter = new MessageFilter(1, new Date(), new Date());
    try{
      filter.setEndDate(null);
      assertTrue(false);
    }catch (AdminException e){}

    filter = new MessageFilter(1, new Date(), new Date());
    filter.setEndDate(new Date());
    filter.setStartDate(new Date(0));
  }
}
