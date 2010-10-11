package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import java.util.Collections;
import java.util.TimeZone;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class RegionsSettingTest {

  private Region region;

  private RegionsSettings settings = new RegionsSettings(Collections.<Region>emptyList());


  private static Region buildRegion() throws AdminException{
    Region r = new Region();
    r.setMaxSmsPerSecond(200);
    r.setName("MR SIBIR'");
    r.setSmsc("SMSC2");
    r.setTimeZone(TimeZone.getDefault());
    r.addMask(new Address("+791394899??"));
    r.addMask(new Address("+791394898??"));
    return r;
  }
  private static Region buildRegionWOMask() throws AdminException{
    Region r = new Region();
    r.setMaxSmsPerSecond(200);
    r.setName("MR SIBIR'");
    r.setSmsc("SMSC2");
    r.setTimeZone(TimeZone.getDefault());
    return r;
  }


  @Test
  public void addGetRemove() throws AdminException{
    Region r = buildRegion();
    settings.addRegion(r);
    assertEquals(r.getRegionId(), "0");
    Region r2 = buildRegion();
    try{
      settings.addRegion(r2);
      assertTrue(false);
    }catch (Exception e){}
    assertTrue(settings.getRegion(r.getRegionId()) != null);
    assertTrue(settings.getRegionByAddress(r.getMasks().iterator().next()) != null);
    assertTrue(!settings.getRegionsBySmsc(r.getSmsc()).isEmpty());
    settings.removeRegion(r.getRegionId());

    assertTrue(settings.getRegion(r.getRegionId()) == null);
    assertTrue(settings.getRegionByAddress(r.getMasks().iterator().next()) == null);
    assertTrue(settings.getRegionsBySmsc(r.getSmsc()).isEmpty());
  }

  @Test
  public void update() throws AdminException{
    Region r = buildRegion();
    settings.addRegion(r);
    assertTrue(settings.getRegion(r.getRegionId()) != null);


    r.setName("MR MOSCOW");
    settings.updateRegion(r);

    assertTrue(settings.getRegion(r.getRegionId()) != null);
    assertTrue(settings.getRegion(r.getRegionId()).getName().equals(r.getName()));


    Region r2 = buildRegion();
    try{
      settings.updateRegion(r2);
      assertTrue(false);
    }catch (Exception e){}

  }

  @Test
  public void getByMask() throws AdminException{
    Region r1 = buildRegionWOMask();
    r1.addMask(new Address("+7952???????"));
    settings.addRegion(r1);
    assertTrue(settings.getRegion(r1.getRegionId()) != null);
    Region r2 = buildRegionWOMask();
    r2.addMask(new Address("+795292?????"));
    settings.addRegion(r2);
    assertTrue(settings.getRegion(r2.getRegionId()) != null);
    Region r3 = buildRegionWOMask();
    r3.addMask(new Address("+79529??????"));
    settings.addRegion(r3);
    assertTrue(settings.getRegion(r3.getRegionId()) != null);

    Region r = settings.getRegionByAddress(new Address("+79529223755"));
    assertTrue(r != null);
    assertEquals(r.getRegionId(), r2.getRegionId());

    r = settings.getRegionByAddress(new Address("+79529323755"));
    assertTrue(r != null);
    assertEquals(r.getRegionId(), r3.getRegionId());


    r = settings.getRegionByAddress(new Address("+79539223755"));
    assertTrue(r == null);
  }





}
