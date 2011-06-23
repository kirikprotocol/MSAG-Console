package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Before;
import org.junit.Test;

import java.util.HashSet;
import java.util.Set;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscTest {
  
  private Smsc smsc;
  
  
  @Before
  public void before() throws AdminException {
    smsc = new Smsc("name1");
    smsc.setHost("dsa");
    smsc.setPort(123);
    smsc.setPassword("qwerty");
    smsc.setInterfaceVersion(2);
    smsc.setSystemId("sid1");
    smsc.setSystemType("sType");
    smsc.setUssdServiceOp(12);
    smsc.setVlrUssdServiceOp(562);
    smsc.setTimeout(12);
    smsc.setRangeOfAddress(34);
    smsc.addImmediateError(1254);
    smsc.addPermanentError(1255);
    smsc.addTempError("1m", new HashSet<Integer>(){{add(1453);}});
  }


  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testImmedError() throws AdminException{
    Integer i = 1;
    smsc.addImmediateError(i);
    try{
      smsc.addImmediateError(i);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    before();
    smsc.addImmediateError(i);
    assertTrue( smsc.getImmediateErrors().contains(i));
    smsc.removeImmediateError(i);
    assertFalse( smsc.getImmediateErrors().contains(i));
  }

  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testPermdError() throws AdminException{
    Integer i = 1;
    smsc.addPermanentError(i);
    try{
      smsc.addPermanentError(i);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    before();
    smsc.addPermanentError(i);
    assertTrue( smsc.getPermanentErrors().contains(i));
    smsc.removePermanentError(i);
    assertFalse( smsc.getPermanentErrors().contains(i));
  }

  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testTempError() throws AdminException{
    
    Set<Integer> errors = new HashSet<Integer>();
    errors.add(1202);
    try{
      smsc.addTempError("blablabla", errors);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    try{
      smsc.addTempError("", errors);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    try{
      smsc.addTempError(null, errors);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    smsc.addTempError("1m", errors);       
    assertTrue( smsc.getTemporaryErrors().containsKey("1m"));
    assertTrue( smsc.getTemporaryErrors().get("1m").contains(1202));
  }

  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testTimeout() throws AdminException{
    
    try{
      smsc.setTimeout(0);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    try{
      smsc.setTimeout(-1);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    smsc.setTimeout(234);
    assertEquals(234, smsc.getTimeout());
  }

  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testRangeOfAddress() throws AdminException{
    
    smsc.setRangeOfAddress(234);
    assertEquals(234, smsc.getRangeOfAddress());
  }

  @SuppressWarnings({"UnusedAssignment"})
  @Test
  public void testName() throws AdminException{
    Smsc smsc;
    try{
      smsc = new Smsc("");
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    try{
      smsc = new Smsc(null);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    smsc = new Smsc("SMSC0");
  }

  @Test
  public void testHost() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setHost("");
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    try{
      smsc.setHost(null);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    smsc.setHost("niagara");

  }

  @Test
  public void setPort() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setPort(-1);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e) {}

    try{
      smsc.setPort(Integer.MAX_VALUE);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e) {}

    smsc.setPort(8080);
  }

  public void setSystemId() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setSystemId("");
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    try{
      smsc.setSystemId(null);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    smsc.setSystemId("sid");
  }

  public void setSystemType() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setSystemType("");
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    try{
      smsc.setSystemType(null);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}

    smsc.setSystemType("sType");
  }

  public void setPassword() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");
    try{
      smsc.setPassword(null);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e){}
    smsc.setPassword("");
    smsc.setPassword("qwerty");
  }



  @Test
  public void setInterfaceVersion() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setInterfaceVersion(-1);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e) {}

    smsc.setInterfaceVersion(Integer.MAX_VALUE);
    smsc.setInterfaceVersion(123);
  }


  @Test
  public void setUssdServiceOp() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setUssdServiceOp(-1);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e) {}

    smsc.setUssdServiceOp(Integer.MAX_VALUE);
    smsc.setUssdServiceOp(123);
  }

  @Test
  public void setVlrUssdServiceOp() throws AdminException{
    Smsc smsc = new Smsc("SMSC0");

    try{
      smsc.setVlrUssdServiceOp(-1);
      smsc.validate(); assertTrue(false);
    }catch (AdminException e) {}

    smsc.setVlrUssdServiceOp(Integer.MAX_VALUE);
    smsc.setVlrUssdServiceOp(123);
  }

  @Test
  public void validateAndCloneTest() throws AdminException {
    Smsc smsc = new Smsc("name1");
    smsc.setHost("dsa");
    smsc.setPort(123);
    smsc.setPassword("qwerty");
    smsc.setInterfaceVersion(2);
    smsc.setSystemId("sid1");
    smsc.setSystemType("sType");
    smsc.setUssdServiceOp(12);
    smsc.setVlrUssdServiceOp(562);
    smsc.setTimeout(12);
    smsc.setRangeOfAddress(34);
    smsc.addImmediateError(1254);
    smsc.addPermanentError(1255);
    smsc.addTempError("1m", new HashSet<Integer>(){{add(1453);}});
    smsc.validate();
    
    Smsc s2 = smsc.cloneSmsc();
    s2.validate();
    
    assertTrue(smsc != s2);
    assertEquals(smsc, s2);
  }

}
