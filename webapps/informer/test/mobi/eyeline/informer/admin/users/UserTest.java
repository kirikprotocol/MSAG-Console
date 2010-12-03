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
    User o = createUser();;
    try{
      o.setPhone("");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setPhone(null);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setPhone("dasdasdasdasda");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetEmail() throws AdminException {
    User o = createUser();;
    try{
      o.setEmail("");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setEmail(null);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setEmail("dasdasdasdasda");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetFirstName() throws AdminException {
    User o = createUser();;
    try{
      o.setFirstName("");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setFirstName(null);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetLastName() throws AdminException {
    User o = createUser();;
    try{
      o.setLastName("");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setLastName(null);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetStatus() throws AdminException {
    User o = createUser();;
    try{
      o.setStatus(null);
      o.validate(); assertTrue(false);
    }
    catch (AdminException e) {}
  }

  @Test
  public void testSetPassword() throws AdminException {
    User o = createUser();;
    try{
      o.setPassword("");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setPassword(null);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetLogin() throws AdminException {
    User o = createUser();;
    try{
      o.setLogin("");
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setLogin(null);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetLocale() throws AdminException {
    User o = createUser();;
    o.setLocale(new Locale("en"));
    assertEquals(o.getLocale().getLanguage(),"en");
  }

  @Test
  public void testRoles() throws AdminException {
    User o = createUser();;
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
    User o = createUser();;
    try{
      o.setSmsPerSec(0);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
    try{
      o.setSmsPerSec(-100);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSourceAddr() throws AdminException {
    User o = createUser();;
    try{
      o.setSourceAddr(null);
      o.validate(); assertTrue(false);
    }catch (AdminException e) {}
  }

  @Test
  public void testSetDeliveryStartTime() throws AdminException {
    User o = createUser();;
    o.setDeliveryStartTime(new Time("09:30:15"));
    assertEquals(o.getDeliveryStartTime().getHour(),9);
    assertEquals(o.getDeliveryStartTime().getMin(),30);
    assertEquals(o.getDeliveryStartTime().getSec(),15);
    assertEquals(o.getDeliveryStartTime().getTimeString(),"09:30:15");
    o.setDeliveryStartTime(null);
  }

  @Test
  public void testSetDeliveryEndTime() throws AdminException {
    User o = createUser();;
    o.setDeliveryEndTime(new Time("08:03:07"));
    assertEquals(o.getDeliveryEndTime().getHour(),8);
    assertEquals(o.getDeliveryEndTime().getMin(),3);
    assertEquals(o.getDeliveryEndTime().getSec(),7);
    assertEquals(o.getDeliveryEndTime().getTimeString(),"08:03:07");
    o.setDeliveryEndTime(null);
    try{
      o.setDeliveryEndTime(new Time("24:00:00"));
      o.validate(); assertTrue(false);
    }catch (IllegalArgumentException e) {}
    try{
      o.setDeliveryEndTime(new Time("00:99:00"));
      o.validate(); assertTrue(false);
    }catch (IllegalArgumentException e) {}
    try{
      o.setDeliveryEndTime(new Time("00:00:60"));
      o.validate(); assertTrue(false);
    }catch (IllegalArgumentException e) {}
  }

  @Test
  public void testDeliveryDays() throws AdminException{

    User o = createUser();;
    List<Integer> set = new ArrayList<Integer>();
    set.add(0);
    set.add(3);
    set.add(6);
    o.setDeliveryDays(set);
    List ret = o.getDeliveryDays();
    assertTrue(ret.size()==3);
    assertTrue(ret.contains(0));
    assertTrue(ret.contains(3));
    assertTrue(ret.contains(6));

    try {
      set.clear();
      set.add(-1);
      o.setDeliveryDays(set);
      o.validate(); assertTrue(false);
    }
    catch (AdminException e){}
    try {
      set.clear();
      set.add(7);
      o.setDeliveryDays(set);
      o.validate(); assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testValidHours() throws AdminException {
    User o = createUser();;
    try {
      o.setValidityPeriod(new Time(0,50,30));
      o.validate(); assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testTransactionMode() throws AdminException {
    User o = createUser();;
    o.setTransactionMode(true);
    assertEquals(o.isTransactionMode(),true);
  }

  @Test
  public void testPolicyId() throws AdminException {
    User o = createUser();;
    o.setPolicyId("blablabla");
    assertEquals(o.getPolicyId(),"blablabla");
    o.setPolicyId(null);
    assertTrue(o.getPolicyId()==null);
  }

  @Test
  public void testRegion() throws AdminException {
    User o = createUser();
    List<Integer> regions = new ArrayList<Integer>();
    regions.add(1);
    regions.add(2);
    regions.add(3);
    o.setRegions(regions);
    assertEquals(o.getRegions(),regions);

    o.setRegions(null);
    assertTrue(o.getRegions()==null);
       
    try {
      regions.add(null);
      o.setRegions(regions);
      o.validate(); assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testDeliveryType() throws AdminException {
    User o = createUser();;
    try {
      o.setDeliveryType(null);
      o.validate(); assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testPriority() throws AdminException {
    User o = createUser();;
    o.setPriority(1001);
    assertEquals(o.getPriority(),1001);
    try {
      o.setPriority(-5);
      o.validate(); assertTrue(false);
    }
    catch (AdminException e){}
  }

  @Test
  public void testNotifications() throws AdminException {
    User o = createUser();;
    o.setSmsNotification(true);
    o.setEmailNotification(false);
    assertEquals(o.isSmsNotification(),true);
    assertEquals(o.isEmailNotification(),false);
  }

  @Test
  public void testDeliveryLifetime() throws AdminException {
    User o = createUser();;
    o.setDeliveryLifetime(333);
    assertEquals(o.getDeliveryLifetime(),333);
    try {
      o.setDeliveryLifetime(0);
      o.validate(); assertTrue(false);
    }
    catch (AdminException e){}

    o.setCreateArchive(true);
    assertEquals(o.isCreateArchive(),true);
  }



  @Test
  public void testCPSettings() throws AdminException {
    User o = createUser();;
    List<UserCPsettings> cpSettings = new ArrayList<UserCPsettings>();
    UserCPsettings ucps = new UserCPsettings();
    ucps.setProtocol(UserCPsettings.Protocol.sftp);
    ucps.setDirectory("/home/bla");
    ucps.setHost("some.bla.host");
    ucps.setPort(2222);
    ucps.setSourceAddress(new Address("+79130000111"));
    ucps.setEncoding("UTF-8");
    ucps.setLogin("user1");
    ucps.setPassword("pwd1");

    UserCPsettings ucpsn = new UserCPsettings(ucps);
    assertEquals(ucps,ucpsn);

    ucps.setProtocol(UserCPsettings.Protocol.file);
    ucps.setDirectory("/some/dir");
    ucps.setHost(null);
    ucps.setPort(null);
    ucpsn.setSourceAddress(new Address("+79130000222"));
    ucps.setEncoding("cp1251");
    ucps.setLogin(null);
    ucps.setPassword(null);

    cpSettings.add(ucpsn);

    o.setCpSettings(cpSettings);



    assertEquals(o.getCpSettings(),cpSettings);

  }


  private User createUser() {
    User o = new User();
    o.setPhone("1");
    o.setOrganization("2");
    o.setEmail("3@dasdsa.re");
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
    o.setCreateCDR(true);
    o.setCdrDestination("dest");
    o.setCdrOriginator("orig");
    o.setDeliveryType(User.DeliveryType.SMS);
    o.setDeliveryLifetime(1);
    o.setPriority(1);
    o.setDeliveryStartTime(new Time(1,1,1));
    o.setDeliveryEndTime(new Time(21,1,1));
    return o;
  }


  @Test
  public void testCloneAndValdiate() throws AdminException{
    User o = createUser();
    List<UserCPsettings> cpSettings = new ArrayList<UserCPsettings>();
    UserCPsettings ucps = new UserCPsettings();
    ucps.setProtocol(UserCPsettings.Protocol.sftp);
    ucps.setDirectory("/home/bla");
    ucps.setHost("some.bla.host");
    ucps.setPort(2222);
    ucps.setSourceAddress(new Address("+79130000111"));
    ucps.setEncoding("UTF-8");

    UserCPsettings ucpsn = new UserCPsettings(ucps);
    assertEquals(ucps,ucpsn);

    ucps.setProtocol(UserCPsettings.Protocol.file);
    ucps.setDirectory("/home/fio");
    ucps.setHost("some.fio.host");
    ucps.setPort(3333);       
    ucpsn.setSourceAddress(new Address("+79130000222"));
    ucps.setEncoding("cp1251");
    cpSettings.add(ucpsn);

    o.setCpSettings(cpSettings);

    o.validate();

    User n = new User(o);

    n.validate();
    
    UserTestUtils.compareUsers(o,n);


  }


}
