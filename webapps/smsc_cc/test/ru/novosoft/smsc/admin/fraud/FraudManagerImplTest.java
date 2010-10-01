package ru.novosoft.smsc.admin.fraud;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

/**
 * @author Artem Snopkov
 */
public class FraudManagerImplTest {

  private File configFile, backupDir;

  @Before
  public void before() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(FraudManagerImplTest.class.getResourceAsStream("fraud.xml"), ".fraud");
    backupDir = TestUtils.createRandomDir(".fraudbackup");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private FraudManagerImpl getManager(ClusterController cc) throws AdminException {
    FraudManagerImpl fm = new FraudManagerImpl(configFile, backupDir, cc, FileSystem.getFSForSingleInst());
    return fm;
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    FraudManagerImpl fm = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = fm.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    FraudManagerImpl fm = getManager(new ClusterControllerImpl1());

    Map<Integer, SmscConfigurationStatus> states = fm.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getFraudConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getFraudConfigState() throws AdminException {
      return null;
    }
  }


}