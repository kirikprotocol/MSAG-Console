package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import java.util.TimeZone;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class RegionTest {


  public void setName() throws AdminException {
    Region r1 = new Region();
    try{
      r1.setName("");
      assertTrue(false);
    }catch (AdminException e){}
    try{
      r1.setName(null);
      assertTrue(false);
    }catch (AdminException e){}

    r1.setName("name1");
  }

  public void setSmsc() throws AdminException {
    Region r1 = new Region();
    try{
      r1.setSmsc("");
      assertTrue(false);
    }catch (AdminException e){}
    try{
      r1.setSmsc(null);
      assertTrue(false);
    }catch (AdminException e){}

    r1.setSmsc("smsc1");
  }

  @Test
  public void addMask() throws AdminException {
    Region r1 = new Region();
    try{
      r1.addMask(null);
      assertTrue(false);
    }catch (AdminException e){}

    try{
      r1.addMask(new Address("+7913???9906"));
      assertTrue(false);
    }catch (Exception e){}

    r1.addMask(new Address("+7913948990?"));
    assertEquals(r1.getMasks().size(), 1);
  }

  @Test
  public void setMaxSmsPerSecond() throws AdminException {
    Region r1 = new Region();
    try{
      r1.setMaxSmsPerSecond(0);
      assertTrue(false);
    }catch (AdminException e){}

    try{
      r1.setMaxSmsPerSecond(-1);
      assertTrue(false);
    }catch (AdminException e){}

    r1.setMaxSmsPerSecond(1);
  }

  @Test
  public void setTimezone() throws AdminException {
    Region r1 = new Region();
    try{
      r1.setTimeZone(null);
      assertTrue(false);
    }catch (AdminException e){}

    r1.setTimeZone(TimeZone.getDefault());
  }


  @Test
  public void cloneTest() throws AdminException{
    Region r1 = new Region();
    r1.setMaxSmsPerSecond(200);
    r1.setName("MR SIBIR'");
    r1.setRegionId("123");
    r1.setSmsc("SMSC1");
    r1.setTimeZone(TimeZone.getDefault());
    r1.addMask(new Address("+7913948????"));
    r1.addMask(new Address("+7913949????"));

    Region r2 = r1.cloneRegion();
    assertTrue(r1 != r2);
    assertEquals(r1.getRegionId(), r2.getRegionId());
    assertEquals(r1.getName(), r2.getName());
    assertEquals(r1.getMaxSmsPerSecond(), r2.getMaxSmsPerSecond());
    assertEquals(r1.getSmsc(), r2.getSmsc());
    assertEquals(r1.getTimeZone(), r2.getTimeZone());
    assertEquals(r1.getMasks().size(), r2.getMasks().size());
    for(Address a1 : r1.getMasks()) {
      boolean error = true;
      for(Address a2:r2.getMasks()) {
        if(a1.getSimpleAddress().equals(a2.getSimpleAddress())) {
          error = false;
          break;
        }
      }
      assertTrue(!error);
    }
  }

}
