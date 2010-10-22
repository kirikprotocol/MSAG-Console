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
    MessageFilter filter = new MessageFilter();
    try{
      filter.setStartDate(null);
      assertTrue(false);
    }catch (AdminException e){}
    filter.setStartDate(new Date());

    filter = new MessageFilter();
    try{
      filter.setEndDate(null);
      assertTrue(false);
    }catch (AdminException e){}

    filter.setEndDate(new Date());


    filter = new MessageFilter();
    filter.setEndDate(new Date(0));
    try{
      filter.setStartDate(new Date());
      assertTrue(false);
    }catch (AdminException e){}

    filter = new MessageFilter();
    filter.setStartDate(new Date());
    try{
      filter.setEndDate(new Date(0));
      assertTrue(false);
    }catch (AdminException e){}

    filter = new MessageFilter();
    filter.setEndDate(new Date());
    filter.setStartDate(new Date(0));
  }
}
