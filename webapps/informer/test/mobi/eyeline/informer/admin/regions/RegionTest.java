package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import java.util.TimeZone;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class RegionTest {  

  private static Region createValidRegion() throws AdminException {
    Region r1 = new Region();
    r1.setMaxSmsPerSecond(200);
    r1.setName("MR SIBIR'");
    r1.setRegionId(123);
    r1.setSmsc("SMSC1");
    r1.setTimeZone(TimeZone.getDefault());
    r1.addMask(new Address("+7913948????"));
    r1.addMask(new Address("+7913949????"));

    r1.validate();
    return r1;
  }

  @Test
  public void testMasksOperations() throws AdminException {
    Region r1 = createValidRegion();

    r1.addMask(new Address("+79139495113"));
    assertTrue(r1.getMasks().contains(new Address("+79139495113")));

    r1.removeMask(new Address("+79139495113"));
    assertFalse(r1.getMasks().contains(new Address("+79139495113")));

    r1.addMask(new Address("+79139495113"));
    r1.clearMasks();
    assertEquals(0, r1.getMasks().size());

    Region r2 = createValidRegion();
    r1.addMasks(r2.getMasks());
    assertEquals(r1.getMasks().size(), r2.getMasks().size());
  }

  @Test
  public void testClone() throws AdminException{

    Region r1 = createValidRegion();

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

  @Test
  public void testAddNullMask() throws AdminException {
    Region r1 = createValidRegion();

    r1.addMask(null);
    assertEquals(2, r1.getMasks().size());
  }

}
