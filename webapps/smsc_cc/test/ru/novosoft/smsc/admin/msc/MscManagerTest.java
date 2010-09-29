package ru.novosoft.smsc.admin.msc;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class MscManagerTest {

  private static File configFile;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(MscManagerTest.class.getResourceAsStream("msc.bin"), ".msc");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
  }

  @Test(expected = IllegalArgumentException.class)
  public void addNullMscTest() throws AdminException {
    MscManager m = new MscManager(configFile, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    m.addMsc(null);
  }

  @Test(expected = IllegalArgumentException.class)
  public void removeNullMscTest() throws AdminException {
    MscManager m = new MscManager(configFile, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    m.removeMsc(null);
  }

  @Test
  public void getMscsTest() throws AdminException {
    MscManager m = new MscManager(configFile, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    List<Address> mscs = new ArrayList<Address>(m.mscs());
    assertNotNull(mscs);
    assertEquals(3, mscs.size());
    assertEquals(new Address("79139495113"), mscs.get(0));
    assertEquals(new Address("6785"), mscs.get(1));
    assertEquals(new Address("34242"), mscs.get(2));
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    MscManager m = new MscManager(configFile, new ClusterControllerImpl(), FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = m.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    MscManager m = new MscManager(configFile, new ClusterControllerImpl1(), FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = m.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getMscConfigState() throws AdminException {
      long now = System.currentTimeMillis();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getMscConfigState() throws AdminException {
      return null;
    }
  }
}
