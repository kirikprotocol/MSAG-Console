package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.Collection;
import java.util.Iterator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class SmscManagerTest {

  private static File configFile, backupDir;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(SmscManagerTest.class.getResourceAsStream("smsc.xml"), ".smsc");
    backupDir = TestUtils.createRandomDir(".config.backup");
  }

  public SmscManager createManager(boolean errorMode) throws Exception {
    Infosme infosem = new TestInfosme(errorMode);
    return new SmscManager(infosem, configFile, backupDir, FileSystem.getFSForSingleInst());
//    for(Smsc s : smscManager.getSmscs()) {
//      infosem.addSmsc(s.getName());
//    }
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  @AfterClass
  public static void shutdown() {
    if(configFile != null) {
      configFile.delete();
    }
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }
  }

  @Test
  public void update() throws Exception {
    SmscManager smscManager = createManager(false);
    Smsc smsc = smscManager.getSmsc("SMSC0");
    assertTrue(smsc != null);
    assertEquals(smsc.getName(), "SMSC0");
    assertEquals(smsc.getHost(), "niagara");
    assertEquals(smsc.getInterfaceVersion(), 89);
    assertEquals(smsc.getPassword(), "infosme");
    assertEquals(smsc.getPort(), 25004);
    assertEquals(smsc.getSystemType(), "sysType");
    assertEquals(smsc.getUssdServiceOp(), 131);
    assertEquals(smsc.getVlrUssdServiceOp(), 153);
    assertEquals(smsc.getTimeout(), 1);
    assertEquals(smsc.getRangeOfAddress(), 2);

    smsc.setHost("niagara1");
    smsc.setInterfaceVersion(3113);
    smsc.setPassword("ewqeq");
    smsc.setPort(132);
    smsc.setSystemType("sT");
    smsc.setUssdServiceOp(46);
    smsc.setVlrUssdServiceOp(634);
    smsc.setSystemId("sid1");
    smsc.setTimeout(2);
    smsc.setRangeOfAddress(3);

    smscManager.updateSmsc(smsc);

    smscManager = createManager(false);

    Smsc s1 = smscManager.getSmsc(smsc.getName());
    assertTrue(s1.equals(smsc));
  }

  @Test
  public void addRemove() throws Exception {

    Smsc smsc = new Smsc("SMSC8");

    smsc.setHost("niagara1");
    smsc.setInterfaceVersion(3113);
    smsc.setPassword("ewqeq");
    smsc.setPort(132);
    smsc.setSystemType("sT");
    smsc.setUssdServiceOp(46);
    smsc.setVlrUssdServiceOp(634);
    smsc.setSystemId("sid2");
    smsc.setTimeout(2);
    smsc.setRangeOfAddress(3);

    SmscManager smscManager = createManager(false);
    smscManager.addSmsc(smsc);

    smscManager = createManager(false);

    Smsc s1 = smscManager.getSmsc(smsc.getName());
    assertTrue(s1.equals(smsc));
    smscManager.removeSmsc("SMSC8");

    smscManager = createManager(false);

    s1 = smscManager.getSmsc("SMSC8");
    assertTrue(s1 == null);
  }


  @Test
  public void testAddRollback() throws Exception {
    SmscManager smscManager = createManager(true);
    Smsc smsc = new Smsc("SMSC_ERROR");

    smsc.setHost("niagara1");
    smsc.setInterfaceVersion(3113);
    smsc.setPassword("ewqeq");
    smsc.setPort(132);
    smsc.setSystemType("sT");
    smsc.setUssdServiceOp(46);
    smsc.setVlrUssdServiceOp(634);
    smsc.setSystemId("sid2");
    smsc.setTimeout(2);
    smsc.setRangeOfAddress(3);

    long beforeLenght = configFile.length();
    try{
      smscManager.addSmsc(smsc);
      assertTrue(false);
    }catch (AdminException e){}

    long afterLenght = configFile.length();
    assertEquals(beforeLenght, afterLenght);
    assertEquals(smscManager.getSmsc(smsc.getName()), null);
  }

  @Test
  public void testRemoveRollback() throws Exception {
    SmscManager smscManager = createManager(true);
    Smsc smsc = smscManager.getSmscs().iterator().next();

    long beforeLenght = configFile.length();
    try{
      smscManager.removeSmsc(smsc.getName());
      assertTrue(false);
    }catch (AdminException e){}

    long afterLenght = configFile.length();
    assertEquals(beforeLenght, afterLenght);
    assertTrue(smscManager.getSmsc(smsc.getName()) != null);
  }

  @Test
  public void testUpdateRollback() throws Exception {
    SmscManager smscManager = createManager(true);

    Smsc smsc = smscManager.getSmscs().iterator().next();
    smsc.setHost(smsc.getHost()+"-");

    long beforeLenght = configFile.length();
    try{
      smscManager.updateSmsc(smsc);
      assertTrue(false);
    }catch (AdminException e){}

    long afterLenght = configFile.length();
    assertEquals(beforeLenght, afterLenght);
    assertTrue(smscManager.getSmsc(smsc.getName()) != null);
    assertEquals(smscManager.getSmsc(smsc.getName()).getHost(),smsc.getHost().substring(0, smsc.getHost().length()-1));
  }

  @Test
  public void testSetDefault() throws Exception {
    SmscManager smscManager = createManager(true);

    Collection<Smsc> smscs = smscManager.getSmscs();
    assertTrue(smscs.size() >= 2);
    Iterator<Smsc> i = smscs.iterator();

    Smsc s1 = i.next();
    Smsc s2 = i.next();

    String oldDefault = smscManager.getDefaultSmsc();
    String newDefault = s1.getName().equals(oldDefault) ? s2.getName() : s1.getName();

    long beforeLenght = configFile.length();
    try{
      smscManager.setDefaultSmsc(newDefault);
      assertTrue(false);
    }catch (AdminException e){}

    long afterLenght = configFile.length();
    assertEquals(beforeLenght, afterLenght);
    assertTrue(smscManager.getDefaultSmsc().equals(oldDefault));
  }


}
