package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import java.util.HashSet;
import java.util.Locale;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * author: alkhal
 */
public class UserTest {


  @Test
  public void testSetPhone() throws AdminException{
    User o = new User();
    o.setPhone("1");
    try{
      o.setPhone("");
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setPhone(null);
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
  public void testSetStatus() throws AdminException {
    User o = new User();
    o.setStatus(User.Status.ENABLED);
    try{
      o.setStatus(null);
      assertTrue(false);
    }
    catch (AdminException e) {}
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
    o.setPhone("1");
    o.setOrganization("2");
    o.setEmail("3");
    o.setFirstName("4");
    o.setLastName("5");
    o.setLogin("6");
    o.setPassword("7");
    o.setStatus(User.Status.ENABLED);
    o.setLocale(new Locale("en"));
    o.setRoles(new HashSet<String>(){{add("1");add("2");}});

    User n = new User(o);
    assertEquals(o.getLogin(),n.getLogin());
    assertEquals(o.getPhone(), n.getPhone());
    assertEquals(o.getOrganization(), n.getOrganization());
    assertEquals(o.getEmail(), n.getEmail());
    assertEquals(o.getFirstName(), n.getFirstName());
    assertEquals(o.getLastName(), n.getLastName());
    assertEquals(o.getPassword(), n.getPassword());
    assertEquals(o.getStatus(), n.getStatus());
    assertEquals(o.getLocale(), n.getLocale());
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

  }

}
