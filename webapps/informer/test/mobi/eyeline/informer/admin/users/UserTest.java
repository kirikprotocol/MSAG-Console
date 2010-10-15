package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.junit.Test;

import java.util.*;

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
  public void testSetOrganization() throws AdminException {
    User o = new User();
    o.setOrganization("1");
    assertEquals("1",o.getOrganization());
  }

  @Test
  public void testSetLocale() throws AdminException {
    User o = new User();
    o.setLocale(new Locale("en"));
    assertEquals(o.getLocale().getLanguage(),"en");
  }

  @Test
  public void testRoles() throws AdminException {
    User o = new User();
    TreeSet<String> roles = new TreeSet<String>();
    roles.add("roleA");
    roles.add("roleB");
    o.setRoles(roles);
    assertTrue(o.hasRole("roleA"));
    assertTrue(o.hasRole("roleB"));
    assertTrue(!o.hasRole("roleC"));
  }

  @Test
  public void testSetSmsPerSec() throws AdminException {
    User o = new User();
    o.setSmsPerSec(100);
    assertEquals(o.getSmsPerSec(),100);
    try{
      o.setSmsPerSec(0);
      assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setSmsPerSec(-100);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSourceAddr() throws AdminException {
    User o = new User();
    o.setSourceAddr(new Address("+79130000000"));
    assertEquals(o.getSourceAddr().getSimpleAddress(),"+79130000000");
    try{
      o.setSourceAddr(null);
      assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetDeliveryStartTime() throws AdminException {
    User o = new User();
    o.setDeliveryStartTime(new Time("09:30:15"));
    assertEquals(o.getDeliveryStartTime().getHour(),9);
    assertEquals(o.getDeliveryStartTime().getMin(),30);
    assertEquals(o.getDeliveryStartTime().getSec(),15);
    assertEquals(o.getDeliveryStartTime().getTimeString(),"09:30:15");
    o.setDeliveryStartTime(null);
  }

  @Test
  public void testSetDeliveryEndTime() throws AdminException {
    User o = new User();
    o.setDeliveryEndTime(new Time("08:03:07"));
    assertEquals(o.getDeliveryEndTime().getHour(),8);
    assertEquals(o.getDeliveryEndTime().getMin(),3);
    assertEquals(o.getDeliveryEndTime().getSec(),7);
    assertEquals(o.getDeliveryEndTime().getTimeString(),"08:03:07");
    o.setDeliveryEndTime(null);
    try{
      o.setDeliveryEndTime(new Time("24:00:00"));
      assertTrue(false);
    }catch (IllegalArgumentException e) {}
    try{
      o.setDeliveryEndTime(new Time("00:99:00"));
      assertTrue(false);
    }catch (IllegalArgumentException e) {}
    try{
      o.setDeliveryEndTime(new Time("00:00:60"));
      assertTrue(false);
    }catch (IllegalArgumentException e) {}
  }

  @Test
  public void testDeliveryDays() throws AdminException{

    User o = new User();
    Set<Integer> set = new HashSet<Integer>();
    set.add(0);
    set.add(3);
    set.add(6);
    o.setDeliveryDays(set);
    Set ret = o.getDeliveryDays();
    assertTrue(ret.size()==3);
    assertTrue(ret.contains(0));
    assertTrue(ret.contains(3));
    assertTrue(ret.contains(6));

    try {
      set.clear();
      set.add(-1);
      o.setDeliveryDays(set);
      assertTrue(false);
    }
    catch (AdminException e){}
    try {
      set.clear();
      set.add(7);
      o.setDeliveryDays(set);
      assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testValidHours() throws AdminException {
    User o = new User();
    o.setValidHours(10);
    assertEquals(o.getValidHours(),10);
    try {
      o.setValidHours(0);
      assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testTransactionMode() throws AdminException {
    User o = new User();
    o.setTransactionMode(true);
    assertEquals(o.isTransactionMode(),true);
  }

  @Test
  public void testPolicyId() throws AdminException {
    User o = new User();
    o.setPolicyId("blablabla");
    assertEquals(o.getPolicyId(),"blablabla");
    try {
      o.setPolicyId("");
      assertTrue(false);
    }
    catch (AdminException e){}
    try {
      o.setPolicyId(null);
      assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testRegion() throws AdminException {
    User o = new User();
    Set<String> regions = new TreeSet<String>();
    regions.add("fio");
    regions.add("fuo");
    regions.add("foe");
    o.setRegions(regions);
    assertEquals(o.getRegions(),regions);
    try {
      o.setRegions(null);
      assertTrue(false);
    }
    catch (AdminException e){}
    try {
      regions.add("");
      o.setRegions(regions);
      assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testDeliveryType() throws AdminException {
    User o = new User();
    o.setDeliveryType(User.DeliveryType.USSD_PUSH_VIA_VLR);
    assertEquals(o.getDeliveryType(),User.DeliveryType.USSD_PUSH_VIA_VLR);
    try {
      o.setDeliveryType(null);
      assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testPriority() throws AdminException {
    User o = new User();
    o.setPriority(1001);
    assertEquals(o.getPriority(),1001);
    try {
      o.setPriority(-5);
      assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testNotifications() throws AdminException {
    User o = new User();
    o.setSmsNotification(true);
    o.setEmailNotification(false);
    assertEquals(o.isSmsNotification(),true);
    assertEquals(o.isEmailNotification(),false);
  }

  @Test
  public void testDeliveryLifetime() throws AdminException {
    User o = new User();
    o.setDeliveryLifetime(333);
    assertEquals(o.getDeliveryLifetime(),333);
    try {
      o.setDeliveryLifetime(0);
      assertTrue(false);
    }
    catch (AdminException e){}

    o.setCreateArchive(true);
    assertEquals(o.isCreateArchive(),true);
  }

  @Test
  public void testDirectory() throws AdminException {
    User o = new User();
    o.setDirectory("aaaaaa");
    assertEquals(o.getDirectory(),"aaaaaa");
    try {
      o.setDirectory("");
      assertTrue(false);
    }
    catch (AdminException e){}
    try {
      o.setDirectory(null);
      assertTrue(false);
    }
    catch (AdminException e){}

    o.setDirectoryPoolPeriod(777);
    assertEquals(o.getDirectoryPoolPeriod(),777);
    try {
      o.setDirectoryPoolPeriod(0);
      assertTrue(false);
    }
    catch (AdminException e){}
    try {
      o.setDirectoryPoolPeriod(-1);
      assertTrue(false);
    }
    catch (AdminException e){}

    o.setCreateReports(true);
    assertEquals(o.isCreateReports(),true);

    o.setReportsLifetime(8888);
    assertEquals(o.getReportsLifetime(),8888);
    try {
      o.setReportsLifetime(0);
      assertTrue(false);
    }
    catch (AdminException e){}

    try {
      o.setReportsLifetime(-100);
      assertTrue(false);
    }
    catch (AdminException e){}
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
    o.setSmsPerSec(9);
    o.setCreateCDR(true);
    o.setSourceAddr(new Address("+79130000000"));


    User n = new User(o);
    UserTestUtils.compareUsers(o,n);
  }


}
