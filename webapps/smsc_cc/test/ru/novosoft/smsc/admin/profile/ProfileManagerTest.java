package ru.novosoft.smsc.admin.profile;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
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
public class ProfileManagerTest {
  private File configFile, backupDir;
  private MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = fs.createNewFile("profiles.bin", ProfileManagerTest.class.getResourceAsStream("profiles.bin"));
    backupDir = fs.mkdirs("backup");
  }

  public ProfileManager getManager(ClusterController cc) throws AdminException {
    return new ProfileManagerImpl(false, configFile, fs, cc);
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    ProfileManager manager = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    ProfileManager manager = getManager(new ClusterControllerImpl1());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getProfilesState() throws AdminException {
      long now = fs.lastModified(configFile);
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getProfilesState() throws AdminException {
      return null;
    }
  }
}
