package ru.novosoft.smsc.admin.alias;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class AliasManagerImplTest {

  private static File aliasesFile;

  @BeforeClass
  public static void beforeClass() throws IOException {
    aliasesFile = TestUtils.exportResourceToRandomFile(AliasManagerImplTest.class.getResourceAsStream("aliases.bin"), ".aliases");
  }

  @AfterClass
  public static void afterClass() {
    if (aliasesFile != null)
      aliasesFile.delete();
  }

  @Test
  public void testGetAliases() throws Exception {
    ClusterController cc = new TestClusterControllerStub();
    AliasManagerImpl manager = new AliasManagerImpl(aliasesFile, cc, FileSystem.getFSForSingleInst());

    AliasSet set = null;
    try {
      set = manager.getAliases();

      assertTrue(set.next());

      Alias alias1 = set.get();
      assertEquals(alias1.getAddress(), new Address(0, 1, "31"));
      assertEquals(alias1.getAlias(), new Address(0, 1, "123"));
      assertEquals(alias1.isHide(), false);

      assertTrue(set.next());

      Alias alias2 = set.get();
      assertEquals(alias2.getAddress(), new Address(0, 1, "112233"));
      assertEquals(alias2.getAlias(), new Address(0, 1, "332211"));
      assertEquals(alias2.isHide(), true);

      assertTrue(set.next());

      Alias alias3 = set.get();
      assertEquals(alias3.getAddress(), new Address(0, 1, "223344"));
      assertEquals(alias3.getAlias(), new Address(0, 1, "443322"));
      assertEquals(alias3.isHide(), false);

      assertFalse(set.next());

    } finally {
      if (set != null)
        set.close();
    }
  }

  @Test
  public void testAddAlias() throws AdminException {
    TestClusterControllerStub cc = new TestClusterControllerStub();
    AliasManagerImpl manager = new AliasManagerImpl(aliasesFile, cc, FileSystem.getFSForSingleInst());

    long time = cc.getAliasLastUpdateTime();

    manager.addAlias(new Alias("+79139495113", "1223", false));

    assertTrue(cc.getAliasLastUpdateTime() > time);
  }

  @Test
  public void testRemoveAlias() throws AdminException {
    TestClusterControllerStub cc = new TestClusterControllerStub();
    AliasManagerImpl manager = new AliasManagerImpl(aliasesFile, cc, FileSystem.getFSForSingleInst());

    long time = cc.getAliasLastUpdateTime();

    manager.deleteAlias(new Address("123"));

    assertTrue(cc.getAliasLastUpdateTime() >= time);
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    ClusterController cc = new ClusterControllerImpl();
    AliasManagerImpl manager = new AliasManagerImpl(aliasesFile, cc, FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    ClusterController cc = new ClusterControllerImpl1();
    AliasManagerImpl manager = new AliasManagerImpl(aliasesFile, cc, FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getAliasesConfigState() throws AdminException {
      long now = System.currentTimeMillis();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getAliasesConfigState() throws AdminException {
      return null;
    }
  }

}
