package ru.novosoft.smsc.admin.acl;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class AclManagerTest {
  @Test
  public void testCreateAcl() throws Exception {
    AclManager manager = new AclManager(new ClusterControllerImpl(false));

    try {
      manager.createAcl("ok", "ok", new ArrayList<Address>());
      assertFalse(true);
    } catch (AclException ignored) {}
    
  }

  @Test
  public void testRemoveAcl() throws Exception {
    AclManager manager = new AclManager(new ClusterControllerImpl(false));

    try {
      manager.removeAcl(1);
      assertFalse(true);
    } catch (AclException ignored) {}
  }

  @Test
  public void testUpdateAcl() throws Exception {
    AclManager manager = new AclManager(new ClusterControllerImpl(false));

    try {
      manager.updateAcl(1, "ok", "ok");
      assertFalse(true);
    } catch (AclException ignored) {}
  }

  @Test
  public void testAddAddresses() throws Exception {
    AclManager manager = new AclManager(new ClusterControllerImpl(false));

    try {
      manager.addAddresses(1, new ArrayList<Address>());
      assertFalse(true);
    } catch (AclException ignored) {}
  }

  @Test
  public void testRemoveAddresses() throws Exception {
    AclManager manager = new AclManager(new ClusterControllerImpl(false));

    try {
      manager.removeAddresses(1, new ArrayList<Address>());
      assertFalse(true);
    } catch (AclException ignored) {}
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    AclManager manager = new AclManager(new ClusterControllerImpl(true));

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();
    assertNotNull(states);

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));

    assertNull(new AclManager(new ClusterControllerImpl(false)).getStatusForSmscs());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    private boolean online;

    public ClusterControllerImpl(boolean online) {
      this.online = online;
    }

    public boolean isOnline() {
      return online;
    }

    public ConfigState getAclState() throws AdminException {
      long now = System.currentTimeMillis();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 100);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }
}
