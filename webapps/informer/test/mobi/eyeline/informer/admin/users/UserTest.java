package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * author: alkhal
 */
public class UserTest {


  @Test
  public void testSetCell() throws AdminException{
    User o = new User();
    o.setCellPhone("1");
    try{
      o.setCellPhone("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setCellPhone(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetDept() throws AdminException {
    User o = new User();
    o.setDept("1");
    try{
      o.setDept("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setDept(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetEmail() throws AdminException {
    User o = new User();
    o.setEmail("1");
    try{
      o.setEmail("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setEmail(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetFirstName() throws AdminException {
    User o = new User();
    o.setFirstName("1");
    try{
      o.setFirstName("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setFirstName(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetLastName() throws AdminException {
    User o = new User();
    o.setLastName("1");
    try{
      o.setLastName("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setLastName(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetHomePhone() throws AdminException {
    User o = new User();
    o.setHomePhone("1");
    try{
      o.setHomePhone("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setHomePhone(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetWorkPhone() throws AdminException {
    User o = new User();
    o.setWorkPhone("1");
    try{
      o.setWorkPhone("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setWorkPhone(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetPassword() throws AdminException {
    User o = new User();
    o.setPassword("1");
    try{
      o.setPassword("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setPassword(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetLogin() throws AdminException {
    User o = new User();
    o.setLogin("1");
    try{
      o.setLogin("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setLogin(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testClone() throws AdminException{
    User o = new User();
    o.setCellPhone("1");
    o.setDept("2");
    o.setEmail("3");
    o.setFirstName("4");
    o.setHomePhone("5");
    o.setLastName("6");
    o.setLogin("7");
    o.setPassword("8");
    o.setRoles(new HashSet<String>(){{add("1");add("2");}});
    o.setWorkPhone("31");
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
    o.setPrefs(oPr);

    User n = new User(o);
    assertEquals(o.getLogin(),n.getLogin());
    assertEquals(o.getCellPhone(), n.getCellPhone());
    assertEquals(o.getDept(), n.getDept());
    assertEquals(o.getEmail(), n.getEmail());
    assertEquals(o.getFirstName(), n.getFirstName());
    assertEquals(o.getHomePhone(), n.getHomePhone());
    assertEquals(o.getLastName(), n.getLastName());
    assertEquals(o.getPassword(), n.getPassword());
    assertEquals(o.getWorkPhone(), n.getWorkPhone());
    assertEquals(o.getRoles().size(), n.getRoles().size());
    for(String ro : o.getRoles()) {
      boolean foundR = false;
      for(String no : n.getRoles()) {
        if(no.equals(ro)) {
          foundR = true;
          break;
        }
      }
      assertTrue(foundR);
    }

    UserPreferences nPr = n.getPrefs();

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
