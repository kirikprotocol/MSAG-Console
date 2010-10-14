package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscTest {


  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testTimeout() throws AdminException{
    Smsc smsc = new Smsc("smsc1");
    try{
      smsc.setTimeout(0);
      assertTrue(false);
    }catch (AdminException e){}
    try{
      smsc.setTimeout(-1);
      assertTrue(false);
    }catch (AdminException e){}

    smsc.setTimeout(234);
    assertEquals(234, smsc.getTimeout());
  }

  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testRangeOfAddress() throws AdminException{
    Smsc smsc = new Smsc("smsc1");
    smsc.setRangeOfAddress(234);
    assertEquals(234, smsc.getRangeOfAddress());
  }

  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testName() throws AdminException{
    Smsc smsc;
    try{
      smsc = new Smsc("");
      assertTrue(false);
    }catch (AdminException e){}
    try{
      smsc = new Smsc(null);
      assertTrue(false);
    }catch (AdminException e){}
    smsc = new Smsc("SMSC0");
  }

  @Test
  public void testHost() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setHost("");
      assertTrue(false);
    }catch (AdminException e){}

    try{
      smsc.setHost(null);
      assertTrue(false);
    }catch (AdminException e){}

    smsc.setHost("niagara");

  }

  @Test
  public void setPort() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setPort(-1);
      assertTrue(false);
    }catch (AdminException e) {}

    try{
      smsc.setPort(Integer.MAX_VALUE);
      assertTrue(false);
    }catch (AdminException e) {}

    smsc.setPort(8080);
  }

  public void setSystemId() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setSystemId("");
      assertTrue(false);
    }catch (AdminException e){}

    try{
      smsc.setSystemId(null);
      assertTrue(false);
    }catch (AdminException e){}

    smsc.setSystemId("sid");
  }

  public void setSystemType() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setSystemType("");
      assertTrue(false);
    }catch (AdminException e){}

    try{
      smsc.setSystemType(null);
      assertTrue(false);
    }catch (AdminException e){}

    smsc.setSystemType("sType");
  }

  public void setPassword() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");
    try{
      smsc.setPassword(null);
      assertTrue(false);
    }catch (AdminException e){}
    smsc.setPassword("");
    smsc.setPassword("qwerty");
  }



  @Test
  public void setInterfaceVersion() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setInterfaceVersion(-1);
      assertTrue(false);
    }catch (AdminException e) {}

    smsc.setInterfaceVersion(Integer.MAX_VALUE);
    smsc.setInterfaceVersion(123);
  }


  @Test
  public void setUssdServiceOp() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setUssdServiceOp(-1);
      assertTrue(false);
    }catch (AdminException e) {}

    smsc.setUssdServiceOp(Integer.MAX_VALUE);
    smsc.setUssdServiceOp(123);
  }

  @Test
  public void setVlrUssdServiceOp() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setVlrUssdServiceOp(-1);
      assertTrue(false);
    }catch (AdminException e) {}

    smsc.setVlrUssdServiceOp(Integer.MAX_VALUE);
    smsc.setVlrUssdServiceOp(123);
  }

  @Test
  public void cloneTest() throws AdminException {
    Smsc s1 = new Smsc("name1");
    s1.setHost("dsa");
    s1.setPort(123);
    s1.setPassword("qwerty");
    s1.setInterfaceVersion(2);
    s1.setSystemId("sid1");
    s1.setSystemType("sType");
    s1.setUssdServiceOp(12);
    s1.setVlrUssdServiceOp(562);
    s1.setTimeout(12);
    s1.setRangeOfAddress(34);
    Smsc s2 = s1.cloneSmsc();
    assertTrue(s1 != s2);
    assertEquals(s1, s2);
  }

}
