package mobi.eyeline.informer.admin.retry_policies;

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

import static org.junit.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 18:43:45
 */
public class RetryPolicyManagerTest {
  private static File configFile, backupDir;

  private RetryPolicyManager manager;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(RetryPolicyManagerTest.class.getResourceAsStream("policies.xml"), ".smsc");
    backupDir = TestUtils.createRandomDir(".config.backup");
  }

  @Before
  public void before() throws Exception {
    Infosme infosem = new TestInfosme();
    manager = new RetryPolicyManager(infosem, configFile, backupDir, FileSystem.getFSForSingleInst());    
    for(RetryPolicy s : manager.getRetryPolicies()) {
      infosem.addSmsc(s.getPolicyId());
    }
  }
  
  @Test
  public void testLoad() throws AdminException {
    RetryPolicy rp = manager.getRetryPolicy("default");
    assertTrue(rp!=null);
    assertTrue(rp.getDefaultTime()==601);
    assertTrue(rp.getEntries().size()==1);
    assertTrue(rp.getEntries().get(0).getErrCode()==1025);
    assertTrue(rp.getEntries().get(0).getTime()==500);
    
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
}
