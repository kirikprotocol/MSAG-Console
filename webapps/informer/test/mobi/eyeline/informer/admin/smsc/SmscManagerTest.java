package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class SmscManagerTest {


  private static File configFile, backupDir;

  private SmscManager smscManager;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(SmscManagerTest.class.getResourceAsStream("smsc.xml"), ".smsc");
    backupDir = TestUtils.createRandomDir(".config.backup");
  }

  @Before
  public void before() throws Exception {
    Infosme infosem = new TestInfosme();
    smscManager = new SmscManager(infosem, configFile, backupDir, FileSystem.getFSForSingleInst());
    for(Smsc s : smscManager.getSmscs()) {
      infosem.addSmsc(s.getName());
    }
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

    smsc.setHost("niagara1");
    smsc.setInterfaceVersion(3113);
    smsc.setPassword("ewqeq");
    smsc.setPort(132);
    smsc.setSystemType("sT");
    smsc.setUssdServiceOp(46);
    smsc.setVlrUssdServiceOp(634);
    smsc.setSystemId("sid1");

    smscManager.updateSmsc(smsc);

    before();

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

    smscManager.addSmsc(smsc);

    before();

    Smsc s1 = smscManager.getSmsc(smsc.getName());
    assertTrue(s1.equals(smsc));
    smscManager.removeSmsc("SMSC8");

    before();

    s1 = smscManager.getSmsc("SMSC8");
    assertTrue(s1 == null);
  }


}
