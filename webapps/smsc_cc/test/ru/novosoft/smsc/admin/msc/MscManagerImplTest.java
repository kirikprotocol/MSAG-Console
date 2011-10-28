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
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class MscManagerImplTest {

  private static File configFile;
  private MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void before() throws IOException, AdminException {
    configFile = fs.createNewFile("msc.bin", MscManagerImplTest.class.getResourceAsStream("msc.bin"));
  }


  @Test(expected = IllegalArgumentException.class)
  public void addNullMscTest() throws AdminException {
    MscManager m = new MscManagerImpl(configFile, new TestClusterControllerStub(), fs);
    m.addMsc(null);
  }

  @Test(expected = IllegalArgumentException.class)
  public void removeNullMscTest() throws AdminException {
    MscManager m = new MscManagerImpl(configFile, new TestClusterControllerStub(), fs);
    m.removeMsc(null);
  }

  @Test
  public void getMscsTest() throws AdminException {
    MscManager m = new MscManagerImpl(configFile, new TestClusterControllerStub(), fs);
    List<Address> mscs = new ArrayList<Address>(m.mscs());
    assertNotNull(mscs);
    assertEquals(3, mscs.size());
    assertTrue(mscs.contains(new Address("79139495113")));
    assertTrue(mscs.contains(new Address("6785")));
    assertTrue(mscs.contains(new Address("34242")));
  }

  @Test
  public void addMscTest() throws AdminException {
    MscManager m = new MscManagerImpl(configFile, new TestClusterControllerStub(), fs);
    m.addMsc(new Address("12345"));

    MscManager m1 = new MscManagerImpl(configFile, new TestClusterControllerStub(), fs);
    List<Address> mscs = new ArrayList<Address>(m1.mscs());
    assertEquals(4, mscs.size());
    assertTrue(mscs.contains(new Address("12345")));
  }

  @Test
  public void removeMscTest() throws AdminException {
    MscManager m = new MscManagerImpl(configFile, new TestClusterControllerStub(), fs);
    m.removeMsc(new Address("34242"));

    MscManager m1 = new MscManagerImpl(configFile, new TestClusterControllerStub(), fs);
    List<Address> mscs = new ArrayList<Address>(m1.mscs());
    assertEquals(2, mscs.size());
    assertFalse(mscs.contains(new Address("34242")));
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    MscManager m = new MscManagerImpl(configFile, new ClusterControllerImpl(), fs);

    Map<Integer, SmscConfigurationStatus> states = m.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    MscManager m = new MscManagerImpl(configFile, new ClusterControllerImpl1(), fs);

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
