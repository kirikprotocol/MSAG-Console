package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * author: alkhal
 */
public class UserPreferencesTest {

  @Test
  public void testSetTopMonMaxSpeed() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setTopMonMaxSpeed(1);
    try{
      o.setTopMonMaxSpeed(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetTopMonGraphScale() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setTopMonGraphScale(1);
    try{
      o.setTopMonGraphScale(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetTopMonGraphHigrid() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setTopMonGraphHigrid(1);
    try{
      o.setTopMonGraphHigrid(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetTopMonGraphHead() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setTopMonGraphHead(1);
    try{
      o.setTopMonGraphHead(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetTopMonGraphGrid() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setTopMonGraphGrid(1);
    try{
      o.setTopMonGraphGrid(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetPerfMonVMinuteGrid() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setPerfMonVMinuteGrid(1);
    try{
      o.setPerfMonVMinuteGrid(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetPerfMonVLightGrid() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setPerfMonVLightGrid(1);
    try{
      o.setPerfMonVLightGrid(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetPerfMonScale() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setPerfMonScale(1);
    try{
      o.setPerfMonScale(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetPerfMonPixPerSecond() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setPerfMonPixPerSecond(1);
    try{
      o.setPerfMonPixPerSecond(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }
  @Test
  public void testSetPerfMonBlock() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setPerfMonBlock(1);
    try{
      o.setPerfMonBlock(0);
      assertTrue(false);
    }catch (AdminException e) {}
  }@Test

  public void testSetLocale() throws AdminException {
    UserPreferences o = new UserPreferences();
    o.setLocale(new Locale("en"));
    try{
      o.setLocale(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }


  @Test
  public void testClone() throws AdminException{
    UserPreferences oPr = new UserPreferences();
    oPr.setLocale(new Locale("ru"));
    oPr.setPerfMonBlock(1);
    oPr.setPerfMonPixPerSecond(2);
    oPr.setPerfMonScale(3);
    oPr.setPerfMonVLightGrid(4);
    oPr.setPerfMonVMinuteGrid(5);
    oPr.setTopMonGraphGrid(6);
    oPr.setTopMonGraphHead(7);
    oPr.setTopMonGraphHigrid(8);
    oPr.setTopMonGraphScale(9);
    oPr.setTopMonMaxSpeed(10);
    oPr.setUnknown(new HashMap<String, String>(){{put("1","2");put("3","4");}});
    UserPreferences nPr = new UserPreferences(oPr);

    assertEquals(oPr.getLocale(),nPr.getLocale());
    assertEquals(oPr.getPerfMonBlock(),nPr.getPerfMonBlock());
    assertEquals(oPr.getPerfMonPixPerSecond(),nPr.getPerfMonPixPerSecond());
    assertEquals(oPr.getPerfMonScale(),nPr.getPerfMonScale());
    assertEquals(oPr.getPerfMonVLightGrid(),nPr.getPerfMonVLightGrid());
    assertEquals(oPr.getTopMonGraphGrid(),nPr.getTopMonGraphGrid());
    assertEquals(oPr.getPerfMonVMinuteGrid(),nPr.getPerfMonVMinuteGrid());
    assertEquals(oPr.getTopMonGraphHead(),nPr.getTopMonGraphHead());
    assertEquals(oPr.getTopMonGraphScale(),nPr.getTopMonGraphScale());
    assertEquals(oPr.getTopMonMaxSpeed(),nPr.getTopMonMaxSpeed());
    assertEquals(oPr.getTopMonMaxSpeed(),nPr.getTopMonMaxSpeed());
    assertEquals(oPr.getTopMonGraphHigrid(),nPr.getTopMonGraphHigrid());
    assertEquals(oPr.getUnknown().size(), nPr.getUnknown().size());

    for(Map.Entry<String, String> uo : oPr.getUnknown().entrySet()) {
      boolean foundR = false;
      for(Map.Entry<String, String> un : nPr.getUnknown().entrySet()) {
        if(uo.equals(un)) {
          foundR = true;
          break;
        }
      }
      assertTrue(foundR);
    }




  }
}
